#ifndef COMMAND_MAP_H
#define COMMAND_MAP_H

#include "session.h"

void do_command_map(session_t *sess);

void ftp_reply(session_t *sess, int status, const char *context);


//命令处理函数
void do_user(session_t *sess);
void do_pass(session_t *sess);
#endif  //COMMAND_MAP_H
