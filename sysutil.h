#ifndef SYS_UTIL_H
#define SYS_UTIL_H

#include "common.h"

//返回一个监听套接字，监听的地址是host和port
int tcp_server(const char *host, unsigned short port);

//获取本机的ip地址
int getlocalip(char *ip);

//把fd设为阻塞或非阻塞
void activate_nonblock(int fd);     //把fd设为非阻塞模式
void deactivate_nonblock(int fd);   //把fd设为阻塞模式

//设置超时IO函数

//判断在waitseconds内fd是否可读，是返回0,否返回-1
int read_timeout(int fd, unsigned int waitseconds);

//判断在waitseconds内fd是否可写，是返回0,否返回-1
int write_timeout(int fd, unsigned int waitseconds);

//判断在waitseconds是否有连接到来，有返回已建立连接套接字描述符
//没有返回-1
int accept_time_out(int fd, struct sockaddr_in *addr, int waitseconds);

//在waitseconds是否建立连接，是返回0,否则返回-1
int connect_time_out(int fd, struct sockaddr_in *addr, int waitseconds);


//精确控制的IO读写函数
size_t readn(int fd, char *buf, int count);
size_t writen(int fd, const char *buf, int count);
size_t readline(int fd, char *buf, int maxline);


//接受和发送套接字描述符

#endif  /*SYS_UTIL_H*/
