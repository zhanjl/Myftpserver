#include "ftp_nobody.h"
#include "common.h"
#include "sysutil.h"
#include "priv_sock.h"
#include "priv_command.h"

void set_nobody();

void handle_nobody(session_t *sess)
{
    set_nobody();
    
    char cmd;
    
    while (1) {
        cmd = priv_sock_recv_cmd(sess->nobody_fd);
        switch (cmd)    //设置不同的命令处理函数
        {
            case PRIV_SOCK_GET_DATA_SOCK: 
                //主动连接客户端，并把连接套接字发送给proto进程
                privop_pasv_get_data_sock(sess);   
                break;
            case PRIV_SOCK_PASV_ACTIVE:
                privop_pasv_active(sess);
                break;
            case PRIV_SOCK_PASV_LISTEN:
                privop_pasv_listen(sess);   //在nobody进程中建立监听套接字
                break;
            case PRIV_SOCK_PASV_ACCEPT:
                privop_pasv_accept(sess);
                break;
            default:
                fprintf(stderr, "Unkown command\n");
                exit(EXIT_FAILURE);
        }
    }
}

void set_nobody()
{
    struct passwd *pw;
    if ((pw = getpwnam("nobody")) == NULL)
        ERR_EXIT("getpwnam");

    if (setegid(pw->pw_gid) == -1)
        ERR_EXIT("setegid");

    if (seteuid(pw->pw_uid) == -1)
        ERR_EXIT("seteuid");

}
