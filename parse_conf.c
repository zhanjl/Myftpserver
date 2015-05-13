#include "parse_conf.h"
#include "configure.h"
#include "strutil.h"
#include "common.h"

static struct parseconf_bool_setting
{
    const char *p_setting_name;
    int *p_variable;
} parseconf_bool_array[] = 
{
    { "pasv_enable", &pasv_enable},
    { "port_enable", &port_enable},
    { 0, 0 }
};

static struct parseconf_uint_setting
{
    const char *p_setting_name;
    unsigned int *p_variable;
} parseconf_uint_array[] =
{
    { "listen_port", &listen_port},
    { "max_clients", &max_clients},
    { "max_per_ip", &max_per_ip},
    { "accept_timeout", &accept_timeout},
    { "connect_timeout", &connect_timeout},
    { "idle_session_timeout", &idle_session_timeout},
    { "local_umask", &local_umask},
    { "upload_max_rate", &upload_max_rate},
    { "download_max_rate", &download_max_rate},
    { 0, 0 }
};

static struct parseconf_str_setting
{
    const char *p_setting_name;
    char **p_variable;
} 
parse_str_array[] =
{
    { "listen_address", &listen_address },
    { 0, 0 }
};

void parse_load_file(const char* path)
{
    char buf[1024], name[1024], val[1024];
     
    FILE *fp;
    fp = fopen(path, "r");
    if (fp == NULL)
        ERR_EXIT("fopen");

    while (fgets(buf, sizeof(buf), fp)) {
        str_trim_crlf(buf);             //消除行尾的回车符
        if (str_all_space(buf)) {       //该行都是空格
            continue;
        }
        
        str_split(buf, name, val, '=');
        
        int flag = 0; 
        int i = 0;
        while (parseconf_bool_array[i].p_setting_name) {
            if (strcmp(parseconf_bool_array[i].p_setting_name, name) == 0) {
                str_upper(val);     //把yes或no变为大写                
                if (strcmp(val, "YES") == 0) {
                    flag = 1;
                    *(parseconf_bool_array[i].p_variable) = 1;
                    break;
                } else if (strcmp(val, "NO") == 0) {
                    flag = 1;
                    *(parseconf_bool_array[i].p_variable) = 0;
                    break;
                }

                fprintf(stderr, "config file format error\n");
                exit(EXIT_FAILURE);
            }
            i++;
        }
        
        if (flag)
            continue;

        i = 0;
        while (parseconf_uint_array[i].p_setting_name) {
            if (strcmp(parseconf_uint_array[i].p_setting_name, name) == 0) {
                flag = 1;
                if (strcmp(name, "local_umask") == 0)
                    *(parseconf_uint_array[i].p_variable) = str_octal_to_uint(val);
                else
                   *(parseconf_uint_array[i].p_variable) = str_dec_to_uint(val);
               break; 
            }
            i++;
        }

        if (flag)
            continue;

        i = 0;
        while (parse_str_array[i].p_setting_name) {
            if (strcmp(parse_str_array[i].p_setting_name, name) == 0) {
                char *ptr = (char*)malloc(strlen(val) + 1);
                strcpy(ptr, val);
                *(parse_str_array[i].p_variable) = ptr;
                flag = 1;
            }
            i++;
        }
        if (flag)
            continue;
        fprintf(stderr, "can't parse configure file\n");
        exit(EXIT_FAILURE);
    }

    return;
}
