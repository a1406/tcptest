#ifndef CONN_NODE_BUF_H
#define CONN_NODE_BUF_H
#include "network.h"

int get_one_buf(CONN_NODE *node);
int remove_one_buf(CONN_NODE *node);

uint8_t * buf_head(CONN_NODE *node);

#endif /* CONN_NODE_BUF_H */
