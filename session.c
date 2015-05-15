#include "common.h"
#include "session.h"
#include "ftp_proto.h"
#include "ftp_nobody.h"
#include "priv_sock.h"
#include "configure.h"
void session_init(session_t *sess)
{
    memset(sess->command, 0, sizeof(sess->command));
    memset(sess->comm, 0, sizeof(sess->comm));
    memset(sess->args, 0, sizeof(sess->args));
    sess->peerfd = -1;
    sess->nobody_fd = -1;
    sess->proto_fd = -1;
    sess->user_uid = 0;
    sess->ascii_mode = 0;
    sess->p_addr = NULL;
    sess->sockfd = -1;
    sess->listenfd = -1;
    sess->restartpos = 0;
    sess->rnfr_name = NULL;

    sess->limits_max_upload = upload_max_rate;
    sess->limits_max_download = download_max_rate;
    sess->start_time_sec = 0;
    sess->start_time_usec = 0;
    sess->has_translate_data = 0;
    sess->is_receive_abort = 0;
    sess->curr_clients = 0;
}

void session_reset_command(session_t *sess)
{
    memset(sess->command, 0, sizeof(sess->command)); 
    memset(sess->comm, 0, sizeof(sess->comm)); 
    memset(sess->args, 0, sizeof(sess->args)); 
}

//主进程的子进程会调用这个函数
void session_begin(session_t *sess)
{
    priv_sock_init(sess);
    pid_t pid;

    pid = fork();
    if (pid == -1) {
        ERR_EXIT("fork error");
    } else if (pid != 0) {  //nobody进程 父进程
        priv_sock_set_nobody_context(sess);
        handle_nobody(sess);
    } else {                //proto进程  子进程
        priv_sock_set_proto_context(sess);
        handle_proto(sess);
    }

}
