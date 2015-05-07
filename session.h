#ifndef SESSION_H
#define SESSION_H

#include "common.h"
#define MAX_COMMAND 1024
typedef struct {
    
    char    command[MAX_COMMAND];
    char    comm[MAX_COMMAND];         //FTP命令
    char    args[MAX_COMMAND];         //命令的参数

    int     peerfd;     //和客户端连接的套接字

    int     nobody_fd;  //nobody进程使用的fd
    int     proto_fd;   //proto进程使用的fd

    uid_t   user_uid;   //登陆的用户ID

    int     ascii_mode; //文件类型
    struct sockaddr_in *p_addr; //客户端发来的地址
    int     sockfd;     //数据传输fd
    int     listenfd;   //监听套接字，用于PASV模式
} session_t;

//初始化session
void session_init(session_t *sess);

//将三个字符数组清0
void session_reset_command(session_t *sess);

//开始一个会话
void session_begin(session_t *sess);
#endif  //SESSION_H
