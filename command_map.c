#include "command_map.h"
#include "common.h"
#include "ftp_codes.h"
#include "sysutil.h"

typedef struct ftpcmd
{
    const char *cmd;
    void (*cmd_handler)(session_t *sess);
} ftpcmd_t;

static ftpcmd_t ctr_cmds[] = {
    { "USER", do_user },
    { "PASS", do_pass },
    { NULL, NULL },
};


void do_command_map(session_t *sess)
{
    int i;
    i = 0;
    while (ctr_cmds[i].cmd) {
        if (strcmp(ctr_cmds[i].cmd, sess->comm) == 0) {
            ctr_cmds[i].cmd_handler(sess);
            break;
        }
        i++;
    }
    
    //没有找到对应的命令
    if (ctr_cmds[i].cmd == NULL) {
        ftp_reply(sess, FTP_BADCMD, "Unknown command.");
    }

    return;
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
