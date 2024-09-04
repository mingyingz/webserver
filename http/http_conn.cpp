#include "http_conn.h"
#include <iostream>

std::string doc_root = "/home/zhonghm/webserver/webserver/root";

const int cache_contral = 30;

extern std::atomic<long long> time_process;
long long get_time();

std::mutex m_mutex;
std::unordered_map<int, std::string> titles = {
    {200,  "OK"},
    {304,  "Not Modified"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {500, "Internal Error"},
};

std::unordered_map<int, std::string> formats = {
    {400, "Your request has bad syntax or is inherently impossible to staisfy.\n"},
    {403, "You do not have permission to get file form this server.\n"},
    {404, "The requested file was not found on this server.\n"},
    {500, "There was an unusual problem serving the request file.\n"},
};

std::map<std::string, std::string> users;
std::unordered_set<std::string> login_token;

void http_conn::initmysql_result(std::shared_ptr<connection_pool> &connPool)
{
    //先从连接池中取一个连接
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql, connPool);

    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user"))
    {
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
    }

    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);

    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);

    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result))
    {
        std::string temp1(row[0]);
        std::string temp2(row[1]);
        users[temp1] = temp2;
    }
}

int setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL); //获取文件描述符旧的状态
    int new_option = old_option | O_NONBLOCK;  //设置非阻塞标志
    fcntl(fd, F_SETFL, new_option); 
    return old_option;  //返回文件描述符旧的状态，以便日后恢复该状态标志
}

void addfd(int epollfd, int fd, bool one_shot, TRIGMode mode){
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(mode == ET)
        event.events = EPOLLIN | EPOLLHUP | EPOLLET;
    else
        event.events = EPOLLIN | EPOLLHUP;
    if(one_shot)
        event.events |= EPOLLONESHOT;
    assert(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event) != -1);
    setnonblocking(fd);
}

void removefd(int epollfd, int fd){
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
    // std::cout << "close: " << fd << std::endl;
    close(fd);
}

void modfd(int epollfd, int fd, int ev, TRIGMode mode){
    epoll_event event;
    event.data.fd = fd;
    if(mode == ET)
        event.events = ev | EPOLLHUP | EPOLLONESHOT | EPOLLET;
    else
        event.events = ev | EPOLLHUP | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);

}

void http_conn::init(){
    mysql = NULL;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    m_url = "";
    m_version = "";
    m_content_length = 0;
    m_host = "";
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    cgi = 0;
    memset(m_read_buf, '\0', READ_BUFFER_SIZE);
    memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
    m_real_file.clear();
    writer->reset();
}

void http_conn::init(int sockfd, const sockaddr_in &addr, TRIGMode mode){
    connection_mode = mode;
    m_sockfd = sockfd;
    m_address = addr;
    addfd(get_m_epollfd(), sockfd, true, mode);
    m_user_count.fetch_add(1);
    init();

}

bool http_conn::read_once(){
    // std::cout << connection_mode << std::endl;
    if(m_read_idx > READ_BUFFER_SIZE)
        return false;
    if(connection_mode == ET){
        while(true){
            int bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
            // std::cout << bytes_read << std::endl;
            if(bytes_read == -1){
                if(errno == EAGAIN || errno == EWOULDBLOCK){ // 这两个错误码表示当前的操作会阻塞程序，但是由于设置了非阻塞标志，因此系统并不会等待，而是立即返回，EWOULDBLOCK可能在其他系统上使用。
                    break;
                }
                return false;
            }
            if(bytes_read == 0) // recv返回值为0，说明对方已经关闭了连接
                return false;
            m_read_idx += bytes_read;
        }
        return true;
    }

    else if(connection_mode == LT){
        int bytes_read = recv(m_sockfd, m_read_buf + m_read_idx, READ_BUFFER_SIZE - m_read_idx, 0);
        // std::cout << "LLLLLLLLLLLLL" << bytes_read << std::endl;
        if(bytes_read <= 0){
            return false;
        }
            
        m_read_idx += bytes_read;
        return true;
    }
    assert(0);
}

