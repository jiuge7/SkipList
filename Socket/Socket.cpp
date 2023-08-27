#include "Socket.h"

//封装网络编程常用函数，避免频繁获取返回值判断是否出错

//设置端口复用
int SetReuseaddr(int sockfd){
    int opt = 1;
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void SetNonBlocking(int fd) {
    // 获取文件状态标志
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        // 获取失败，打印错误信息
        perror("fcntl");
        return;
    }
    // 用O_NONBLOCK位或操作
    flags |= O_NONBLOCK;
    // 设置回去
    if (fcntl(fd, F_SETFL, flags) == -1) {
        // 设置失败，打印错误信息
        perror("fcntl");
        return;
    }
}

int Socket(int family, int type, int protocol){
    int ret;
    if( (ret = socket(family, type, protocol)) < 0 ){
        perror("socket error");
    }
    return ret;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    int ret;
    if ((ret = bind(sockfd, addr, addrlen)) < 0){
        perror("bind error");
    }
    return ret;
}

int Listen(int sockfd, int backlog){
    int ret;
    if ((ret = listen(sockfd, backlog)) < 0){
        perror("listen error");
    }
    return ret;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    int ret;
    if ((ret = accept(sockfd, addr, addrlen)) < 0){
        perror("accept error");
    }
    return ret;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    int ret;
    if ((ret = connect(sockfd, addr, addrlen)) < 0){
        perror("connect error");
    }
    return ret;
}

int Socketpair(int sv[2]) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
        perror("socketpair");
        return -1;
    }
    return 0;
}

