#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <functional>
#include <string_view>
#include "../log/log.h"
#include "../CGImysql/sql_connection_pool.h"
#include <atomic>

enum TRIGMode{ET, LT};

void modfd(int epollfd, int fd, int ev, TRIGMode mode);

#include "http_writer.h"

class http_conn
{
public:
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 8192;
    static const int WRITE_BUFFER_SIZE = 81920;
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
        RELOCATION
    };
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };
    enum WRITE_METHOD{
        DEFAULT,
        DIRICT,
        ZEROCOPY,
        MIXED
    };

public:
    http_conn() : registered(true), m_sockfd(-1){
        std::function<void()> func = std::bind((void(http_conn::*)())&http_conn::init, this);
        writer = std::make_unique<default_write>(m_write_buf, func, connection_mode);
        // writer = new default_write(m_write_buf, func, connection_mode);
    };
    ~http_conn() = default;

public:
    void init();
    void init(int sockfd, const sockaddr_in &addr, TRIGMode mode);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();
    bool write();
    static int &get_m_epollfd(){ static int m_epollfd; return m_epollfd;}
    static int get_m_user_count(){ return m_user_count.load();}
    sockaddr_in *get_address()
    {
        return &m_address;
    }
    void initmysql_result(std::shared_ptr<connection_pool> &connPool);

private:
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    HTTP_CODE parse_request_line(std::string_view text);
    HTTP_CODE parse_headers(std::string_view text);
    HTTP_CODE parse_content(std::string_view text);
    HTTP_CODE do_request();
    std::string_view get_line() { return m_read_buf + m_start_line; };
    LINE_STATUS parse_line();
    void unmap();
    bool add_response(const char* format, ...);
    bool add_content(const std::string &content);
    bool add_status_line(int status, const std::string &title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
    bool add_cookies();
    bool add_cache();
    void get_cookie(std::string_view text);
    static std::atomic<int> m_user_count;

public:
    MYSQL *mysql;
    std::shared_ptr<connection_pool> pool;
    char m_read_buf[READ_BUFFER_SIZE];
    int m_sockfd;

private:
    sockaddr_in m_address;
    int m_read_idx;
    int m_checked_idx;
    int m_start_line;
    char m_write_buf[WRITE_BUFFER_SIZE];
    int m_write_idx;
    CHECK_STATE m_check_state;
    METHOD m_method;
    std::string m_real_file;
    std::string m_url;
    std::string m_version;
    std::string m_host;
    std::string user_name;
    std::unordered_map<std::string, std::string> m_headers;
    int m_content_length;
    bool m_linger;
    struct stat m_file_stat;
    int cgi;        //是否启用的POST
    std::string m_string; //存储请求头数据
    TRIGMode listen_mode;
    TRIGMode connection_mode;
    bool registered;
    std::unordered_map<std::string, std::string> cookies;
    std::unique_ptr<http_write> writer;
    // default_write *writer;
    
};

#endif
