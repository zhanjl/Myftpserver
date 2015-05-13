#ifndef TRANS_DATA_H
#define TRANS_DATA_H

#include "session.h"

//int get_trans_data_fd(session_t *sess); //建立连接
void trans_lists(session_t *sess, int level);  //发送目录数据
void upload_file(session_t *sess, int is_appe); //上传文件
void download_file(session_t *sess); //下载文件

int get_trans_data_fd(session_t *sess);
#endif  //TRANS_DATA_H
