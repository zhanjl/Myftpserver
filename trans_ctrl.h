#ifndef TRANS_CTRL_H
#define TRANS_CTRL_H

#include "session.h"

void limit_curr_rate(session_t *sess, int nbytes, int is_upload);

//实现空闲断开功能
void setup_signal_alarm();
void start_signal_alarm();

//设置SIGURG信号的处理函数
void setup_signal_sigurg();
#endif  //TRANS_CTRL_H
