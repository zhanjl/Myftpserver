#include "sysutil.h"
#include "session.h"
#include "parse_conf.h"
#include "configure.h"
#define     LISTENPORT      9981

//void printconfig(); //测试解析配置文件函数
int main(int argc, char *argv[])
{

    if (getuid()) {
        fprintf(stderr, "the ftpserver must started by root\n");
        exit(EXIT_FAILURE); 
    }

    parse_load_file("ftpserver.conf");
    printf("parse_load_file success\n");
//    printconfig();

    int listenfd; 
    listenfd = tcp_server(listen_address, listen_port);
    
    printf("tcp_server success, listenfd = %d\n", listenfd);
    int connfd;
    pid_t   pid;
    session_t sess;
    while (1) {
          
        connfd = accept_time_out(listenfd, NULL, accept_timeout);
        if (connfd == -1) {
        //    printf("don't has connection in %d seconds\n", accept_timeout);
            continue;
        }
        printf("connect success\n");
        session_init(&sess);
        pid = fork();
        if (pid == -1) {
            ERR_EXIT("fork error");
        } else if (pid == 0) {   //子进程
            close(listenfd); 
            sess.peerfd = connfd;
            session_begin(&sess);   //建立一个会话
            exit(EXIT_SUCCESS);     //此时是nobody 进程
        } else {                //父进程
            close(connfd);
        }
    }
    return 0;
}
//void printconfig()
//{
//    printf("pasv_enable = %d\n", pasv_enable);
//    printf("listen_port = %d\n", listen_port);
//    printf("accept_timeout = %d\n", accept_timeout);
//
//    printf("local_umask = %d\n", local_umask);
//    printf("max_clients = %d\n", max_clients);
//    printf("listen_address = %s\n", listen_address);
//}
