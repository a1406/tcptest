#include "game_event.h"
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

static void libevent_log(int severity, const char *msg)
{
	printf("%s %d %d: %s", __FUNCTION__, __LINE__, severity, msg);
	log4c_category_log(mycat, severity, msg);
}

#define EVUTIL_ERR_CONNECT_RETRIABLE(e)			\
	((e) == EINTR || (e) == EINPROGRESS)
#define EVUTIL_ERR_CONNECT_REFUSED(e)					\
	((e) == ECONNREFUSED)


static int
evutil_socket_connect_(evutil_socket_t *fd_ptr, const struct sockaddr *sa, int socklen)
{
	int made_fd = 0;

	if (*fd_ptr < 0) {
		if ((*fd_ptr = socket(sa->sa_family, SOCK_STREAM, 0)) < 0)
			goto err;
		made_fd = 1;
		if (evutil_make_socket_nonblocking(*fd_ptr) < 0) {
			goto err;
		}
	}

	if (connect(*fd_ptr, sa, socklen) < 0) {
		int e = evutil_socket_geterror(*fd_ptr);
		if (EVUTIL_ERR_CONNECT_RETRIABLE(e))
			return 0;
		if (EVUTIL_ERR_CONNECT_REFUSED(e))
			return 2;
		goto err;
	} else {
		return 1;
	}

err:
	if (made_fd) {
		evutil_closesocket(*fd_ptr);
		*fd_ptr = -1;
	}
	return -1;
}

int game_event_init()
{
	int ret = 0;
	struct event_config *config = NULL;
//	struct event *event_signal1 = NULL;
//	struct event *event_signal2 = NULL;	
	
	event_set_log_callback(libevent_log);

	config = event_config_new();
	if (!config)
	{
		LOG_ERR("%s %d: event_config_new failed[%d]", __FUNCTION__, __LINE__, errno);
		ret = -10;
		goto fail;
	}
	event_config_require_features(config, /*EV_FEATURE_ET |*/ EV_FEATURE_O1 /*| EV_FEATURE_FDS*/);
	event_config_set_flag(config, EVENT_BASE_FLAG_NOLOCK | EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);
	base = event_base_new_with_config(config);
	if (!base)
	{
		LOG_ERR("%s %d: event_base_new_with_config failed[%d]", __FUNCTION__, __LINE__, errno);
		ret = -20;
		goto fail;
	}
/*
	event_signal1 = evsignal_new(base, SIGUSR1, cb_signal, NULL);
	if (!event_signal1) {
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "%s %d: evsignal_new failed[%d]", __FUNCTION__, __LINE__, errno);
		ret = -30;
		goto fail;
	}
	event_signal2 = evsignal_new(base, SIGUSR2, cb_signal, NULL);
	if (!event_signal2) {
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, "%s %d: evsignal_new failed[%d]", __FUNCTION__, __LINE__, errno);
		ret = -30;
		goto fail;
	}
	
//	struct event event_signal, event_timer;
//	evsignal_assign(&event_signal, base, SIGUSR1, cb_signal, NULL);
//	evtimer_assign(&event_timer, base, cb_timer, NULL);
	evsignal_add(event_signal1, NULL);
	evsignal_add(event_signal2, NULL);	
*/	
	return (0);
	
fail:
/*	
	if (event_signal1) {
		event_free(event_signal1);
		event_signal1 = NULL;
	}
	if (event_signal2) {
		event_free(event_signal2);
		event_signal2 = NULL;
	}
*/	
	if (config) {
		event_config_free(config);
		config = NULL;
	}
	if (base) {
		event_base_free(base);
		base = NULL;
	}
	return (ret);
}

//static void cb_signal(evutil_socket_t fd, short events, void *arg)
//{
//	LOG_DEBUG("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
//}

