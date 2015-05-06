#include "priv_sock.h"
#include "sysutil.h"
#include "common.h"

void priv_sock_init(session_t *sess)
{
    int fd[2];
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, fd) < 0)
        ERR_EXIT("socketpair error");        
     
    sess->nobody_fd = fd[0];
    sess->proto_fd = fd[1];
}

void priv_sock_close(session_t *sess)
{
    if (sess->nobody_fd != -1)
        close(sess->nobody_fd);
    if (sess->proto_fd != -1)
        close(sess->proto_fd);
}

void priv_sock_set_nobody_context(session_t *sess)
{
    if (sess->proto_fd != -1) {
        close(sess->proto_fd);
        sess->proto_fd = -1;
    }
}

void priv_sock_set_proto_context(session_t *sess)
{
    if (sess->nobody_fd != -1) {
        close(sess->nobody_fd);
        sess->nobody_fd = -1;
    }
}

void priv_sock_send_cmd(int fd, char cmd)
{
    if (writen(fd, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        fprintf(stderr, "priv_sock_send_cmd error\n");
        exit(EXIT_FAILURE); 
    }
}

char priv_sock_recv_cmd(int fd)
{
    char res;
    int  ret;

    ret = readn(fd, &res, sizeof(res));

    if (ret == 0) {
        printf("Proto close!\n");
        exit(EXIT_FAILURE);
    }

    if (ret != sizeof(res)) {
        fprintf(stderr, "priv_sock_recv_cmd error");
        exit(EXIT_FAILURE);
    }
    return res;
}

void priv_sock_send_result(int fd, char res)
{
    if (writen(fd, &res, sizeof(res)) != sizeof(res)) {
        fprintf(stderr, "priv_sock_send_cmd error\n");
        exit(EXIT_FAILURE); 
    }
}

char priv_sock_recv_result(int fd)
{
    char res;
    if (readn(fd, &res, sizeof(res)) != sizeof(res)) {
        fprintf(stderr, "priv_sock_recv_cmd error");
        exit(EXIT_FAILURE);
    }
    return res;
}


void priv_sock_send_int(int fd, int the_int)
{
    if (writen(fd, (const char*)&the_int, sizeof(the_int)) != sizeof(the_int)) {
        fprintf(stderr, "priv_sock_send_cmd error\n");
        exit(EXIT_FAILURE); 
    }
}

int priv_sock_recv_int(int fd)
{
    int res;
    if (readn(fd, (char*)&res, sizeof(res)) != sizeof(res)) {
        fprintf(stderr, "priv_sock_recv_cmd error");
        exit(EXIT_FAILURE);
    }
    return res;
}


void priv_sock_send_str(int fd, const char *buf, unsigned len)
{
    priv_sock_send_int(fd, len);
    if (writen(fd, buf, len) != len) {
        fprintf(stderr, "priv_sock_send_str error");
        exit(EXIT_FAILURE);
    }
}

void priv_sock_recv_str(int fd, char *buf, unsigned len)
{
    unsigned int recv_len = priv_sock_recv_int(fd);
    if (recv_len > len) {
        fprintf(stderr, "priv_sock_recv_str error");
        exit(EXIT_FAILURE);
    }

    if (readn(fd, buf, recv_len) != recv_len) {
        fprintf(stderr, "priv_sock_recv_str error");
        exit(EXIT_FAILURE);
    }
}

void priv_sock_send_fd(int sockfd, int fd)
{
    send_fd(sockfd, fd);
}

int priv_sock_recv_fd(int sockfd)
{
    int ret;
    ret = recv_fd(sockfd);
    return ret;
}
