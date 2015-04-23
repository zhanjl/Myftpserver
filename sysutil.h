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

//
#endif  /*SYS_UTIL_H*/
