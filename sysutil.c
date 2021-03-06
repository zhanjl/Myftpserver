#include "sysutil.h"

//host为IP地址，port是端口号，返回一个对应的监听套接字，监听的地址就是host和port
int tcp_server(const char *host, unsigned short port)
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR_EXIT("socket error");        
    }

    struct sockaddr_in  servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    
    if (host == NULL) {
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        if (inet_aton(host, &servaddr.sin_addr) < 0) {
            ERR_EXIT("inet_aton error");
        }
    }

    int on = 1;
    if ((setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on))) < 0) {
        ERR_EXIT("setsockopt error");
    }

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind error");
    }

    if (listen(sockfd, SOMAXCONN) < 0) {
        ERR_EXIT("listen error");
    }
    
    return sockfd;
}

//ip指向一个已分配足够大小的空间
int getlocalip(char *ip)
{
    char name[100];
    if (gethostname(name, 100) < 0) {
        return -1;
    }

    struct hostent *addr;
    if ((addr = gethostbyname(name)) == NULL)
        return -1;

    strcpy(ip, inet_ntoa(*(struct in_addr*)(*(addr->h_addr_list))));
    return 0;
}

//设为非阻塞模式
void activate_nonblock(int fd)
{
    int ret;

    if ((ret = fcntl(fd, F_GETFL, 0)) < 0)
        ERR_EXIT("fcntl error");

    ret |= O_NONBLOCK;

    if (fcntl(fd, F_SETFL, ret) < 0)
        ERR_EXIT("fnctl error");
}

//设为阻塞模式
void deactivate_nonblock(int fd)
{
    int ret;

    if ((ret = fcntl(fd, F_GETFL, 0)) < 0)
        ERR_EXIT("fcntl error");

    ret &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, ret) < 0)
        ERR_EXIT("fnctl error");
}


//在waitseconds时间内fd是否可读
//可读返回0,否则返回-1
int read_timeout(int fd, unsigned int waitseconds) { 

    if (waitseconds <= 0)
        return 0;
     
    fd_set  readset;
    FD_ZERO(&readset);
    FD_SET(fd, &readset);

    struct timeval time;
    time.tv_sec = waitseconds;
    time.tv_usec = 0;

    int ret;
    do {
        ret = select(fd + 1, &readset, NULL, NULL, &time);
    } while (ret == -1 && errno == EINTR);
    
    if (ret == 0)   //超时
        return -1;
    
    if (ret == -1)           //发生其他错误
        ERR_EXIT("select error");
    return 0;
}

//在waitseconds时间内fd是否可写
//可写返回0,否则返回-1
int write_timeout(int fd, unsigned int waitseconds)
{
    if (waitseconds <= 0)
       return 0;
     
    fd_set  writeset;
    FD_ZERO(&writeset);
    FD_SET(fd, &writeset);

    struct timeval time;
    time.tv_sec = waitseconds;
    time.tv_usec = 0;

    int ret;

    do {
        ret = select(fd + 1, NULL, &writeset, NULL, &time);
    } while (ret == -1 && errno == EINTR);

    if (ret == 0)   //超时
        return -1;

    if (ret == -1 && errno != EINTR)                      //发生其他错误
        ERR_EXIT("select error");

    return 0;
}

//判断在waitseconds是否有连接到来，有返回已建立连接套接字描述符
//没有返回-1，fd必须是个监听套接字
int accept_time_out(int fd, struct sockaddr_in *addr, int waitseconds)
{
    int ret;
    ret = read_timeout(fd, waitseconds);
    if (ret == -1)
        return -1;
        
    socklen_t   len;
    
    if (addr == NULL) {
        ret = accept(fd, NULL, NULL);
    } else {
        len = sizeof(*addr);
        ret = accept(fd, (struct sockaddr*)addr, &len);    
    }


    if (ret == -1)
        ERR_EXIT("accept error");

    return ret;
}

