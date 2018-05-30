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

extern "C" {
#include "log4c.h"
}

log4c_category_t* trace_cat = NULL;
log4c_category_t* info_cat = NULL;
log4c_category_t* mycat = NULL;

int game_event_init()
{
	global_el = aeCreateEventLoop(65536);
	
	return (0);
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

int add_timer(struct timeval t, void *arg)
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

static void cb_send_func(aeEventLoop *el, int fd, void *arg, int mask)
{
	assert(arg);
	conn_node_base *client = (conn_node_base *)arg;
	client->send_data_to_client();
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

	client->disconnect();
}

int send_one_buffer(conn_node_base *node, char *buffer, uint32_t len)
{
#if 1
#ifdef FLOW_MONITOR
	add_one_client_answer(head);
#endif
	if (node->send_buffer_end_pos+len >= node->send_buffer_size) {  ///缓冲区溢出, 关闭连接
		LOG_DEBUG("[%s: %d]: fd: %d: send msg len[%d], begin[%d]end[%d] buffer full", __PRETTY_FUNCTION__, __LINE__, node->fd, len, node->send_buffer_begin_pos, node->send_buffer_end_pos);
		return -1;
	}

	if (node->send_buffer_begin_pos == node->send_buffer_end_pos)
	{
		if (aeCreateFileEvent(global_el, node->fd, AE_WRITABLE, cb_send_func, node) != AE_OK)
		{
			LOG_DEBUG("[%s: %d]: fd: %d: createfileevent failed", __PRETTY_FUNCTION__, __LINE__, node->fd);
			return -10;			
		}
	}

	memcpy(node->send_buffer+node->send_buffer_end_pos, buffer, len);
//	encoder_data((PROTO_HEAD*)(node->send_buffer+send_buffer_end_pos));

	node->send_buffer_end_pos += len;
	return 0;
#else
	return conn_node_base::send_one_msg(head, force);
#endif
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
    get_conn_node callback = (get_conn_node)(privdata);

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

		node->on_connected();
		LOG_DEBUG("fd %d accept from %s %d", node->fd, cip, cport);
    }
}

int game_add_listen_event(int port, get_conn_node get_node_func, const char *name)
{
	int fd = anetTcpServer(NULL, port, NULL, 511);
	anetSetBlock(fd, 0);
	if (aeCreateFileEvent(global_el, fd, AE_READABLE, acceptTcpHandler, (void *)get_node_func) == AE_ERR)
	{
		LOG_ERR("Unrecoverable error creating server.ipfd file event.\n");
		close(fd);
		return -1;
	}

	LOG_INFO("%s: %s fd = %d, port = %d", __FUNCTION__, name, fd, port);
	return (fd);
}

static void connect_func(aeEventLoop *el, int fd, void *privdata, int mask)
{
	conn_node_base *node = (conn_node_base *)privdata;
	if (!(node->flag & NODE_CONNECTED))
	{
		int err = 0;
		socklen_t errlen = sizeof(err);
		if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) == -1) {
			LOG_ERR("getsocktopt failed connect failed");
			return;
		}
		if (err) {
			node->disconnect();
			LOG_ERR("connect failed, err = %d", err);
			return;
		}
		node->flag |= NODE_CONNECTED;
		node->on_connected();

	}

    aeFileEvent *fe = &global_el->events[fd];	
	fe->rfileProc = cb_recv_func;
	aeDeleteFileEvent(global_el, fd, AE_WRITABLE);
	
	node->recv_func(fd);
}

int game_add_connect_event(conn_node_base *node, char *addr, int port)
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

	node->flag = 0;
	node->fd = s;
	aeCreateFileEvent(global_el, s, AE_READABLE, connect_func, node);
	aeCreateFileEvent(global_el, s, AE_WRITABLE, connect_func, node);
	
	return s;
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
