#include "ftp_proto.h"
#include "common.h"
#include "sysutil.h"
#include "strutil.h"
void handle_proto(session_t *sess)
{
    //向客户端发送初始化信息
    writen(sess->peerfd, "220 (miniftp 0.1)\r\n", strlen("220 (miniftp 0.1)\r\n"));
    
    while (1) {
        //读客户端发来的命令，并解析
        int ret = readline(sess->peerfd, sess->command, MAX_COMMAND);
        if (ret == 0) {
            printf("server process %d exit\n", getpid());
            exit(EXIT_SUCCESS);
        }
        sess->command[ret] = '\0';
        printf("receive data: %s", sess->command);
        str_trim_crlf(sess->command);
        str_split(sess->command, sess->comm, sess->args, ' ');
        printf("COMMD=[%s], ARGS=[%s]\n", sess->comm, sess->args);
    }  
}