static void cb_timer(evutil_socket_t fd, short events, void *arg)
{
	LOG_DEBUG("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	if (arg)
		event_free((event *)arg);
}

void remove_listen_callback_event(conn_node_base *client)
{
	if (!client)
		return;
	if (client->fd == 0) {
		LOG_ERR("%s: fd is already closed", __FUNCTION__);
		return;
	}
	LOG_DEBUG("%s: close connect from fd [%p]%u", __FUNCTION__, client, client->fd);
	
	event_del(&client->event_recv);
	if (client->get_write_event()) {
		event_del(client->get_write_event());
	}
	

	evutil_closesocket(client->fd);
	delete(client);
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
	if (!event_timer) {
		event_timer = evtimer_new(base, cb_timer, arg);
		if (!event_timer) {
			LOG_ERR("%s %d: evtimer_new failed[%d]", __FUNCTION__, __LINE__, errno);					
			return (-1);
		}
		event_timer->ev_arg = event_timer;
	} else if (!(event_timer->ev_flags & EVLIST_TIMEOUT)
		&& !(event_timer->ev_flags & EVLIST_ACTIVE)) {
		evtimer_assign(event_timer, base, event_timer->ev_callback, arg);
	}

	return evtimer_add(event_timer, &t);
}
int add_signal(int signum, struct event *event, event_callback_fn callback)
{
	if (!event) {
		event = evsignal_new(base, signum, callback, NULL);
		if (!event) {
			LOG_ERR("%s %d: evsignal_new failed[%d]", __FUNCTION__, __LINE__, errno);					
			return (-1);
		}
		event->ev_arg = event;
	} else {
		evsignal_assign(event, base, signum, event->ev_callback, NULL);
	}

	return evsignal_add(event, NULL);
}

static void cb_recv(evutil_socket_t fd, short events, void *arg)
{
	assert(arg);
	conn_node_base *client = (conn_node_base *)arg;

	game_cork_on(fd);
	int ret = client->recv_func(fd);
	game_cork_off(fd);
	
	if (ret >= 0)
		return;

//	if (unlikely(events == EV_WRITE))
//		return;
	
	remove_listen_callback_event(client);	
}

static void cb_listen(evutil_socket_t fd, short events, void *arg)
{
	assert(arg);
	conn_node_base *client = NULL;
	int new_fd = 0;
	struct sockaddr_in ss;
	size_t socklen = sizeof(ss);
	listen_node_base *callback = (listen_node_base *)arg;
	if (callback->listen_pre_func() < 0) {
		LOG_INFO("%s: connect pre refused", __FUNCTION__);
		new_fd = accept(fd, (struct sockaddr*)&ss, (socklen_t *)&socklen);		
		goto fail;
	}
	
	new_fd = accept(fd, (struct sockaddr*)&ss, (socklen_t *)&socklen);
	if (new_fd < 0) {
		LOG_ERR("%s %d: accept failed[%d]", __FUNCTION__, __LINE__, errno);
		goto fail;
	}
	LOG_INFO("%s %d: fd = %d, new_fd = %d, ip = %x, port = %u, events = %d, arg = %p",
		__FUNCTION__, __LINE__, fd, new_fd, ss.sin_addr.s_addr, htons(ss.sin_port), events, arg);
	
	game_set_socket_opt(new_fd);	

	client = callback->get_conn_node(new_fd);
	if (!client) {
		LOG_ERR("%s %d: cb_get_client_map with fd[%u] new_fd[%u] failed", __FUNCTION__, __LINE__, fd, new_fd);		
		goto fail;
	}
	client->sock = ss;
	
	if (0 != event_assign(&client->event_recv, base, new_fd, EV_READ | EV_PERSIST, cb_recv, client)) {
		LOG_ERR("%s %d: event_assign failed[%d]", __FUNCTION__, __LINE__, errno);
		goto fail;
	}

	if (event_add(&client->event_recv, NULL) != 0) {
		LOG_ERR("%s %d: event_assign failed[%d]", __FUNCTION__, __LINE__, errno);
		goto fail;			
	}

	if (callback->listen_after_func(new_fd) < 0) {
		LOG_INFO("%s: connect after refused", __FUNCTION__);
		goto fail;
	}
	return;
fail:
	if (client) {
		remove_listen_callback_event(client);
	}
	if (new_fd > 0)
		evutil_closesocket(new_fd);			
}


int game_add_listen_event(uint16_t port, listen_node_base *callback, const char *name)
{
	struct event *event_accept = &callback->event_accept;
	int fd = 0;
	struct sockaddr_in addr;

	if (!callback)
		return (-1);
	
	fd = create_new_socket(1);
	if (fd < 0) {
		LOG_ERR("%s: create new socket failed[%d]", __FUNCTION__, errno);
		return (-2);		
	}
	evutil_make_listen_socket_reuseable(fd);	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(0);
	addr.sin_port = htons(port);	
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		LOG_ERR("%s: bind port[%d] failed[%d]", __FUNCTION__, port, errno);
		return (-10);
	}
	if (listen(fd, 128) != 0) {
		LOG_ERR("%s: listen failed[%d]", __FUNCTION__, errno);
		return (-20);		
	}
	if (0 != event_assign(event_accept, base, fd, EV_READ|EV_PERSIST, cb_listen, (void *)callback)) {
		LOG_ERR("%s: event_new failed[%d]", __FUNCTION__, errno);
		goto fail;
	}
	event_add(event_accept, NULL);

	LOG_INFO("%s: %s fd = %d, port = %d, callback = %p", __FUNCTION__, name, fd, port, callback);
	return (0);
fail:
	if (fd > 0) {
		evutil_closesocket(fd);				
	}
	if (event_accept) {
		event_del(event_accept);				
	}
	return (-1);
}

int game_add_connect_event(struct sockaddr *sa, int socklen, conn_node_base *client)
{
	int ret;
	struct event *event_conn = &client->event_recv;
	int fd = 0;
	char buf[128];
	struct sockaddr_in *sin = (struct sockaddr_in *)sa;
	evutil_inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf));
	
	fd = create_new_socket(0);
	if (0 != event_assign(event_conn, base, fd, EV_READ|EV_PERSIST, cb_recv, client)) {
		LOG_ERR("%s %d: event_new failed[%d]", __FUNCTION__, __LINE__, errno);				
		goto fail;
	}
	event_add(event_conn, NULL);
	
	ret = evutil_socket_connect_(&fd, sa, socklen);
	if (ret != 1) {
		if (ret > 0)
			ret = -ret;
		if (ret == 0)
			ret = -1;
		LOG_ERR("%s %d: evutil_socket_connect failed[%d][%d]", __FUNCTION__, __LINE__, errno, ret);		
		goto fail;
	}

	game_set_socket_opt(fd);
	client->fd = fd;

	LOG_INFO("%s: connect[%s][%u] fd = %d", __FUNCTION__, buf, htons(sin->sin_port), fd);
	
	return (fd);
fail:
	if (fd > 0) {
		evutil_closesocket(fd);		
	}
	if (event_conn) {
		event_del(event_conn);		
	}
	return (-1);	
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
