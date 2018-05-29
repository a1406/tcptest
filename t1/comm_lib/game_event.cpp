#include "game_event.h"
#include "ae_net.h"
#include "comm.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <netinet/tcp.h>
#include <event2/event_struct.h>
#include "event2/event.h"
//#include "util-internal.h"
#include "listen_node.h"

extern "C" {
#include "log4c.h"
}
struct event_base *base = NULL;
log4c_category_t* trace_cat = NULL;
log4c_category_t* info_cat = NULL;
log4c_category_t* mycat = NULL;

std::map<int, get_conn_node> listen_get_conn_maps;
std::map<int, del_conn_node> listen_del_conn_maps;

int game_event_init()
{
	global_el = aeCreateEventLoop(65536);
	
	return (0);
}

//static void cb_signal(evutil_socket_t fd, short events, void *arg)
//{
//	LOG_DEBUG("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
//}

__attribute_used__ static void cb_timer(evutil_socket_t fd, short events, void *arg)
{
	LOG_DEBUG("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	if (arg)
		event_free((event *)arg);
}

void remove_listen_callback_event(conn_node_base *node)
{
	assert(listen_del_conn_maps.find(node->fd) != listen_del_conn_maps.end());
	del_conn_node callback = listen_del_conn_maps[node->fd];

	callback(node);
	
	node->flag |= NODE_DISCONNECTING;
		//ondisconnected();
    if (node->fd > 0)
	{
        close(node->fd);
	}

	aeDeleteFileEvent(global_el, node->fd, AE_READABLE);
	aeDeleteFileEvent(global_el, node->fd, AE_WRITABLE);
	node->fd = 0;
}

static int game_setnagleoff(int fd)
{
    /* Disable the Nagle (TCP No Delay) algorithm */ 
    int flag = 1; 

    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
    return 0;
}

static int game_cork_off(int fd)
{
    /* Disable the Nagle (TCP No Delay) algorithm */ 
    int flag = 0; 

    setsockopt(fd, IPPROTO_TCP, TCP_CORK, (char *)&flag, sizeof(flag));
    return 0;
}

static int game_cork_on(int fd)
{
    /* Disable the Nagle (TCP No Delay) algorithm */ 
    int flag = 1; 

    setsockopt(fd, IPPROTO_TCP, TCP_CORK, (char *)&flag, sizeof(flag));
    return 0;	
}

static int game_set_socket_opt(int fd)
{
	evutil_make_socket_nonblocking(fd);
	game_setnagleoff(fd);

//	int nRecvBuf=8192;
//	setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
//	setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(const char*)&nRecvBuf,sizeof(int));
	
	int on = 1;
	setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&on, sizeof(on));
	return (0);
}

int create_new_socket(int set_opt)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd <= 0) {
		LOG_ERR("%s %d: socket failed[%d]", __FUNCTION__, __LINE__, errno);		
		return fd;
	}

	if (likely(set_opt))
		game_set_socket_opt(fd);
	return fd;
}

int add_timer(struct timeval t, struct event *event_timer, void *arg)
{
	return (0);
	// if (!event_timer) {
	// 	event_timer = evtimer_new(base, cb_timer, arg);
	// 	if (!event_timer) {
	// 		LOG_ERR("%s %d: evtimer_new failed[%d]", __FUNCTION__, __LINE__, errno);					
	// 		return (-1);
	// 	}
	// 	event_timer->ev_arg = event_timer;
	// } else if (!(event_timer->ev_flags & EVLIST_TIMEOUT)
	// 	&& !(event_timer->ev_flags & EVLIST_ACTIVE)) {
	// 	evtimer_assign(event_timer, base, event_timer->ev_callback, arg);
	// }

	// return evtimer_add(event_timer, &t);
}
int add_signal(int signum, struct event *event, event_callback_fn callback)
{
	return (0);
	// if (!event) {
	// 	event = evsignal_new(base, signum, callback, NULL);
	// 	if (!event) {
	// 		LOG_ERR("%s %d: evsignal_new failed[%d]", __FUNCTION__, __LINE__, errno);					
	// 		return (-1);
	// 	}
	// 	event->ev_arg = event;
	// } else {
	// 	evsignal_assign(event, base, signum, event->ev_callback, NULL);
	// }

	// return evsignal_add(event, NULL);
}

static void cb_send_func(aeEventLoop *el, int fd, void *privdata, int mask)
{
}

