#include "trans_data.h"
#include "common.h"
#include "sysutil.h"
#include "ftp_codes.h"
#include "configure.h"
#include "command_map.h"
const char *statbuf_get_perms(struct stat *sbuf);
const char *statbuf_get_date(struct stat *sbuf);
const char *statbuf_get_filename(struct stat *sbuf, const char* name);
const char *statbuf_get_user_info(struct stat *sbuf);
const char *statbuf_get_size(struct stat *sbuf);

void trans_lists(session_t *sess)
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

        strcpy(buf, statbuf_get_perms(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_user_info(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_date(&sbuf));
        strcat(buf, " ");
        strcat(buf, statbuf_get_size(&sbuf));
        strcat(buf, " ");
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

int is_port_active(session_t *sess)
{
    return sess->p_addr != NULL;
}

//被动模式，现在还没设计这个功能
int is_pasv_active(session_t *sess)
{
    return 0;
}

int get_trans_data_fd(session_t *sess)
{
    int is_port = is_port_active(sess);
    int is_pasv = is_pasv_active(sess);

    if (!is_port && !is_pasv) {
        ftp_reply(sess, FTP_BADSENDCONN, "Use PORT or PASV first.");
        return -1;
    }
    
    //主动模式
    if (is_port) {
        int data_fd;
        if ((data_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) 
            ERR_EXIT("socket");
        int ret;
        ret = connect_time_out(data_fd, sess->p_addr, connect_timeout);
        if (ret == -1)  //连接超时
            ERR_EXIT("connect_time_out");

        sess->sockfd = data_fd;

        free(sess->p_addr);
        sess->p_addr = NULL;
    }

    return 0;
}
