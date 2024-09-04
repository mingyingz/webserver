
#include "http_conn.h"
#include <sys/sendfile.h>

class http_conn;

class http_write{
public:
    http_write(char *m_write_buf, std::function<void()> init, TRIGMode connection_mode): buf(m_write_buf), http_init(init), connection_mode(connection_mode){};
    virtual void file_init(std::string path, struct stat &m_file_stat) = 0;
    virtual void file_modify(int m_write_idx) = 0;
    virtual bool write(int epollfd, int sockfd, bool linger) = 0;
    virtual void reset() = 0;
    virtual ~http_write() {};
protected:
    TRIGMode connection_mode;
    char* buf;
    std::function<void()> http_init;
    int write_idx;
    int bytes_to_send;
    int bytes_have_send;
    size_t file_size;
};

class default_write : public http_write{
public:
    default_write(char *m_write_buf, std::function<void()> init, TRIGMode connection_mode) : http_write(m_write_buf, init, connection_mode), m_file_address(nullptr){};
    // virtual ~default_write() {};
    virtual void file_init(std::string path, struct stat &m_file_stat){
        int fd = open(path.c_str(), O_RDONLY);
        file_size = m_file_stat.st_size;
        m_file_address = (char *)mmap(0, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        m_iv[1].iov_base = m_file_address;
        m_iv[1].iov_len = file_size;
        close(fd);
    }
    
    virtual void file_modify(int m_write_idx){
        m_iv[0].iov_base = buf;
        m_iv[0].iov_len = m_write_idx;
        write_idx = m_write_idx;
        if(m_file_address != nullptr){
            m_iv_count = 2;
            bytes_to_send = m_write_idx + file_size;
        }
        else{
            m_iv_count = 1;
            bytes_to_send = m_write_idx;
        }
    }

    virtual bool write(int epollfd, int sockfd, bool linger){
        int temp = 0;

        if (bytes_to_send == 0)
        {
            modfd(epollfd, sockfd, EPOLLIN, connection_mode);
            http_init();
            return true;
        }

        while (1)
        {
            // std::cout << "1111111111 " << m_iv[1].iov_len << std::endl;
            // std::cout << "to send " << bytes_to_send << std::endl;
            // std::cout << "iv_count " << m_iv_count << std::endl;

            temp = writev(sockfd, m_iv, m_iv_count);
            // std::cout << "ttttttttttt: " << temp << std::endl;
            // std::cout << "temp: " << temp << " sockfd: " << m_sockfd << std::endl;

            if (temp < 0)
            {
                if (errno == EAGAIN)
                {
                    modfd(epollfd, sockfd, EPOLLOUT, connection_mode);
                    return true;
                }
                unmap();
                return false;
            }

            bytes_have_send += temp;
            bytes_to_send -= temp;
            // std::cout << bytes_have_send << " " << bytes_to_send << std::endl;
            if (bytes_have_send >= write_idx)
            {
                m_iv[0].iov_len = 0;
                m_iv[1].iov_base = m_file_address + (bytes_have_send - write_idx);
                m_iv[1].iov_len = bytes_to_send;
            }
            else
            {
                m_iv[0].iov_base = buf + bytes_have_send;
                m_iv[0].iov_len = write_idx - bytes_have_send;
            }

            if (bytes_to_send <= 0)
            {
                unmap();
                modfd(epollfd, sockfd, EPOLLIN, connection_mode);

                if (linger)
                {
                    http_init();
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
    virtual void reset(){
        unmap();
        bytes_to_send = 0;
        bytes_have_send = 0;
    }
    void unmap(){
        if (m_file_address != nullptr){
            munmap(m_file_address, file_size);
            m_file_address = nullptr;
            file_size = 0;
        }
    }
private:
    struct iovec m_iv[2];
    int m_iv_count;
    char *m_file_address;
};

class zero_copy_write : public http_write{
public:
    zero_copy_write(char *m_write_buf, std::function<void()> init, TRIGMode connection_mode) : http_write(m_write_buf, init, connection_mode), file_fd(-1){};
    virtual ~zero_copy_write() {};
    virtual void file_init(std::string path, struct stat &m_file_stat){
        file_size = m_file_stat.st_size;
        file_fd = open(path.c_str(), O_RDONLY);
    }
    
    virtual void file_modify(int m_write_idx){
        bytes_to_send = m_write_idx + file_size;
        write_idx = m_write_idx; 
    }

    virtual bool write(int epollfd, int sockfd, bool linger){
        int temp = 0;

        if (bytes_to_send == 0)
        {
            modfd(epollfd, sockfd, EPOLLIN, connection_mode);
            http_init();
            return true;
        }

        while (1)
        {
            // std::cout << "1111111111 " << m_iv[1].iov_len << std::endl;
            // std::cout << "to send " << bytes_to_send << std::endl;
            // std::cout << "iv_count " << m_iv_count << std::endl;
            if(write_idx > bytes_have_send)
                temp = send(sockfd, buf + bytes_have_send, write_idx - bytes_have_send, 0);
            else{
                temp = sendfile(sockfd, file_fd, &offset, bytes_to_send);
            }
            // std::cout << "ttttttttttt: " << temp << std::endl;
            // std::cout << "temp: " << temp << " "  << offset << " " << (errno == EAGAIN) << " " << write_idx << " " << bytes_have_send << " " << bytes_to_send << " "  << std::endl;

            if (temp < 0)
            {
                if (errno == EAGAIN)
                {
                    modfd(epollfd, sockfd, EPOLLOUT, connection_mode);
                    return true;
                }
                close_file();
                return false;
            }

            bytes_have_send += temp;
            bytes_to_send -= temp;
            // std::cout << bytes_have_send << " " << bytes_to_send << std::endl;

            if (bytes_to_send <= 0)
            {
                modfd(epollfd, sockfd, EPOLLIN, connection_mode);
                close_file();

                if (linger)
                {
                    http_init();
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
    }
    virtual void reset(){
        bytes_to_send = 0;
        bytes_have_send = 0;
        offset = 0;
        close_file();
    }
    void close_file(){
        if(file_fd > 0)
            close(file_fd);
        file_fd = -1;
    }
private:
    int file_fd;
    off_t offset;
};
