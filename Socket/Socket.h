#ifndef SOCKET_H
#define SOCKET_H
#include <cstdio>
#include <arpa/inet.h>
#include <fcntl.h>

//封装网络编程常用函数，避免频繁获取返回值判断是否出错

//设置端口复用
int SetReuseaddr(int sockfd);

//设置文件描述符非阻塞
void SetNonBlocking(int fd);

int Socketpair(int sv[2]);

int Socket(int family, int type, int protocol);

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

int Listen(int sockfd, int backlog);

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

#endif