http_conn::LINE_STATUS http_conn::parse_line(){
    char temp;
    for (; m_checked_idx < m_read_idx; ++m_checked_idx)
    {
        temp = m_read_buf[m_checked_idx];
        if (temp == '\r')
        {
            //下一个字符达到了buffer结尾，则接收不完整，需要继续接收
            if ((m_checked_idx + 1) == m_read_idx)
                return LINE_OPEN;
            else if (m_read_buf[m_checked_idx + 1] == '\n')
            {
                m_read_buf[m_checked_idx++] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }

        //如果当前字符是\n，也有可能读取到完整行
        //一般是上次读取到\r就到buffer末尾了，没有接收完整，再次接收时会出现这种情况
        else if (temp == '\n')
        {
            if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r')
            {
                m_read_buf[m_checked_idx - 1] = '\0';
                m_read_buf[m_checked_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    //并没有找到\r\n，需要继续接收
    return LINE_OPEN;
}

http_conn::HTTP_CODE http_conn::parse_request_line(std::string_view text){
    // printf("request_line: %s\n", text.c_str());
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    std::string s(text);
    if(regex_match(s, subMatch, patten)) {   
        if(subMatch[1] == "GET"){
            m_method=GET;
        }
        else if(subMatch[1] == "POST"){
            m_method=POST;
            cgi=1;
        }
        m_url = subMatch[2];
        if(m_url.substr(0, 7) == "http://"){
            m_url = m_url.substr(m_url.find('/', 7));
        }
	else if(m_url.substr(0, 8) == "https://"){
            m_url = m_url.substr(m_url.find('/', 8));
        }
        if(m_url[0] != '/')
            return BAD_REQUEST;
        if(m_url.size() == 1)
            m_url += "judge.html";
        m_version = subMatch[3];
        if(m_version != "1.1")
            return BAD_REQUEST;
        m_check_state = CHECK_STATE_HEADER;
        return NO_REQUEST;
    }
    return BAD_REQUEST;

}

void http_conn::get_cookie(std::string_view text){
    std::string_view key, value;
    int pos = 0;
    while(pos < text.size() && pos >= 0){
        // std::cout << text << std::endl;
        int key_len = text.find('=', pos);
        if(key_len == 0 || key_len == std::string::npos)
            break;
        key = text.substr(pos, key_len);
        pos += key_len + 1;
        int value_len = text.find(';', pos);
        if(value_len == 0)
            break;
        value = text.substr(pos, value_len);
        cookies[std::string(key)] = value;
        pos += value_len + 1;
    }
}

http_conn::HTTP_CODE http_conn::parse_headers(std::string_view text){
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    
    // std::cout << "-----------------header----------------" << std::endl;
    // std::cout << text << std::endl;
    std::string s(text);
    if(std::regex_match(s, subMatch, patten)) {
        m_headers[subMatch[1]] = subMatch[2];
        if(subMatch[1] == "Connection"){
            if(subMatch[2] == "keep-alive")
                m_linger = true;
        }
        else if(subMatch[1] == "Content-Length"){
            m_content_length = stoi(subMatch[2]);
        }
        else if(subMatch[1] == "Cookie"){
            get_cookie(subMatch[2].str());
            // std::cout << cookies["LoginToken"] << std::endl;
            if(login_token.find(cookies["LoginToken"]) != login_token.end()){
                registered = true;
            }
        }
        return NO_REQUEST;
    }
    else {
        if(m_headers.find("Content-Length") == m_headers.end()){
            return GET_REQUEST;
        }
        m_check_state = CHECK_STATE_CONTENT;
        return NO_REQUEST;
    }
}

http_conn::HTTP_CODE http_conn::parse_content(std::string_view text){
    if (m_read_idx >= (m_content_length + m_checked_idx)){
        //text[m_content_length] = '\0';
        //POST请求中最后为输入的用户名和密码
        m_string = text.substr(0, m_content_length);
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::process_read(){
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    std::string_view text;

    while((m_check_state==CHECK_STATE_CONTENT && line_status == LINE_OK) || ((line_status = parse_line()) == LINE_OK)){
	    text = get_line();
        m_start_line = m_checked_idx;
        // std::cout << text << 1 << std::endl;
        // std::cout << (int) m_read_buf[m_start_line - 3] << m_read_buf[m_start_line - 1] << std::endl;
        LOG_INFO("http:  %s", text.data());
        // Log::get_instance()->write_log(1, "%s", text);
        Log::get_instance()->flush();

        switch(m_check_state){
            case CHECK_STATE_REQUESTLINE:
                ret = parse_request_line(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            case CHECK_STATE_HEADER:
                ret = parse_headers(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                else if (ret == GET_REQUEST)
                    return do_request();
                break;
            case CHECK_STATE_CONTENT:
                ret = parse_content(text);
                if (ret == GET_REQUEST)
                    return do_request();
                line_status = LINE_OPEN;
                break;
            default:
                return INTERNAL_ERROR;

        }
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::do_request()
{
    // printf("m_url:%s\n", m_url.c_str());
    int index = m_url.rfind('/');

    //处理cgi
    if (cgi == 1 && (m_url[index + 1] == '2' || m_url[index + 1] == '3'))
    {

        //根据标志判断是登录检测还是注册检测
        char flag = m_url[1];

        std::string m_url_real = "/" + m_url.substr(2, m_url.size() - 3);
        m_real_file = doc_root + m_url_real;

        //将用户名和密码提取出来
        //user=123&passwd=123
        std::string name, password;
        name = m_string.substr(5, m_string.find("&") - 5);
        password = m_string.substr(m_string.find("&") + 10, m_string.size() - name.size() - 10);


        if (m_url[index + 1] == '3')
        {
            //如果是注册，先检测数据库中是否有重名的
            //没有重名的，进行增加数据

            std::string sql_insert = "INSERT INTO user(username, passwd) VALUES('" + name + "', '" + password + "')";

            if (users.find(name) == users.end())
            {
                connectionRAII connection(&mysql, pool);
                int res = 0;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    res = mysql_query(mysql, sql_insert.c_str());
                    users.insert(std::pair<std::string, std::string>(name, password));
                }

                if (!res)
                    m_url = "/log.html";
                else
                    m_url = "/registerError.html";
            }
            else
                m_url = "/registerError.html";
        }
        //如果是登录，直接判断
        //若浏览器端输入的用户名和密码在表中可以查找到，返回1，否则返回0
        else if (m_url[index + 1] == '2')
        {
            if (users.find(name) != users.end() && users[name] == password){
                registered = true;
                m_url = "/welcome.html";
            }
            else
                m_url = "/logError.html";
        }
    }

    if (m_url[index + 1] == '0')
    {
        m_real_file = doc_root + "/register.html";
    }
    else if (m_url[index + 1] == '1')
    {
        m_real_file = doc_root + "/log.html";
    }
    else if(!registered){
        m_real_file = doc_root + "/judge.html";
    }
    else if (m_url[index + 1] == '5')
    {
        m_real_file = doc_root + "/picture.html";
    }
    else if (m_url[index + 1] == '6')
    {
        m_real_file = doc_root + "/video.html";
    }
    else if (m_url[index + 1] == '7')
    {
        m_real_file = doc_root + "/fans.html";
    }
    else
        m_real_file = doc_root + m_url;

    // if(m_headers["If-None-Match"] == "111111111111")
    //     return RELOCATION;


    // stat()函数是用来获取文件的状态信息的函数之一。它可以用来获取文件的各种属性，比如文件大小、权限、创建时间等。
    // buf参数是一个指向stat结构的指针，用于存储获取到的文件状态信息。成功调用stat()函数会返回0，失败则返回-1，并设置errno以指示错误的原因。
    if (stat(m_real_file.c_str(), &m_file_stat) < 0){
        return NO_RESOURCE;
    }

    //判断文件的权限，是否可读，不可读则返回FORBIDDEN_REQUEST状态
    if (!(m_file_stat.st_mode & S_IROTH)){
        return FORBIDDEN_REQUEST;
    }

    //判断文件是不是目录
    if (S_ISDIR(m_file_stat.st_mode))
        return BAD_REQUEST;

    //以只读方式获取文件描述符，通过mmap将该文件映射到内存中，mmap后可以把文件关闭
    writer->file_init(m_real_file, m_file_stat);
    return FILE_REQUEST;
}

bool http_conn::add_response(const char *format, ...)
{
    if (m_write_idx >= WRITE_BUFFER_SIZE)
        return false;
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx))
    {
        va_end(arg_list);
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);
    // std::cout << "buf: " << m_write_buf << std::endl;
    LOG_INFO("request:%s", m_write_buf);
    Log::get_instance()->flush();
    return true;
}

bool http_conn::add_status_line(int status, const std::string &title){
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title.c_str());
}

bool http_conn::add_content_length(int content_len){
    return add_response("Content-Length:%d\r\n", content_len);
}

bool http_conn::add_headers(int content_len){
    add_content_length(content_len);
    add_linger();
    //add_cookies();
    add_cache();
    add_blank_line();
    return true;
}

bool http_conn::add_linger(){
    return add_response("Connection:%s\r\n", (m_linger == true) ? "keep-alive" : "close");
}

bool http_conn::add_blank_line(){
    return add_response("\r\n");
}

bool http_conn::add_cookies(){
    if(!registered)
        return false;
    else if(cookies.find("LoginToken") == cookies.end() || login_token.find(cookies["LoginToken"]) == login_token.end()){
    // else{
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<long long> distr;
        // std::hash<std::string> hasher;
        // size_t hashValue = hasher(user_name);
        std::string token = std::to_string(distr(gen));
        login_token.insert(token);
        return add_response("Set-Cookie:%s\r\n", ("LoginToken=" + token + ";path=/").c_str());
        // return true;
    }
    else{
        return false;
    }
}

bool http_conn::add_cache(){
    add_response("Etag: %lld\r\n", 111111111111);
    return add_response("Cache-Control: max-age=%d\r\n", cache_contral);
}

bool http_conn::add_content(const std::string &content){
    return add_response("%s", content.c_str());
}

bool http_conn::process_write(HTTP_CODE read_ret){
    switch(read_ret){
        case FILE_REQUEST:
            add_status_line(200, titles[200]);
            if(m_file_stat.st_size != 0){ 
                add_headers(m_file_stat.st_size);
                writer->file_modify(m_write_idx);
                return true;
            }
            else{
                //如果请求的资源大小为0，则返回空白html文件
                const char *ok_string = "<html><body></body></html>";
                add_headers(strlen(ok_string));
                if (!add_content(ok_string))
                    return false;
            }
            break;
        case BAD_REQUEST:
            add_status_line(400, titles[400]);
            add_headers(formats[400].size());
            if(!add_content(formats[400]))
                return false;
            break;
        case FORBIDDEN_REQUEST:
            add_status_line(403, titles[403]);
            add_headers(formats[403].size());
            if(!add_content(formats[403]))
                return false;
            break;
        case NO_RESOURCE:
            add_status_line(404, titles[404]);
            add_headers(formats[404].size());
            if(!add_content(formats[404]))
                return false;
            break;
        case INTERNAL_ERROR:
            add_status_line(500, titles[500]);
            add_headers(formats[500].size());
            if(!add_content(formats[500]))
                return false;
            break;
        case RELOCATION:
            add_status_line(304, titles[304]);
            add_headers(formats[304].size());
            break;
        default:
            return false;
    }
    writer->file_modify(m_write_idx);
    return true;
}

bool http_conn::write()
{
    // int temp=0;
    // // int bytes_have_send=0;
    // // int bytes_to_send=m_write_idx;
    // std::cout << "to_send: " << bytes_to_send << std::endl;
    // int &m_epollfd = get_m_epollfd();
    // if(bytes_to_send==0)
    // {
    //     modfd(m_epollfd,m_sockfd,EPOLLIN,connection_mode);
    //     init();
    //     return true;
    // }
    // while(1)
    // {
    //     temp=writev(m_sockfd,m_iv,m_iv_count);
    //     if(temp<=-1)
    //     {
    //         if(errno==EAGAIN)
    //         {
    //             modfd(m_epollfd,m_sockfd,EPOLLOUT,connection_mode);
    //             return true;
    //         }
    //         unmap();
    //         return false;
    //     }
    //     bytes_to_send-=temp;
    //     bytes_have_send+=temp;
    //     if(bytes_to_send<=bytes_have_send)
    //     {
    //         unmap();
    //         if(m_linger)
    //         {
    //             init();
    //             modfd(m_epollfd,m_sockfd,EPOLLIN,connection_mode);
    //             return true;
    //         }
    //         else
    //         {
    //             modfd(m_epollfd,m_sockfd,EPOLLIN,connection_mode);
    //             return false;
    //         }
    //     }
    // }
    assert(writer.get() != nullptr);
    return writer->write(get_m_epollfd(), m_sockfd, m_linger);
}

void http_conn::process(){
    long long t1 = get_time();
    HTTP_CODE read_ret = process_read();
    int &m_epollfd = get_m_epollfd();

    if(read_ret == NO_REQUEST){
        modfd(m_epollfd, m_sockfd, EPOLLIN, connection_mode);
        return;
    }
    bool write_ret = process_write(read_ret);
    if(!write_ret){
        close_conn();
    }
    modfd(m_epollfd, m_sockfd, EPOLLOUT, connection_mode);
    long long t2 = get_time();
    assert(t2 >= t1);
    time_process.fetch_add(t2 - t1);
}

void http_conn::close_conn(bool real_close){
    if(real_close){
        // std::cout << "close: " << m_sockfd << std::endl;
        int sockfd = m_sockfd;
        m_sockfd = -1;
        user_name = "";
        cookies.clear();
        m_user_count.fetch_sub(1);
        removefd(get_m_epollfd(), sockfd);
    }
}
