#include "ftp_nobody.h"
#include "common.h"
#include "sysutil.h"
void set_nobody();
void handle_nobody(session_t *sess)
{
    set_nobody();
    
    char cmd;
    
    while (1) {
        int ret = readn(sess->nobody_fd, &cmd, sizeof (cmd));
        if (ret == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("readn");
        }

        for (;;)
            pause();
    } 
}

void set_nobody()
{
    struct passwd *pw;
    if ((pw = getpwnam("zhan")) == NULL)
        ERR_EXIT("getpwnam");

    if (setegid(pw->pw_gid) == -1)
        ERR_EXIT("setegid");

    if (seteuid(pw->pw_uid) == -1)
        ERR_EXIT("seteuid");

}