//在waitseconds是否建立连接，是返回0,否则返回-1
//如果waitseconds==0,则一直等待connect超时或成功返回
int connect_time_out(int fd, struct sockaddr_in *addr, int waitseconds)
{
    if (waitseconds > 0)
       activate_nonblock(fd); 

    int ret;
    ret = connect(fd, (struct sockaddr*)addr, sizeof(*addr));

    if (ret == -1 && errno == EINPROGRESS) {
        fd_set flag;
        FD_ZERO(&flag);
        FD_SET(fd, &flag); 
        struct timeval time;
        time.tv_sec = waitseconds;
        time.tv_usec = 0;

        int res;
         
        do {
            res = select(fd + 1, NULL, &flag, NULL, &time);
        } while (res == -1 && errno == EINTR);

        if (res == 0)   //超时
            return -1;
        
        if (res == -1 && errno != EINTR)          //发生其他错误
            ERR_EXIT("select error");

        //res返回1,此时有两种可能，连接建立成功或连接建立失败
        //使用getsockopt获取错误码
        int err;
        socklen_t len = sizeof(err);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) == -1) //getsockopt发生错误
            ERR_EXIT("getsockopt error");

        if (err == 0)
            ret = 0;
        else
            ret = -1;

    } else if (ret == -1) {     //connect发生其他错误
        ERR_EXIT("connect error");
    }
     
    if (waitseconds > 0)
        deactivate_nonblock(fd); 
    return ret;
}

//读count个字节的数据
size_t readn(int fd, char *buf, int count)
{
    int nleft, nread;
    nleft = count;
    while (nleft > 0) {
        nread = read(fd, buf, nleft);
        if (nread == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("read error");
        }

        if (nread == 0)
            break;
        nleft -= nread;
        buf += nread;
    }
    return count - nleft;
}

//写count个字节的数据
size_t writen(int fd, const char *buf, int count)
{
    const char *pbuf = buf;
    int nleft, nwrite;
    nleft = count;
    while (nleft > 0) {
        nwrite = write(fd, pbuf, nleft);
        if (nwrite == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("write error");
        }

        if (nwrite == 0)
            break;
        nleft -= nwrite;
        pbuf += nwrite;
    }

    return count - nleft;
}

/**
 * recv_peek - 仅仅查看套接字缓冲区中的数据，但是不删除数据
 * sockfd:套接字描述符
 * buf:接受缓冲区
 * len:缓冲区的大小
 * 成功返回读取的字节数，失败返回-1
 */
