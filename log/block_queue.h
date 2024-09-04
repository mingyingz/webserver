#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <iostream>

template <typename T>
class block_queue{
public:
    block_queue(int max_size = 1000){
        if (max_size <= 0)
        {
            exit(-1);
        }

        m_max_size = max_size;
        m_size = 0;
        m_front = -1;
        m_back = -1;
        is_close = false;
    };
    void clear(){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_size = 0;
        m_front = -1;
        m_back = -1;
    }
    ~block_queue(){
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while(!m_queue.empty()){
                m_queue.pop();
            }
            is_close = true;
        }
        m_cond_consumer.notify_all();
        m_cond_producer.notify_all();
    }
    bool full() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size() == m_max_size;
    }
    bool empty(){
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size() == 0;

    }
    bool front(T &value) 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (0 == m_queue.size()){
            return false;
        }
        value = m_queue.front();
        return true;
    }
    //返回队尾元素
    bool back(T &value) 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (0 == m_queue.size()){
            return false;
        }
        value = m_queue.back();
        return true;
    }

    int size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    int max_size(){
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_max_size;
    }

    bool push(T& item){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond_producer.wait(lock, [this] { return m_queue.size() < m_max_size; });
        if(is_close){
            return false;
        }
        m_queue.push(item);
        m_cond_consumer.notify_one();
        return true;
    }

    bool pop(T &item){
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond_consumer.wait(lock, [this] { return m_queue.size() > 0; });
        if(is_close){
            return false;
        }
        item = m_queue.front();
        m_queue.pop();
        m_cond_producer.notify_one();
        return true;
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cond_producer;
    std::condition_variable m_cond_consumer;

    std::queue<T> m_queue;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
    bool is_close;
};

#endif