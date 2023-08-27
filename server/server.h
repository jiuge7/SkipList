#ifndef SERVER_H
#define SERVER_H
#include "../skiplist/skiplist.h"
#include <netinet/in.h>
#include "../thread_pool/thread_pool.h"
#include "../con/con.h"
#include <sys/epoll.h>
#include <map>
using namespace std;

const int MAX_FD = 65536;           //最大文件描述符
const int MAX_EVENT_NUMBER = 1000;  // 最大事件数

class Server {
public:
    int m_port;   //定义端口号
    int m_fd;   
    int m_epfd; 
    Server(int port, int thread_num) {
        m_port = port;
        m_thread_num = thread_num;
        //创建http_conn类对象
        users = new con[MAX_FD];
    }
    void threadpool();
    void initServer();
    void eventLoop();

    con *users;
    // 线程池相关
    thread_pool<con> *m_pool;
    int m_thread_num;           // 最大线程数

    // epoll_event相关
    epoll_event events[MAX_EVENT_NUMBER];
    void addfd(int fd);
    // 添加新客户端
    bool dealclinetdata();

};

#endif