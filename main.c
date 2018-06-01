#include "network.h"
#include <assert.h>
#include <signal.h>

#define ADDR "127.0.0.1"
#define CLIENT_NUM 200
#define SEND_NUM  200000

static char global_send_buf[256];
static int global_send_len;
static int global_reconnect_count;
static CONN_NODE global_node[CLIENT_NUM];

static char buf[1024][256];
static int last_len[1024];
static int _t_index[1024];

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
		last_len[fd] = 0;
//		printf("connect success\n");		
	}
	int ret = recv(fd, &(buf[fd][last_len[fd]]), 256 - last_len[fd], 0);
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
		PROTO_HEAD *head = (PROTO_HEAD *)(buf[fd]);
//		printf("recv ret = %d, len = %d, last_len = %d\n", ret, head->len, last_len[fd]);
		
		ret += last_len[fd];
		if (ret < sizeof(PROTO_HEAD))
			return;
		
		for (; ret >= sizeof(PROTO_HEAD) && head->len <= ret;)
		{
//			printf("recv len = %d, head len = %d, head buf = %s, head = %p, _t_index = %d\n", ret, head->len, head->data, head, _t_index[fd]);
			assert(head->len == 19);
			assert(memcmp(head->data, "tangpeilei", 10) == 0);			
			ret -= head->len;
			head = (PROTO_HEAD *)((char *)head + head->len);
			++_t_index[fd];
		}
		memmove(buf[fd], head, ret);
		last_len[fd] = ret;
//		printf("memmove = %d\n", ret);
	}
		
}

static void	check_finished()
{
	int i;
	for (i = 0; i < CLIENT_NUM; ++i)
	{
		if (global_node[i].send_num < SEND_NUM)
			return;
	}
	printf("all finished\n");
	sleep(3);
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
		last_len[fd] = 0;		
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
void on_disconnect(aeEventLoop *el, CONN_NODE *node)
{
	++global_reconnect_count;
	printf("reconnect count = %d\n", global_reconnect_count);
	
	node->fd = connect_server(ADDR, PORT);
	node->flag = 0;

	aeCreateFileEvent(el, node->fd, AE_READABLE, recv_func, node);
	aeCreateFileEvent(el, node->fd, AE_WRITABLE, write_func, node);
}

int main(int argc, char *argv[])
{
	PROTO_HEAD *head = (PROTO_HEAD *)(&global_send_buf[0]);
	strcpy(head->data, "tangpeilei");
	head->len = sizeof(PROTO_HEAD) + 11;
	global_send_len = head->len;

	if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
		printf("set sigpipe ign failed\n");		
		return (0);
	}	

	set_disconnect_callback(on_disconnect);
	
    el = aeCreateEventLoop(65536);

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
	printf("main exit, stop = %d\n", el->stop);
	
	aeDeleteEventLoop(el);
    return 0;
}


