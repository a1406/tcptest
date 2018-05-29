#ifndef AE_NET_H
#define AE_NET_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include "ae.h"

#define PORT 2000

#define NODE_BLOCK 0x1
#define NODE_CONNECTED 0x2
#define NODE_DISCONNECTING 0x4
#define NODE_FREEING 0x8
#define NODE_IN_CALLBACK 0x10

#define ANET_OK 0
#define ANET_ERR -1
#define ANET_ERR_LEN 256

extern aeEventLoop *global_el;

//typedef void (*on_disconnect_func)(aeEventLoop *el, CONN_NODE *node);
//void set_disconnect_callback(on_disconnect_func func);

void anetSetError(char *err, const char *fmt, ...);
int anetSetBlock(int fd, int block);
int connect_server(char *addr, int port);
int anetTcp6Server(char *err, int port, char *bindaddr, int backlog);
int anetTcpServer(char *err, int port, char *bindaddr, int backlog);
int anetTcpAccept(char *err, int s, char *ip, size_t ip_len, int *port);

#endif /* AE_NET_H */
