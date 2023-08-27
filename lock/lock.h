#ifndef LOCKER_H
#define LOCKER_H
#include <semaphore.h>
#include <stdexcept>
#include <mutex>

//线程同步机制封装

//互斥锁类,通过RAII机制自动销毁
class Mutex{
private:
    std::mutex mutex;
    bool islock;
public:
    Mutex(){
        mutex.lock();
        islock = true;
    }
    ~Mutex(){
        if(islock){
            mutex.unlock();
        }
    }
    void unlock(){
        mutex.unlock();
        islock = false;
    }
    Mutex(const Mutex&) = delete;//禁止拷贝
    Mutex& operator = (const Mutex&) = delete;//禁止赋值
};

//信号量类
class sem
{
private:
    sem_t m_sem;
public:
    sem(int value = 0) {
        if(sem_init(&m_sem, 0, value) != 0){
            //信号量初始化失败，抛出异常并附带异常的描述信息
            throw std::exception();
        }
    };
    ~sem(){
        sem_destroy(&m_sem);
    };
    //等待信号量
    bool wait(){
        return sem_wait(&m_sem) == 0;
    }
    //增加信号量
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }
};




#endif