ssize_t recv_peek(int sockfd, char *buf, size_t len)
{
    int ret;

    do {
        ret = recv(sockfd, buf, len, MSG_PEEK);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

/**
 * readline - 读取一行数据
 * fd:套接字描述符
 * buf:缓冲区
 * maxline:缓冲区的长度
 * 返回行缓冲区中存放的字节数
 */
size_t readline(int fd, char *buf, int maxline)
{
    int nleft, ret;
    char *pbuf;
    nleft = maxline;
    pbuf = buf;
    while (nleft > 0) {
        ret = recv_peek(fd, pbuf, nleft);
        if (ret == -1)
            ERR_EXIT("recv error");

        if (ret == 0)
            break;
        int i; 
        for (i = 0; i < ret; i++) {
            if (pbuf[i] == '\n') {
                readn(fd, pbuf, i + 1);
                nleft -= i+1;
                return maxline - nleft;
            }
        }

        //如果没找到'\n'，则读取全部数据
        readn(fd, pbuf, ret);
        nleft -= ret;
        pbuf += ret;
    }

    return maxline - nleft;
}

void send_fd(int sock_fd, int fd)
{
    int     ret;
    struct msghdr   msg;
    struct cmsghdr  *p_cmsg;
    struct iovec    vec;
    char cmsgbuf[CMSG_SPACE(sizeof(fd))];
    int *p_fds = 0;
    char sendchar = 0;
    
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    p_cmsg = CMSG_FIRSTHDR(&msg);

    p_cmsg->cmsg_level = SOL_SOCKET;
    p_cmsg->cmsg_type = SCM_RIGHTS;
    p_cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
    p_fds = (int*)CMSG_DATA(p_cmsg);
    *p_fds = send_fd;

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;

    vec.iov_base = &sendchar;
    vec.iov_len = sizeof(sendchar);
    ret = sendmsg(sock_fd, &msg, 0);

    if (ret != 1)
        ERR_EXIT("sendmsg");
}

int recv_fd(int sock_fd)
{
    int ret;
    struct msghdr msg;
    char recvchar;
    struct iovec vec;
    int recv_fd;
     
    char cmsgbuf[CMSG_SPACE(sizeof(recv_fd))];
    struct cmsghdr *p_cmsg;
    int *p_fd;
    vec.iov_base = &recvchar;
    vec.iov_len = sizeof(recvchar);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    msg.msg_flags = 0;

    p_fd = (int *)CMSG_DATA(CMSG_FIRSTHDR(&msg));
    *p_fd = -1;

    ret = recvmsg(sock_fd, &msg, 0);
    if (ret != 1)
        ERR_EXIT("recvmsg");

    p_cmsg = CMSG_FIRSTHDR(&msg);
    if (p_cmsg == NULL)
        ERR_EXIT("no passed fd");

    p_fd = (int *)CMSG_DATA(p_cmsg);
    recv_fd = *p_fd;
    if (recv_fd == -1)
        ERR_EXIT("no passed fd");

    return recv_fd;
}

int lock_file_read(int fd) //加读锁
{
    struct flock file_lock;
    file_lock.l_type = F_RDLCK;
    file_lock.l_start = 0;
    file_lock.l_whence = SEEK_SET;
    file_lock.l_len = 0;

    int ret;
    do {
        ret = fcntl(fd, F_SETLKW, &file_lock);  //以阻塞模式加锁
    } while (ret == -1 && errno == EINTR);

    return ret;     //如果返回-1,表示出错
}

int lock_file_write(int fd) //以阻塞方式加写锁
{
    struct flock file_lock;
    file_lock.l_type = F_WRLCK;
    file_lock.l_start = 0;
    file_lock.l_whence = SEEK_SET;
    file_lock.l_len = 0;

    int ret;
    do {
        ret = fcntl(fd, F_SETLKW, &file_lock);  //以阻塞模式加锁
    } while (ret == -1 && errno == EINTR);

    return ret;     //如果返回-1,表示出错
}

int unlock_file(int fd)
{
    struct flock file_lock;
    file_lock.l_type = F_UNLCK;
    file_lock.l_start = 0;
    file_lock.l_whence = SEEK_SET;
    file_lock.l_len = 0;

    int ret;
    ret = fcntl(fd, F_SETLK, &file_lock);

    return ret;
}

//定义一个全局变量
static struct timeval tv = {0, 0};

int get_cur_time_sec()     //秒
{
    if (gettimeofday(&tv, NULL) == -1)
        ERR_EXIT("gettimeofday");
    return tv.tv_sec;
}

int get_cur_time_usec()    //微秒
{
    return tv.tv_usec;
}

int nano_sleep(double t)   //睡眠t秒
{
    int sec = (time_t)t;
    int nsec = (t - sec) * 1000000000;

    struct timespec ts;
    ts.tv_sec = sec;
    ts.tv_nsec = nsec;
    int ret;

    do {
        ret = nanosleep(&ts, &ts);
    } while (ret == -1 && errno == EINTR);

    return ret;
}

void activate_oobinline(int sockfd)
{
    int oob_inline = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_OOBINLINE, &oob_inline, sizeof(oob_inline)) == -1)
        ERR_EXIT("setsockopt oob_inline");
}

void activate_signal_sigurg(int sockfd)
{
    if (fcntl(sockfd, F_SETOWN, getpid()) == -1)
        ERR_EXIT("fcntl sigurg");
}
