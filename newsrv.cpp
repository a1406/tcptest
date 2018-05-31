#include <map>
#include "network.h"
#include "conn_node_buf.h"

std::map<int, CONN_NODE *> all_clients;

static void recv_func(aeEventLoop *el, int fd, void *privdata, int mask)
{
}

static void send_func(aeEventLoop *el, int fd, void *privdata, int mask)
{
}

#define MAX_ACCEPTS_PER_CALL 1000
#define NET_IP_STR_LEN 46 /* INET6_ADDRSTRLEN is 46, but we need to be sure */
#define UNUSED(V) ((void) V)
static void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask)
{
    int cport, cfd, max = MAX_ACCEPTS_PER_CALL;
    char cip[NET_IP_STR_LEN];
    UNUSED(el);
    UNUSED(mask);
    UNUSED(privdata);

    while(max--) {
        cfd = anetTcpAccept(NULL, fd, cip, sizeof(cip), &cport);
        if (cfd == ANET_ERR) {
//            if (errno != EWOULDBLOCK)
//                serverLog(LL_WARNING,
//                    "Accepting client connection: %s", server.neterr);
            return;
        }

		CONN_NODE *node = (CONN_NODE *)malloc(sizeof(CONN_NODE));
		node->fd = cfd;
		node->flag = NODE_CONNECTED;
		node->send_buffer_begin_pos = 0;
		node->send_buffer_end_pos = 0;
		node->on_write = send_func;
		node->pos_begin = node->pos_end = 0;
		node->max_buf_len = 10 * 1024;
		node->buf = (uint8_t *)malloc(node->max_buf_len);
		all_clients[cfd] = node;
		aeCreateFileEvent(el, node->fd, AE_READABLE, recv_func, node);		
//        serverLog(LL_VERBOSE,"Accepted %s:%d", cip, cport);
//        acceptCommonHandler(cfd,0,cip);
		
    }
}

int main(int argc, char *argv[])
{
	el = aeCreateEventLoop(65536);
	int fd = anetTcpServer(NULL, PORT, NULL, 511);
	if (aeCreateFileEvent(el, fd, AE_READABLE, acceptTcpHandler, NULL) == AE_ERR)
	{
		printf("Unrecoverable error creating server.ipfd file event.");
		return 0;
	}	
    return 0;
}
