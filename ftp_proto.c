#include "ftp_proto.h"
#include "common.h"
#include "sysutil.h"
void handle_proto(session_t *sess)
{
    //向客户端发送初始化信息
    writen(sess->peerfd, "220 (miniftp 0.1)\r\n", strlen("220 (miniftp 0.1)\r\n"));
    
    while (1) {
        //读客户端发来的命令，并解析

        for ( ; ; )
            pause();
    }  
}
