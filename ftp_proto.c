#include "ftp_proto.h"
#include "common.h"
#include "sysutil.h"
#include "strutil.h"
#include "ftp_codes.h"
#include "command_map.h"

void handle_proto(session_t *sess)
{
    //向客户端发送初始化信息
    ftp_reply(sess, FTP_GREET, "mini ftpserver0.1"); 
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

        do_command_map(sess);
    }
}
