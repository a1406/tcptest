#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include "ae.h"

#define ADDR "127.0.0.1"
#define PORT 2000

#define NODE_BLOCK 0x1
#define NODE_CONNECTED 0x2
#define NODE_DISCONNECTING 0x4
#define NODE_FREEING 0x8
#define NODE_IN_CALLBACK 0x10
typedef struct conn_node
{
	int fd;
	uint16_t flag;
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
        close(node->fd);

	aeDeleteFileEvent(el, node->fd, AE_READABLE);
	aeDeleteFileEvent(el, node->fd, AE_WRITABLE);
	node->fd = 0;
}

static void recv_func(aeEventLoop *el, int fd, void *privdata, int mask)
{
	printf("recv func\n");
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
		printf("connect success\n");		
	}
	char buf[128];
	int ret = recv(fd, buf, 128, 0);
	if (ret == 0)
	{
		disconnect(el, node);
	}
	else
	{
		printf("recv ret[%d] %s\n", ret, buf);
	}
		
}

static void write_func(aeEventLoop *el, int fd, void *privdata, int mask)
{
	printf("write func\n");
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
		printf("connect success\n");		
		aeDeleteFileEvent(el, fd, AE_WRITABLE);
	}

	char buf[128];
	int ret = write(fd, buf, 128);
}

static CONN_NODE node;
int main(int argc, char *argv[])
{
    aeEventLoop *el = aeCreateEventLoop(65536);

	node.fd = connect_server(ADDR, PORT);
	node.flag = 0;
	aeCreateFileEvent(el, node.fd, AE_READABLE, recv_func, &node);
	aeCreateFileEvent(el, node.fd, AE_WRITABLE, write_func, &node);	

	aeMain(el);
	aeDeleteEventLoop(el);
    return 0;
}


