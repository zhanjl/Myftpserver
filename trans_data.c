#include "trans_data.h"
#include "common.h"
#include "sysutil.h"
#include "ftp_codes.h"
#include "configure.h"
#include "command_map.h"
#include "priv_sock.h"
const char *statbuf_get_perms(struct stat *sbuf);
const char *statbuf_get_date(struct stat *sbuf);
const char *statbuf_get_filename(struct stat *sbuf, const char* name);
const char *statbuf_get_user_info(struct stat *sbuf);
const char *statbuf_get_size(struct stat *sbuf);

void trans_lists(session_t *sess, int level)
{
    DIR *dir = opendir(".");    //打开当前目录文件
    if (dir == NULL)
        ERR_EXIT("opendir");

    struct dirent *dr;
    while ((dr = readdir(dir))) {   //读取目录文件中的每一项
        const char *filename = dr->d_name;
        if (filename[0] == '.')      //忽略.和..这两项
            continue;

        char buf[1024] = {0};
        struct stat sbuf;

        if (lstat(filename, &sbuf) == -1) {
            ERR_EXIT("lstat");
        }
        if (level == 1) {
            strcpy(buf, statbuf_get_perms(&sbuf));
            strcat(buf, " ");
            strcat(buf, statbuf_get_user_info(&sbuf));
            strcat(buf, " ");
            strcat(buf, statbuf_get_date(&sbuf));
            strcat(buf, " ");
            strcat(buf, statbuf_get_size(&sbuf));
            strcat(buf, " ");
        }
        strcat(buf, statbuf_get_filename(&sbuf, filename));
        strcat(buf, "\r\n");

        writen(sess->sockfd, buf, strlen(buf));
    }
    closedir(dir);
}

const char *statbuf_get_perms(struct stat *sbuf)
{
    static char perms[] = "----------";
    mode_t mode = sbuf->st_mode;

    //文件类型
    switch(mode & S_IFMT)
    {
        case S_IFSOCK:
            perms[0] = 's';
            break;
        case S_IFLNK:
            perms[0] = 'l';
            break;
        case S_IFREG:
            perms[0] = '-';
            break;
        case S_IFBLK:
            perms[0] = 'b';
            break;
        case S_IFDIR:
            perms[0] = 'd';
            break;
        case S_IFCHR:
            perms[0] = 'c';
            break;
        case S_IFIFO:
            perms[0] = 'p';
            break;
    }

    //权限
    if(mode & S_IRUSR)
        perms[1] = 'r';
    if(mode & S_IWUSR)
        perms[2] = 'w';
    if(mode & S_IXUSR)
        perms[3] = 'x';
    if(mode & S_IRGRP)
        perms[4] = 'r';
    if(mode & S_IWGRP)
        perms[5] = 'w';
    if(mode & S_IXGRP)
        perms[6] = 'x';
    if(mode & S_IROTH)
        perms[7] = 'r';
    if(mode & S_IWOTH)
        perms[8] = 'w';
    if(mode & S_IXOTH)
        perms[9] = 'x';

    if(mode & S_ISUID)
        perms[3] = (perms[3] == 'x') ? 's' : 'S';
    if(mode & S_ISGID)
        perms[6] = (perms[6] == 'x') ? 's' : 'S';
    if(mode & S_ISVTX)
        perms[9] = (perms[9] == 'x') ? 't' : 'T';

    return perms;
}

const char* statbuf_get_date(struct stat *sbuf)
{
    static char datebuf[1024] = {0};
    struct tm *ptm;
    time_t ct = sbuf->st_ctime;

    if ((ptm = localtime(&ct)) == NULL)
        ERR_EXIT("localtime");

    const char *format = "%b %e %H:%M"; //时间格式
    if (strftime(datebuf, sizeof(datebuf), format, ptm) == 0) {
        fprintf(stderr, "strftime error\n");
        exit(EXIT_FAILURE); 
    }

    return datebuf;
}

const char* statbuf_get_filename(struct stat *sbuf, const char *name)
{
    static char filename[1024] = {0};
    
    //处理符号连接文件
    if (S_ISLNK(sbuf->st_mode)) {
        char linkfile[1024] = {0};
        if (readlink(name, linkfile, sizeof(linkfile)) == -1)
            ERR_EXIT("readlink");
        snprintf(filename, sizeof(filename), "%s -> %s", name, linkfile);
    } else {
        strcpy(filename, name);
    }

    return filename;
}


const char *statbuf_get_user_info(struct stat *sbuf)
{
    static char info[1024] = {0};
    snprintf(info, sizeof info, " %3d %8d %8d", sbuf->st_nlink, sbuf->st_uid, sbuf->st_gid);

    return info;
}

const char *statbuf_get_size(struct stat *sbuf)
{
    static char buf[100] = {0};
    snprintf(buf, sizeof buf, "%8lu", (unsigned long)sbuf->st_size);
    return buf;
}

static int is_port_active(session_t *sess)
{
    return sess->p_addr != NULL;
}

//被动模式
static int is_pasv_active(session_t *sess)
{
    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_PASV_ACTIVE);
    return priv_sock_recv_int(sess->proto_fd); 
}

static void get_port_data_fd(session_t *sess);
static void get_pasv_data_fd(session_t *sess);
//建立连接
int get_trans_data_fd(session_t *sess)
{
    int is_port = is_port_active(sess);
    int is_pasv = is_pasv_active(sess);

    if (!is_port && !is_pasv) {
        ftp_reply(sess, FTP_BADSENDCONN, "Use PORT or PASV first.");
        return -1;
    }
    
    if (is_port && is_pasv) {
        fprintf(stderr, "Both port and pasv are active\n");
        exit(EXIT_FAILURE);
    }
    //主动模式
    if (is_port) {
        get_port_data_fd(sess);
    } else if (is_pasv) {   //被动模式
        get_pasv_data_fd(sess);
    }

    return 0;
}

static void get_port_data_fd(session_t *sess)
{
    //向nobody进程发送命令
    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_GET_DATA_SOCK); 
    //发送客户端监听套接字的地址，由nobody进程负责建立连接
    char *ip = inet_ntoa(sess->p_addr->sin_addr);
    uint16_t port = ntohs(sess->p_addr->sin_port);
    priv_sock_send_str(sess->proto_fd, ip, strlen(ip));
    priv_sock_send_int(sess->proto_fd, port);

    //接受nobody进程的应答
    char result = priv_sock_recv_result(sess->proto_fd);
    if (result == PRIV_SOCK_RESULT_BAD) {
        ftp_reply(sess, FTP_BADCMD, "get port data_fd error");
        fprintf(stderr, "get data fd error\n");
        exit(EXIT_FAILURE);
    }
    sess->sockfd = priv_sock_recv_fd(sess->proto_fd);
    free(sess->p_addr);
    sess->p_addr = NULL;

}

static void get_pasv_data_fd(session_t *sess)
{

    priv_sock_send_cmd(sess->proto_fd, PRIV_SOCK_PASV_ACCEPT);
    
    char res = priv_sock_recv_result(sess->proto_fd);
    if (res == PRIV_SOCK_RESULT_BAD) {
        ftp_reply(sess, FTP_BADCMD, "get pasv data_fd error");
        fprintf(stderr, "get data fd error\n");
        exit(EXIT_FAILURE);
    }

    sess->sockfd = priv_sock_recv_fd(sess->proto_fd);
}
