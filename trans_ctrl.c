#include "common.h"
#include "trans_ctrl.h"
#include "sysutil.h"
#include "session.h"
#include "configure.h"
#include "ftp_codes.h"
#include "command_map.h"
#include "strutil.h"

session_t *p_sess = NULL;

static void handle_signal_alarm(int signo);

void limit_curr_rate(session_t *sess, int nbytes, int is_upload)
{
    int curr_time_sec = get_cur_time_sec();
    int curr_time_usec = get_cur_time_usec();

    double elapsed = 0.0;
    elapsed += (curr_time_sec - sess->start_time_sec);
    elapsed += (curr_time_usec - sess->start_time_usec) / (double)1000000;
    
    if (elapsed < 0.000001)     //小于1毫秒
        elapsed = 0.001;

    double curr_rate = nbytes / elapsed;    //当前传输速度

    double rate_radio = 0.0;    //用来计算睡眠时间
    if (is_upload) {            //上传限速
        //需要进行限速
        if (sess->limits_max_upload > 0 && curr_rate > sess->limits_max_upload) {
            rate_radio = curr_rate / sess->limits_max_upload; 
        } else {    //不需要进行限速
            sess->start_time_sec = get_cur_time_sec(); 
            sess->start_time_usec = get_cur_time_usec();
            return;
        }
    } else {                    //下载限速
        //需要进行限速
        if (sess->limits_max_download > 0 && curr_rate > sess->limits_max_download) {
            rate_radio = curr_rate / sess->limits_max_download; 
        } else {    //不需要进行限速
            sess->start_time_sec = get_cur_time_sec(); 
            sess->start_time_usec = get_cur_time_usec();
            return;
        }
    }

    //计算需要睡眠的时间
    double sleep_time = (rate_radio - 1) * elapsed;

    if (nano_sleep(sleep_time) == -1)
        ERR_EXIT("nano_sleep");

    sess->start_time_sec = get_cur_time_sec(); 
    sess->start_time_usec = get_cur_time_usec();
}

void setup_signal_alarm()
{
    if (signal(SIGALRM, handle_signal_alarm) == SIG_ERR)
        ERR_EXIT("signal");
}

void start_signal_alarm()
{
    alarm(idle_session_timeout);
}

static void handle_signal_alarm(int signo)
{
    if (idle_session_timeout > 0 && p_sess->has_translate_data == 0) {
        if (p_sess->sockfd != -1)   //打开了数据连接
            close(p_sess->sockfd);

        ftp_reply(p_sess, FTP_IDLE_TIMEOUT, "Timeout.");
        close(p_sess->peerfd);
        exit(EXIT_SUCCESS);
    }
    p_sess->has_translate_data = 0;
    start_signal_alarm();
}

static void handle_signal_sigurg(int signo);
void setup_signal_sigurg()
{
    if (signal(SIGURG, handle_signal_sigurg) == SIG_ERR)
        ERR_EXIT("signal");
}

static void handle_signal_sigurg(int signo)
{
    char    cmdline[1024] = {0};
    int ret = readline(p_sess->peerfd, cmdline, sizeof(cmdline));
    if (ret <= 0)
       ERR_EXIT("readline");
    
    str_trim_crlf(cmdline);
    str_upper(cmdline);
    
    if (strcmp("ABOR", cmdline) == 0 ||
           strcmp("\377\364\377\362ABOR", cmdline) == 0) {
        p_sess->is_receive_abort = 1;
        close(p_sess->sockfd);
        p_sess->sockfd = -1;
    } else {
        ftp_reply(p_sess, FTP_BADCMD, "Unkown command");
    }
}

void do_site_chmod(session_t *sess, char *args)
{
    if (strlen(args) == 0) {
        ftp_reply(sess, FTP_BADCMD, "SITE CHMOD need 2 arguments.");
        return;
    }
    char perm[100] = {0};
    char file[100] = {0};
    str_split(args, perm, file, ' ');
    if (strlen(file) == 0) {
        ftp_reply(sess, FTP_BADCMD, "SITE CHMOD need 2 arguments.");
        return;
    }

    unsigned int mode = str_octal_to_uint(perm);
    if (chmod(file, mode) < 0) {
        ftp_reply(sess, FTP_CHMODOK, "SITE CHMOD command failed.");
    } else {
        ftp_reply(sess, FTP_CHMODOK, "SITE CHMOD command success.");
    }
}
void do_site_umask(session_t *sess, char *args)
{
    if (strlen(args) == 0) {
        char text[1024] = {0};
        sprintf(text, "Your current UMASK is 0%o", local_umask);
        ftp_reply(sess, FTP_UMASKOK, text);
    } else {
        unsigned int um = str_octal_to_uint(args);
        umask(um);
        char text[1024] = {0};
        sprintf(text, "UMASK set to 0%o", um);
        ftp_reply(sess, FTP_UMASKOK, text);
    }
}
void do_site_help(session_t *sess)
{
    ftp_reply(sess, FTP_HELP, "CHMOD UMASK HELP");
}
