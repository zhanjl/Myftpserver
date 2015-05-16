#include "sysutil.h"
#include "session.h"
#include "parse_conf.h"
#include "configure.h"
#include "ftp_codes.h"
#include "command_map.h"
#include "hash.h"
#define     LISTENPORT      9981

extern session_t *p_sess;
unsigned int num_of_clients = 0;    //连接数目
void handle_sigchld(int sig);
void limit_num_clients(session_t *sess);

hash_t  *ip_to_clients;
hash_t  *pid_to_ip;
unsigned int hash_func(unsigned int buckets, void *key, unsigned int keySize);
unsigned int add_ip_to_hash(hash_t *hash, uint32_t ip);
int main(int argc, char *argv[])
{

    if (getuid()) {
        fprintf(stderr, "the ftpserver must started by root\n");
        exit(EXIT_FAILURE); 
    }

    if (signal(SIGCHLD, handle_sigchld) == SIG_ERR)
        ERR_EXIT("signal");
    parse_load_file("ftpserver.conf");
    printf("parse_load_file success\n");
//    printconfig();
    
    ip_to_clients = hash_alloc(256, hash_func);
    pid_to_ip = hash_alloc(256, hash_func);
    int listenfd; 
    listenfd = tcp_server(listen_address, listen_port);
    
    printf("tcp_server success, listenfd = %d\n", listenfd);
    int connfd;
    pid_t   pid;
    session_t sess;
    while (1) {
        struct sockaddr_in addr;
        connfd = accept_time_out(listenfd, &addr, accept_timeout);
        if (connfd == -1) {
        //    printf("don't has connection in %d seconds\n", accept_timeout);
            continue;
        }
        
        uint32_t ip = addr.sin_addr.s_addr;
        printf("connect success\n");
        session_init(&sess);
        num_of_clients++;
        sess.curr_clients = num_of_clients;
        sess.curr_ip_clients = add_ip_to_hash(ip_to_clients, ip);
        sess.ip = ip;
        p_sess = &sess;
        pid = fork();
        if (pid == -1) {
            ERR_EXIT("fork error");
        } else if (pid == 0) {   //子进程
            close(listenfd); 
            sess.peerfd = connfd;
            session_begin(&sess);   //建立一个会话
            exit(EXIT_SUCCESS);
        } else {                //父进程
            close(connfd);
            hash_add_entry(pid_to_ip, &pid, sizeof(pid), &ip, sizeof(ip));
        }
    }
    return 0;
}

void handle_sigchld(int sig)
{
    pid_t   pid;
    unsigned int *value;
    uint32_t *ip; 
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        --num_of_clients;
        ip = hash_lookup_value_by_key(pid_to_ip, &pid, sizeof(pid)); 
        hash_free_entry(pid_to_ip, &pid, sizeof(pid));      

        value = hash_lookup_value_by_key(ip_to_clients, &ip, sizeof(ip));
        *value = *value - 1;
        if (*value == 0)
            hash_free_entry(ip_to_clients, &ip, sizeof(ip));
    }
}

void limit_num_clients(session_t *sess)
{
    if (max_clients > 0 && sess->curr_clients > max_clients) {
        ftp_reply(sess, FTP_TOO_MANY_USERS, "There are too many users, please try again later");
        exit(EXIT_FAILURE);        
    }
    if (max_per_ip > 0 && sess->curr_ip_clients > max_per_ip) {
        ftp_reply(sess, FTP_IP_LIMIT, "There are too many connections from your ip.");
        exit(EXIT_FAILURE);
    }
}

unsigned int hash_func(unsigned int buckets, void *key, unsigned int keySize)
{
    unsigned int *number = (unsigned int*)key;
    return (*number) % buckets;
}

unsigned int add_ip_to_hash(hash_t *hash, uint32_t ip)
{
    unsigned int *p_value = (unsigned int*)hash_lookup_value_by_key(hash,&ip,sizeof(ip));
    if (p_value == NULL) {
        unsigned int value = 1;
        hash_add_entry(hash, &ip, sizeof(ip), &value, sizeof(value));
        return 1;
    } else {
        unsigned int value = *p_value;
        value++;
        *p_value = value;
        return value;
    }
}
