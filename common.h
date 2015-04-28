#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <pwd.h>
#include <ctype.h>

#define ERR_EXIT(m) \
        do { \
        perror(m);\
        exit(EXIT_FAILURE); \
        } while (0)

#endif  /*COMMON_H*/
