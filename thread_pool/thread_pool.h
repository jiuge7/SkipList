#ifndef THREAD_POLL
#define THREAD_POLL

#include <list>
#include <thread>
#include <iostream>
#include <mutex>
#include "../lock/lock.h"

//线程池类，定义成模板是为了代码的复用，模板T为任务类
template<typename T>
class thread_pool
{
public:
    //thread_number是线程池中线程的数量
    //max_requests是请求队列中最多允许的、等待处理的请求的数量
    //connPool是数据库连接池指针
    thread_pool(int thread_number = 8, int max_request = 10000);
    ~thread_pool() {}

    //向请求队列中插入任务请求
    bool append(T* request);
private:
    //调用run函数，处理线程
    static void *worker(void *arg);
    //完成线程处理要求
    void run();
    int m_max_request;      //最大线程数
    int m_thread_number;    //当前线程数

    //线程池数组，大小为m_max_request
    pthread_t* m_threads;

    std::mutex m_mutex;

    //请求队列
    std::list<T *> worklist;

    //信号量，判断是否有任务需要处理
    sem liststat;

    //是否结束线程
    bool m_stop;

};

//创建线程池
template<typename T>
thread_pool<T>::thread_pool(int thread_number, int max_request) : 
    m_thread_number(thread_number), m_max_request(max_request), m_stop(false), m_threads(NULL){

    if(thread_number <= 0 || max_request <= 0){
        throw std::exception();
    }
    //线程初始化
    m_threads = new pthread_t[m_thread_number];
    if(!m_threads){
        throw std::exception();
    }

    //创建thread_number个线程，并设置线程脱离
    for(int i = 0;i < thread_number; ++i){
        //创建进程，并转交给worker
        if(pthread_create(m_threads + i, NULL ,worker, this) != 0){
            delete[] m_threads;
            throw std::exception();
        }
        //线程分离
        if( pthread_detach(m_threads[i]) ){
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template<typename T>
bool thread_pool<T>::append(T* request){
    std::unique_lock<std::mutex> lock(m_mutex);
    //队列超过最大长度
    if(worklist.size() > m_max_request){
        return false;
    }
    //任务加入队列
    worklist.push_back(request);
    //信号量加1，提示有任务需要处理
    liststat.post();
    return true;
}

template<typename T>
void* thread_pool<T>::worker(void *arg){
    //强转为线程池类，调用run方法
    thread_pool* pool = (thread_pool*)arg;
    pool->run();
    return pool;
}

template<typename T>
void thread_pool<T>::run(){
    while(!m_stop){

        //信号量等待
        liststat.wait();

        std::unique_lock<std::mutex> lock(m_mutex);
        if(worklist.empty())
        {
            continue;
        }

        //从请求队列中取出一个任务
        T* request = worklist.front();
        worklist.pop_front();

        if(!request){
            continue;
        }
        // process(模板类中的方法)进行处理
        request->process();
    }
}


#endif