#include <stdio.h>
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

#define ADDR "192.168.1.201"
#define PORT 2000
#define CLIENT_NUM 500
#define SEND_NUM  100000

#define NODE_BLOCK 0x1
#define NODE_CONNECTED 0x2
#define NODE_DISCONNECTING 0x4
#define NODE_FREEING 0x8
#define NODE_IN_CALLBACK 0x10

typedef struct st_proto_head
{
    uint32_t len;     //长度
    uint16_t msg_id;  //消息ID
    uint16_t seq;     //序号
                      //	uint32_t crc;		//crc校验
    char data[0];     // PROTO 内容
} __attribute__ ((packed)) PROTO_HEAD;

typedef struct conn_node
{
	int fd;
	uint16_t flag;
	int send_num;
} CONN_NODE;

int anetSetBlock(int fd, int block)
{
    int flags;

    /* Set the socket blocking (if non_block is zero) or non-blocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        return -1;
    }

    if (!block)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1) {
        return -1;
    }
    return 0;
}

int connect_server(char *addr, int port)
{
    int s = -1, rv;
    char portstr[6];  /* strlen("65535") + 1; */
    struct addrinfo hints, *servinfo, *p;

    snprintf(portstr,sizeof(portstr),"%d",port);
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr,portstr,&hints,&servinfo)) != 0) {
        return -1;
    }
    for (p = servinfo; p != NULL; p = p->ai_next) {
        /* Try to create the socket and to connect it.
         * If we fail in the socket() call, or on connect(), we retry with
         * the next entry in servinfo. */
        if ((s = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
            continue;
		int yes = 1;
		if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
			goto error;
        if (anetSetBlock(s, 0) != 0)
            goto error;
        if (connect(s,p->ai_addr,p->ai_addrlen) == -1) {
            /* If the socket is non-blocking, it is ok for connect() to
             * return an EINPROGRESS error here. */
            if (errno == EINPROGRESS)
                goto end;
            close(s);
            s = -1;
            continue;
        }

        /* If we ended an iteration of the for loop without errors, we
         * have a connected socket. Let's return to the caller. */
        goto end;
    }

error:
    if (s != -1) {
        close(s);
        s = -1;
    }

end:
    freeaddrinfo(servinfo);
	return s;
}

static void disconnect(aeEventLoop *el, CONN_NODE *node)
{
	printf("disconnect\n");
	node->flag |= NODE_DISCONNECTING;
		//ondisconnected();
    if (node->fd > 0)
	{
        close(node->fd);
	}

	aeDeleteFileEvent(el, node->fd, AE_READABLE);
	aeDeleteFileEvent(el, node->fd, AE_WRITABLE);
	node->fd = 0;
}

static void recv_func(aeEventLoop *el, int fd, void *privdata, int mask)
{
	CONN_NODE *node = (CONN_NODE *)privdata;
	if (!(node->flag & NODE_CONNECTED))
	{
		int err = 0;
		socklen_t errlen = sizeof(err);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
			printf("getsocktopt failed connect failed\n");
			return;
		}
		if (err) {
			disconnect(el, node);
			printf("connect failed, err = %d\n", err);
			return;
		}
		node->flag |= NODE_CONNECTED;
//		printf("connect success\n");		
	}
	char buf[256];
	int ret = recv(fd, buf, 128, 0);
	if (ret == 0)
	{
		disconnect(el, node);
	}
	else if (ret < 0 && errno != EAGAIN)
	{
		printf("recv ret[%d] errno = %d\n", ret, errno);
		disconnect(el, node);		
	}
	else
	{
//		PROTO_HEAD *head = (PROTO_HEAD *)buf;
//		printf("recv len = %d, head len = %d, head buf = %s\n", ret, head->len, head->data);
	}
		
}
char global_send_buf[256];
int global_send_len;
static CONN_NODE global_node[CLIENT_NUM];

static void	check_finished()
{
	int i;
	for (i = 0; i < CLIENT_NUM; ++i)
	{
		if (global_node[i].send_num < SEND_NUM)
			return;
	}
	printf("all finished\n");
	exit(0);
}

static void write_func(aeEventLoop *el, int fd, void *privdata, int mask)
{
//	printf("write func\n");
	CONN_NODE *node = (CONN_NODE *)privdata;
	if (!(node->flag & NODE_CONNECTED))
	{
		int err = 0;
		socklen_t errlen = sizeof(err);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
			printf("getsocktopt failed connect failed\n");
			return;
		}
		if (err) {
			disconnect(el, node);
			printf("connect failed, err = %d\n", err);
			return;
		}
		node->flag |= NODE_CONNECTED;
//		printf("connect success\n");		
//		aeDeleteFileEvent(el, fd, AE_WRITABLE);
	}

	int len = 0;
	while (len < global_send_len)
	{
		int ret = write(fd, global_send_buf, global_send_len);
		if (ret <= 0)
		{
			printf("write err, ret = %d, err = %d\n", ret, errno);
			return;
		}
		len += ret;
	}
	++node->send_num;
	if (node->send_num >= SEND_NUM)
	{
		aeDeleteFileEvent(el, fd, AE_WRITABLE);
		check_finished();
	}
}

void send_to_server(aeEventLoop *el, CONN_NODE *node)
{
	aeCreateFileEvent(el, node->fd, AE_WRITABLE, write_func, node);
}

int main(int argc, char *argv[])
{
	PROTO_HEAD *head = (PROTO_HEAD *)(&global_send_buf[0]);
	strcpy(head->data, "tangpeilei");
	head->len = sizeof(PROTO_HEAD) + 11;
	global_send_len = head->len;
	
    aeEventLoop *el = aeCreateEventLoop(65536);

	int i;
	for (i = 0; i < CLIENT_NUM; ++i)
	{
		global_node[i].fd = connect_server(ADDR, PORT);
		global_node[i].flag = 0;
		global_node[i].send_num = 0;
		aeCreateFileEvent(el, global_node[i].fd, AE_READABLE, recv_func, &global_node[i]);
		aeCreateFileEvent(el, global_node[i].fd, AE_WRITABLE, write_func, &global_node[i]);
	}

	aeMain(el);
	aeDeleteEventLoop(el);
    return 0;
}


