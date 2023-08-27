#include "server.h"
#include "../Socket/Socket.h"
#include <sys/epoll.h>
#include <assert.h>
#include <csignal>
#include <unistd.h>

using namespace std;

// 发送消息的最大字节长度
const int MSGSIZE = 1024;


void Server::addfd(int fd) {
    // 将内核事件表注册读事件
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &event);
}

// 初始化服务器
void Server::initServer () {
    // 创建服务器socket
    m_fd = Socket(AF_INET, SOCK_STREAM, 0);
    // 设置端口复用
    SetReuseaddr(m_fd);

    // 绑定套接字
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(m_port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(m_fd, (sockaddr*)&(address), sizeof(address));

    // 进入监听
    Listen(m_fd, 10);

    // epoll创建内核事件表
    epoll_event events[MAX_EVENT_NUMBER];
    m_epfd = epoll_create1(0);
    assert(m_epfd != -1);
    
    con::m_epollfd = m_epfd;
    // 设置非阻塞
    SetNonBlocking(m_fd);
    addfd(m_fd);

    // 忽略SIGPIPE信号，防止因为向已关闭的连接写入数据导致进程终止
    signal(SIGPIPE, SIG_IGN);
    
}

void Server::threadpool () {
    //线程池
    m_pool = new thread_pool<con>(m_thread_num);
}

void Server::eventLoop () {
    bool stop_server = false;

    while (!stop_server) {
        // 调用epoll_wait等待事件发生
        int number = epoll_wait(m_epfd, events, MAX_EVENT_NUMBER, -1);
        if (number < 0 && errno != EINTR)
        {
            break;
        }
        // 遍历所有事件
        for (int i = 0; i < number; i++) {
            int sockfd = events[i].data.fd;

            //处理新到的客户连接
            if (sockfd == m_fd)
            {
                // std::cout << "接收到客户连接" << std::endl;
                bool flag = dealclinetdata();
                if (false == flag)
                    continue;
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                // 服务器端关闭连接
                std::cout << "客户端 " << sockfd << " 连接中断" << std::endl;
                --con::m_user_count;
                // 从内核事件表中删除该事件
                epoll_ctl(m_epfd, EPOLL_CTL_DEL, sockfd, nullptr);
                // 关闭文件描述符
                close(sockfd);
            }
            //处理客户连接上接收到的数据
            else if (events[i].events & EPOLLIN)
            {
                // std::cout << "监听到读事件" << std::endl;
                //若监测到读事件，将该事件放入请求队列
                m_pool->append(users + sockfd);
            }
        }
    }
}

bool Server::dealclinetdata() {
    sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
   
    // 接受一个连接请求
    int connfd = Accept(m_fd, (struct sockaddr *)&client_address, &client_addrlength);
    
    // 如果当前连接数已达到上限，返回错误信息给客户端并关闭连接
    if (con::m_user_count >= MAX_FD)
    {
        return true;
    }
    users[connfd].con::init(connfd, client_address);
    SetNonBlocking(connfd);
    addfd(connfd);
    std::cout << "客户端 " << connfd << " 连接" << std::endl;

    return false;
}
