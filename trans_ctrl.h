#ifndef TRANS_CTRL_H
#define TRANS_CTRL_H

#include "session.h"

void limit_curr_rate(session_t *sess, int nbytes, int is_upload);

#endif  //TRANS_CTRL_H
