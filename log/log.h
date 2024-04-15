#ifndef LOG_H
#define LOG_H

#include <memory>
#include <string>
#include <mutex>
#include <stdarg.h>
#include "block_queue.h"
#include <iostream>

class Log{
public:
    static std::shared_ptr<Log> get_instance(){
        static std::shared_ptr<Log> p = std::shared_ptr<Log>(new Log);
        return p;
    }

    static void *flush_log_thread()
    {
        Log::get_instance()->async_write_log();
    }

    //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
    bool init(const char *file_name, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);

    virtual ~Log();

private:
    Log();

    void *async_write_log()
    {
        std::string single_log;
        //从阻塞队列中取出一个日志string，写入文件
        while (m_log_queue->pop(single_log)){
            std::unique_lock<std::mutex> lock(m_mutex);
            fputs(single_log.c_str(), m_fp);
        }
    }

private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines;  //日志最大行数
    int m_log_buf_size; //日志缓冲区大小
    long long m_count;  //日志行数记录
    int m_today;        //因为按天分类,记录当前时间是那一天
    FILE *m_fp;         //打开log的文件指针
    char* m_buf;
    std::unique_ptr<block_queue<std::string>> m_log_queue; //阻塞队列
    bool m_is_async;                  //是否同步标志位
    std::mutex m_mutex;

};

inline bool m_close_log = true;
#define LOG_DEBUG(format, ...) if(flase == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(false == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(false == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(false == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif