#include "common.h"
#include "session.h"
#include "ftp_proto.h"
#include "ftp_nobody.h"

void session_init(session_t *sess)
{
    memset(sess->command, 0, sizeof(sess->command));
    memset(sess->comm, 0, sizeof(sess->comm));
    memset(sess->args, 0, sizeof(sess->args));
    //memset(sess, 0, sizeof(struct session_t));
    sess->peerfd = -1;
    sess->nobody_fd = -1;
    sess->proto_fd = -1;
    sess->ascii_mode = 0;
}

//主进程的子进程会调用这个函数
void session_begin(session_t *sess)
{
    int fd[2];
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, fd) < 0)
        ERR_EXIT("socketpair error");        

    pid_t pid;

    pid = fork();
    if (pid == -1) {
        ERR_EXIT("fork error");
    } else if (pid != 0) {  //nobody进程 父进程
        close(fd[0]);
        sess->nobody_fd = fd[1];
        handle_nobody(sess);
    } else {                //proto进程  子进程
        close(fd[1]);
        sess->proto_fd = fd[0];
        handle_proto(sess);
    }

}
