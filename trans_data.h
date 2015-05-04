#ifndef TRANS_DATA_H
#define TRANS_DATA_H

#include "session.h"

int get_trans_data_fd(session_t *sess); //建立连接
void trans_lists(session_t *sess);  //发送目录数据

#endif  //TRANS_DATA_H
