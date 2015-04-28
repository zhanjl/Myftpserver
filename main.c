#include "sysutil.h"
#include "session.h"

#define     LISTENPORT      9981
int main(int argc, char *argv[])
{

    if (getuid()) {
        fprintf(stderr, "the ftpserver must started by root\n");
        exit(EXIT_FAILURE); 
    }

    int listenfd; 
    listenfd = tcp_server("127.0.0.1", LISTENPORT);
    
    printf("tcp_server success, listenfd = %d\n", listenfd);
    int connfd;
    pid_t   pid;
    session_t sess;
    while (1) {
          
        connfd = accept_timeout(listenfd, NULL, 10);
        if (connfd == -1) {
            printf("don't has connection in 10 seconds\n");
            continue;
        }
        printf("connect success\n");
        session_init(&sess);
        sess.peerfd = connfd;
        pid = fork();
        if (pid == -1) {
            ERR_EXIT("fork error");
        } else if (pid == 0) {   //子进程
            close(listenfd); 
            session_begin(&sess);   //建立一个会话
        } else {                //父进程
            close(connfd);
        }
    }
    return 0;
}
