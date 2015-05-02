#include "command_map.h"
#include "common.h"
#include "ftp_codes.h"
#include "sysutil.h"

typedef struct ftpcmd
{
    const char *cmd;
    void (*cmd_handler)(session_t *sess);
} ftpcmd_t;

//命令映射图
static ftpcmd_t ctr_cmds[] = {
    { "USER", do_user },
    { "PASS", do_pass },
    { "TYPE", do_type },
    { "PWD", do_pwd },
    { "FEAT", do_feat },
    { "SYST", do_syst },
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

void ftp_lreply(session_t *sess, int status, const char *context)
{
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%d-%s\r\n", status, context);
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

void do_type(session_t *sess)
{
    if (strcmp(sess->args, "A") == 0) {
        sess->ascii_mode = 1;
        ftp_reply(sess, FTP_TYPEOK, "Switching to ASCII mode.");
    } else if (strcmp(sess->args, "I") == 0) {
        sess->ascii_mode = 0;
        ftp_reply(sess, FTP_TYPEOK, "Switching to Binary mode.");
    } else {
        ftp_reply(sess, FTP_BADCMD, "Unrecongnised TYPE command.");
    }
}

void do_pwd(session_t *sess)
{
    char tmp[1024];
    if (getcwd(tmp, sizeof(tmp)) == NULL) {
        fprintf(stderr, "get cwd error\n");
        ftp_reply(sess, FTP_BADMODE, "error");
        return;
    }

    char text[1024];
    snprintf(text, sizeof text, "\"%s\"", tmp);
    ftp_reply(sess, FTP_PWDOK, text);
}

void do_feat(session_t *sess)
{
    ftp_lreply(sess, FTP_FEAT, "Features:");

    writen(sess->peerfd, " EPRT\r\n", strlen(" EPRT\r\n"));
    writen(sess->peerfd, " EPSV\r\n", strlen(" EPSV\r\n"));
    writen(sess->peerfd, " MDTM\r\n", strlen(" MDTM\r\n"));
    writen(sess->peerfd, " PASV\r\n", strlen(" PASV\r\n"));
    writen(sess->peerfd, " REST STREAM\r\n", strlen(" REST STREAM\r\n"));
    writen(sess->peerfd, " SIZE\r\n", strlen(" SIZE\r\n"));
    writen(sess->peerfd, " TVFS\r\n", strlen(" TVFS\r\n"));
    writen(sess->peerfd, " UTF8\r\n", strlen(" UTF8\r\n"));

    ftp_reply(sess, FTP_FEAT, "End");
}

void do_syst(session_t *sess)
{
    ftp_reply(sess, FTP_SYSTOK, "UNIX Type: L8");
}
