#ifndef MY_NETWORK_H
#define MY_NETWORK_H

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

typedef struct st_proto_head
{
    uint32_t len;     //长度
    uint16_t msg_id;  //消息ID
    uint16_t seq;     //序号
                      //	uint32_t crc;		//crc校验
    char data[0];     // PROTO 内容
} __attribute__ ((packed)) PROTO_HEAD;

#define MAX_CLIENT_SEND_BUFFER_SIZE (1024*256)
typedef struct conn_node
{
	int fd;
	uint16_t flag;
	int send_num;
	char send_buffer[MAX_CLIENT_SEND_BUFFER_SIZE];
	int32_t send_buffer_begin_pos;
	int32_t send_buffer_end_pos;
//	aeFileProc *on_read;
	aeFileProc *on_write;

	uint32_t pos_begin;	
	uint32_t pos_end;	
	uint8_t *buf;//[MAX_BUF_PER_CLIENT + sizeof(EXTERN_DATA)];
	uint32_t max_buf_len;
} CONN_NODE;

extern aeEventLoop *el;
typedef void (*on_disconnect_func)(aeEventLoop *el, CONN_NODE *node);
void set_disconnect_callback(on_disconnect_func func);

void anetSetError(char *err, const char *fmt, ...);
int connect_server(char *addr, int port);
void disconnect(aeEventLoop *el, CONN_NODE *node);
int anetTcp6Server(char *err, int port, char *bindaddr, int backlog);
int anetTcpServer(char *err, int port, char *bindaddr, int backlog);
int anetTcpAccept(char *err, int s, char *ip, size_t ip_len, int *port);

int send_data(CONN_NODE *node, char *buf, int len);
int send_data_real(CONN_NODE *node);
#endif /* MY_NETWORK_H */
