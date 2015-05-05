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

                break;
            case PRIV_SOCK_PASV_ACTIVE:

                break;
            case PRIV_SOCK_PASV_LISTEN:

                break;
            case PRIV_SOCK_PASV_ACCEPT:

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
