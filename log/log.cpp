#include <cstring>
#include <memory>
#include <cassert>
#include <ctime>
#include <sys/time.h>
#include <thread>
#include <functional>
#include <stdarg.h>
#include "log.h"
#include <iostream>

Log::Log(){
    m_count = 0;
    m_is_async = false;
}

Log::~Log(){
    if (m_fp != NULL){
        fclose(m_fp);
    }
}

bool Log::init(const char* file_name, int log_buf_size, int split_lines, int max_queue_size){
    if (max_queue_size >= 1){
        m_is_async = true;
        m_log_queue = std::make_unique<block_queue<std::string>>(max_queue_size);
        // m_log_queue = std::make_unique<block_queue<std::string>>(max_queue_size);
        //flush_log_thread为回调函数,这里表示创建线程异步写日志
        //如果是静态成员函数或全局函数，直接使用函数名，否则应该使用&Log::flush_log_thread以获得函数指针
        std::thread thread(Log::flush_log_thread);
        thread.detach();  
    }
    m_close_log = false;
    m_log_buf_size = log_buf_size;
    m_buf = new char[m_log_buf_size]{'\0'};
    m_split_lines = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    
    const char *splitchr = strrchr(file_name, '/');
    char log_full_name[256] = {0};
    assert(strlen(file_name) <= 255);

    if(splitchr == nullptr){
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
        strcpy(log_name, file_name);
        
    }
    else{
        assert(splitchr - file_name <= 127);
        assert(strlen(splitchr) <= 127);
        strcpy(log_name, splitchr + 1);
        std::cout << log_name << std::endl;
        strncpy(dir_name, file_name, splitchr - file_name);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    }

    m_today = my_tm.tm_mday;
    
    m_fp = fopen(log_full_name, "a");
    if (m_fp == NULL)
    {
        return false;
    }

    return true;
}


void Log::write_log(int level, const char *format, ...){
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};
    switch (level){
        case 0:
            strcpy(s, "[debug]:");
            break;
        case 1:
            strcpy(s, "[info]:");
            break;
        case 2:
            strcpy(s, "[warn]:");
            break;
        case 3:
            strcpy(s, "[erro]:");
            break;
        default:
            strcpy(s, "[info]:");
            break;
    }
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_count++;

        if (m_today != my_tm.tm_mday || m_count % m_split_lines == 0) {
            char new_log[256] = {0};
            fflush(m_fp);
            fclose(m_fp);
            char tail[16] = {0};
            snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
            if (m_today != my_tm.tm_mday){
                snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
                m_today = my_tm.tm_mday;
                m_count = 0;
            }
            else{
                snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, m_count / m_split_lines);
            }
            m_fp = fopen(new_log, "a");
        }
    }
    va_list valst;
    va_start(valst, format);
    std::string log_str;

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                        my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                        my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);

        int m = vsnprintf(m_buf + n, m_log_buf_size - n - 1, format, valst);
        m_buf[n + m] = '\n';
        m_buf[n + m + 1] = '\0';
        log_str = m_buf;
    }
    if (m_is_async && !m_log_queue->full()){
        m_log_queue->push(log_str);
    }
    else{
        std::unique_lock<std::mutex> lock(m_mutex);
        fputs(log_str.c_str(), m_fp);
    }

    va_end(valst);

}

void Log::flush(void)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    //强制刷新写入流缓冲区
    fflush(m_fp);
}