#include "sysutil.h"
#include "session.h"
#include "parse_conf.h"
#include "configure.h"
#include "ftp_codes.h"
#include "command_map.h"
#define     LISTENPORT      9981

extern session_t *p_sess;
unsigned int num_of_clients = 0;    //连接数目
void handle_sigchld(int sig);
void limit_num_clients(session_t *sess);
int main(int argc, char *argv[])
{

    if (getuid()) {
        fprintf(stderr, "the ftpserver must started by root\n");
        exit(EXIT_FAILURE); 
    }

    if (signal(SIGCHLD, handle_sigchld) == SIG_ERR)
        ERR_EXIT("signal");
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
        num_of_clients++;
        session_init(&sess);
        p_sess = &sess;
        pid = fork();
        if (pid == -1) {
            ERR_EXIT("fork error");
        } else if (pid == 0) {   //子进程
            close(listenfd); 
            sess.peerfd = connfd;
            sess.curr_clients = num_of_clients;
            session_begin(&sess);   //建立一个会话
            exit(EXIT_SUCCESS);
        } else {                //父进程
            close(connfd);
        }
    }
    return 0;
}

void handle_sigchld(int sig)
{
    pid_t   pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        --num_of_clients;
    }
}

void limit_num_clients(session_t *sess)
{
    if (max_clients > 0 && sess->curr_clients > max_clients) {
        ftp_reply(sess, FTP_TOO_MANY_USERS, "There are too many users, please try again later");
        exit(EXIT_FAILURE);        
    }
}
