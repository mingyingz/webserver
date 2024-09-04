#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <assert.h>
#include <functional>
#include <vector>
#include <atomic>



#include "../CGImysql/sql_connection_pool.h"

extern std::atomic<long long> time_add;
extern std::atomic<long long> time_wait;
long long get_time();

class threadpool{
public:
    threadpool(std::shared_ptr<connection_pool> m_connPool, int thread_number = 8, int max_requests = 40000);
    ~threadpool();
    template <typename F>
    bool append(F&& task);
private:

    int m_thread_number;        //线程池中的线程数
    int m_max_requests;         //请求队列中允许的最大请求数
    std::vector<std::thread> m_threads;       //描述线程池的数组，其大小为m_thread_number
    std::list<std::function<void()>> m_workqueue; //请求队列
    std::mutex m_queuemutex;       //保护请求队列的互斥锁
    std::condition_variable m_queuecond;            //是否有任务需要处理
    bool m_stop;                //是否结束线程
    std::shared_ptr<connection_pool> m_connPool;  //数据库
};

threadpool::threadpool(std::shared_ptr<connection_pool> m_connPool, int thread_number, int max_requests): m_connPool(m_connPool), m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false){
    if(thread_number <= 0 || max_requests <= 0){
        throw std::exception();
    }
    for(int i = 0; i < thread_number; i++){
	m_threads.emplace_back([this]{
            while(!m_stop){
                long long t1 = get_time();
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_queuemutex);
                    m_queuecond.wait(lock, [this] { return m_stop || !m_workqueue.empty(); });
                    // std::cout << "sssssssssssssss " << m_workqueue.size() << std::endl;
                    // TODO: request move
                    if(m_stop)
                        return;
                    assert(!m_workqueue.empty());
                    task = m_workqueue.front(); 
                    m_workqueue.pop_front();
                }
                if(!task)
                    continue;

                long long t2 = get_time();
                time_wait.fetch_add(t2 - t1);
                task();
            }
        });
        //t.detach();
	//m_threads.push_back(t);
	m_threads[i].detach();
    }
}

threadpool::~threadpool(){
    {
        std::unique_lock<std::mutex> lock(m_queuemutex);
        m_stop = true;
    }
    m_queuecond.notify_all();
}

template<typename F>
bool threadpool::append(F&& task){
    long long t1 = get_time();
    {
        std::unique_lock<std::mutex> lock(m_queuemutex);
        // std::cout << "sssssssssssssss " << m_workqueue.size() << std::endl;
        if(m_workqueue.size() > m_max_requests){
            return false;
        }
        m_workqueue.emplace_back(std::forward<F>(task));
    }
    m_queuecond.notify_one();
    long long t2 = get_time();
    time_add.fetch_add(t2 - t1);
    return true;
}

#endif
