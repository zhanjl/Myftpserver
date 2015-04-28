#include "strutil.h"
#include <string.h>
#include <ctype.h>
void str_trim_crlf(char *str)
{
    char *p;
    p = str + strlen(str) - 1; 

    while ((*p == '\r') || (*p == '\n')) {
        *p = 0;
        p--;
    }
}

void str_split(const char *str, char *left, char *right, char c)
{
    const char *ptr;
    ptr = str;
    
    while ((*ptr != c) && (*ptr != '\0')) {
        *left = *ptr;
        left++;
        ptr++;
     } 
    *left = '\0';

    strcpy(right, ptr);
} 

int str_all_space(const char *str)
{
    while (*str != '\0') {
        if (!isspace(*str))
            return 0;
        str++;
    }

    return 1;
}

void str_upper(char *str)
{
    while (*str) {
        if (islower(*str)) {
            *str = toupper(*str);
        }
        str++;
    }
}

unsigned int str_octal_to_uint(const char *str)
{
    int base;
    unsigned int res;
    const char *ptr;

    ptr = str + strlen(str) - 1;
    base = 1, res = 0;

    while (ptr != str) {
        res += (*ptr - '0') * base;
        base *= 8;
    }

    res += (*ptr - '0') * base;

    return res;
}
