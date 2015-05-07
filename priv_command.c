#include "priv_command.h"
#include "priv_sock.h"
#include "sysutil.h"
#include "configure.h"
#include "common.h"

void privop_pasv_get_data_sock(session_t *sess)
{
    char ip[16] = {0};
    priv_sock_recv_str(sess->nobody_fd, ip, sizeof(ip));
    uint16_t port = priv_sock_recv_int(sess->nobody_fd);

    int data_fd;
    if ((data_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        ERR_EXIT("socket");

    struct sockaddr_in  addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    int ret = connect_time_out(data_fd, &addr, connect_timeout);
    if (ret == -1)
       ERR_EXIT("connect_time_out"); 
    priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_OK);
    priv_sock_send_fd(sess->nobody_fd, data_fd);
    close(data_fd);
}

void privop_pasv_active(session_t *sess)
{
    priv_sock_send_int(sess->nobody_fd, sess->listenfd != -1); 
}

void privop_pasv_listen(session_t *sess)
{
    char ip[16];
    if (getlocalip(ip) == -1)
        ERR_EXIT("getlocalip");

    int listenfd;
    listenfd = tcp_server(ip, 0);   //建立监听套接字 一般要设为20，这点要注意
    sess->listenfd = listenfd;
    struct sockaddr_in  addr;
    socklen_t len = sizeof (addr);
    if (getsockname(listenfd, (struct sockaddr*)&addr, &len) == -1)
        ERR_EXIT("getsockname");

    priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_OK);

    uint16_t net_port = ntohs(addr.sin_port);
    priv_sock_send_int(sess->nobody_fd, net_port);
}

void privop_pasv_accept(session_t *sess)
{
    int peerfd = accept_time_out(sess->listenfd, NULL, accept_timeout);
    close(sess->listenfd);
    sess->listenfd = -1;
    if (peerfd == -1) {
        priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_BAD);
        exit(EXIT_FAILURE);
    }

    priv_sock_send_result(sess->nobody_fd, PRIV_SOCK_RESULT_OK);
    priv_sock_send_fd(sess->nobody_fd, peerfd);
    close(peerfd);
}