static void cb_recv_func(aeEventLoop *el, int fd, void *privdata, int mask)
{
	assert(privdata);
	conn_node_base *client = (conn_node_base *)privdata;

	game_cork_on(fd);
	int ret = client->recv_func(fd);
	game_cork_off(fd);
	
	if (ret >= 0)
		return;

	remove_listen_callback_event(client);	
}

#define MAX_ACCEPTS_PER_CALL 100
#define NET_IP_STR_LEN 46 /* INET6_ADDRSTRLEN is 46, but we need to be sure */
static void acceptTcpHandler(aeEventLoop *el, int fd, void *privdata, int mask)
{
    int cport, cfd;
	int max = MAX_ACCEPTS_PER_CALL;
    char cip[NET_IP_STR_LEN];
    UNUSED(el);
    UNUSED(mask);
    UNUSED(privdata);

	assert(listen_get_conn_maps.find(fd) != listen_get_conn_maps.end());
	get_conn_node callback = listen_get_conn_maps[fd];

    while(max--) {
        cfd = anetTcpAccept(NULL, fd, cip, sizeof(cip), &cport);
        if (cfd == ANET_ERR) {
//            if (errno != EWOULDBLOCK)
//                serverLog(LL_WARNING,
//                    "Accepting client connection: %s", server.neterr);
            return;
        }

		anetSetBlock(cfd, 0);
		conn_node_base *node = callback(cfd);
		if (!node)
		{
			close(cfd);
			continue;
		}
		node->fd = cfd;
		node->port = cport;
		node->flag = NODE_CONNECTED;
		node->send_buffer_begin_pos = 0;
		node->send_buffer_end_pos = 0;
		node->on_write = cb_send_func;
		node->pos_begin = node->pos_end = 0;

		aeCreateFileEvent(el, node->fd, AE_READABLE, cb_recv_func, node);

		LOG_DEBUG("fd %d accept from %s\n", node->fd, cip);
    }
}

int game_add_listen_event(int port, get_conn_node cb1, del_conn_node cb2, const char *name)
{
	assert(cb1);
	assert(cb2);	
	int fd = anetTcpServer(NULL, port, NULL, 511);
	if (aeCreateFileEvent(global_el, fd, AE_READABLE, acceptTcpHandler, NULL) == AE_ERR)
	{
		LOG_ERR("Unrecoverable error creating server.ipfd file event.\n");
		close(fd);
		return -1;
	}
	listen_get_conn_maps[fd] = cb1;
	listen_del_conn_maps[fd] = cb2;	

	LOG_INFO("%s: %s fd = %d, port = %d", __FUNCTION__, name, fd, port);
	return (fd);
}

int game_add_connect_event(struct sockaddr *sa, int socklen, conn_node_base *client)
{
	// TODO: 
	return (0);
}

static const char* dated_r_format(
    const log4c_layout_t*  	a_layout,
    const log4c_logging_event_t*a_event)
{
    int n, i;
    struct tm	tm;

//    gmtime_r(&a_event->evt_timestamp.tv_sec, &tm);
    localtime_r(&a_event->evt_timestamp.tv_sec, &tm);
    n = snprintf(a_event->evt_buffer.buf_data, a_event->evt_buffer.buf_size,
		 "%04d%02d%02d %02d:%02d:%02d.%03ld %-8s %s\n",
		 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_hour, tm.tm_min, tm.tm_sec,
		 a_event->evt_timestamp.tv_usec / 1000,
		 log4c_priority_to_string(a_event->evt_priority),
		 a_event->evt_msg);

    if (n >= (int)a_event->evt_buffer.buf_size) {
	/*
	 * append '...' at the end of the message to show it was
	 * trimmed
	 */
	for (i = 0; i < 3; i++)
	    a_event->evt_buffer.buf_data[a_event->evt_buffer.buf_size - 4 + i] = '.';
    }

    return a_event->evt_buffer.buf_data;
}

/*******************************************************************************/
static const log4c_layout_type_t log4c_layout_type_test = {
  "mydated_r",
  dated_r_format,
};


void init_mycat()
{
    log4c_layout_t*   layout1   = log4c_layout_get("dated");
    log4c_layout_set_type(layout1, &log4c_layout_type_test);	
	
	trace_cat = log4c_category_get("six13log.log.trace");
	info_cat = log4c_category_get("six13log.log.info");
	mycat = trace_cat;
}

void change_mycat()
{
	if (mycat == trace_cat)
		mycat = info_cat;		
	else
		mycat = trace_cat;
}
