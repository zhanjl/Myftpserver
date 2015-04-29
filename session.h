#ifndef SESSION_H
#define SESSION_H

typedef struct {
    
    char    command[1024];
    char    comm[1024];         //FTP命令
    char    args[1024];         //命令的参数

    int     peerfd;     //和客户端连接的套接字

    int     nobody_fd;  //nobody进程使用的fd
    int     proto_fd;   //proto进程使用的fd
} session_t;

//初始化session
void session_init(session_t *sess);

//开始一个会话
void session_begin(session_t *sess);
#endif  //SESSION_H