#include "sysutil.h"

//host为IP地址，port是端口号，返回一个对应的监听套接字，监听的地址就是host和port
int tcp_server(const char *host, unsigned short port)
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR_EXIT("socket error");        
    }

    struct sockaddr_in  servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_aton(host, &servaddr.sin_addr) < 0) {
        ERR_EXIT("inet_aton error");
    }

    int on = 1;
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on))) < 0) {
        ERR_EXIT("setsockopt error");
    }

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind error");
    }

    if (listen(sockfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen error");
    }
    
    return sockfd;
}

//ip指向一个已分配足够大小的空间
int getlocalip(char *ip)
{
    char name[100];
    if (gethostname(name, 100) < 0) {
        return -1;
    }

    struct hostent *addr;
    if ((addr = gethostbyname(name)) == NULL)
        return -1;

    strcpy(ip, inet_ntoa(*(struct in_addr*)(*(addr->h_addr_list))));
    return 0;
}

//设为非阻塞模式
void activate_nonblock(int fd)
{
    int ret;

    if ((ret = fcntl(fd, F_GETFL, 0)) < 0)
        ERR_EXIT("fcntl error");

    ret |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, ret) < 0)
        ERR_EXIT("fnctl error");
}

//设为阻塞模式
void deactivate_nonblock(int fd)
{
    int ret;

    if ((ret = fcntl(fd, F_GETFL, 0)) < 0)
        ERR_EXIT("fcntl error");

    ret &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, ret) < 0)
        ERR_EXIT("fnctl error");
}


