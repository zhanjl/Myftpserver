#include "ftp_proto.h"
#include "common.h"
#include "sysutil.h"
#include "strutil.h"
#include "ftp_codes.h"

void ftp_reply(session_t *sess, int status, const char *context);
void do_user(session_t *sess);
void do_pass(session_t *sess);
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

        if (strcmp(sess->comm, "USER") == 0) {
            do_user(sess);
        }

        if (strcmp(sess->comm, "PASS") == 0) {
            do_pass(sess);
        }
    }  
}

void ftp_reply(session_t *sess, int status, const char *context)
{
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d %s\r\n", status, context);
    writen(sess->peerfd, buf, strlen(buf));
}

void do_user(session_t *sess)
{
    struct passwd *pw;
    if ((pw = getpwnam(sess->args)) == NULL) {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    sess->user_uid = pw->pw_uid;
    ftp_reply(sess, FTP_GIVEPWORD, "Please input the password");
}

void do_pass(session_t *sess)
{
    struct passwd *pwd;
    if ((pwd = getpwuid(sess->user_uid)) == NULL) {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }    

    struct spwd *spd;
    if ((spd = getspnam(pwd->pw_name)) == NULL) {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    char *encrypted_password = crypt(sess->args, spd->sp_pwdp);
    if (encrypted_password == NULL) {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }    

    if (strcmp(encrypted_password, spd->sp_pwdp) != 0) {
        ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
        return;
    }

    if (setegid(pwd->pw_gid) == -1)
        ERR_EXIT("setegid");
    if (seteuid(pwd->pw_uid) == -1)
        ERR_EXIT("setegid");

    ftp_reply(sess, FTP_LOGINOK, "Login successful.");
}
