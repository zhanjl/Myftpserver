#include "sysutil.h"

#define     LISTENPORT      9981
int main(int argc, char *argv[])
{

    int listenfd; 
    listenfd = tcp_server("127.0.0.1", LISTENPORT);
    
    printf("tcp_server success, listenfd = %d\n", listenfd);
    int connfd;
    pid_t   pid;
    while (1) {
          
        connfd = accept_timeout(listenfd, NULL, 10);
        if (connfd == -1) {
            printf("don't has connection in 10 seconds\n");
            continue;
        }
        printf("connect success\n");
        pid = fork();
        if (pid == -1) {
            ERR_EXIT("fork error");
        } else if (pid == 0) {   //child
            printf("child\n");
            close(listenfd); 
            for (; ; )
                pause();
        } else {                //father
            printf("father\n");
            close(connfd);
        }
    }
    return 0;
}
