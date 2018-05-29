#include <signal.h>
#include <assert.h>
#include <search.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#include <sys/socket.h>
#include <map>
#include <pthread.h>
#include "game_event.h"
#include "time_helper.h"
#include "oper_config.h"
#include "deamon.h"
#include "conn_node_client.h"
#include <evhttp.h>
#include "flow_record.h"

void generic_request_handler(struct evhttp_request *req, void *arg)
{
	struct evbuffer *returnbuffer = evbuffer_new();

	evbuffer_add_printf(returnbuffer, "Thanks for the request!");
	evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
	evbuffer_free(returnbuffer);
	return;
}
extern struct event_base *event_global_current_base_;
void init_http_server()
{
	short          http_port = 8081;
	const char          *http_addr = "0.0.0.0";
	struct evhttp *http_server = NULL;

//	base = event_init();
	event_global_current_base_ = base;
	http_server = evhttp_start(http_addr, http_port);
	evhttp_set_gencb(http_server, generic_request_handler, NULL);
}

int init_conn_client_map()
{
	return (0);
}

static void cb_signal(evutil_socket_t fd, short events, void *arg)
{
//	if (conn_node_gamesrv::server_node)
//		shutdown(conn_node_gamesrv::server_node->fd, SHUT_WR);
//	conn_node_gamesrv::server_node = NULL;
	LOG_DEBUG("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	//连接服断开，存流量数据到数据库
#ifdef FLOW_MONITOR
	save_flow_record_to_mysql();
#endif
	
}

static void cb_signal2(evutil_socket_t fd, short events, void *arg)
{
//	exit(0);
	change_mycat();
	LOG_INFO("%s: fd = %d, events = %d, arg = %p", __FUNCTION__, fd, events, arg);
	
}

static struct event connsrv_event_timer;
static struct timeval connsrv_timeout;
void cb_connsrv_timer(evutil_socket_t, short, void* /*arg*/)
{
	add_timer(connsrv_timeout, &connsrv_event_timer, NULL);

// 	HeartbeatNotify nty;
// 	heartbeat_notify__init(&nty);
// 	uint64_t times = time_helper::get_micro_time();
// 	nty.curtime = times / 1000 / 1000;

// 	PROTO_HEAD* head = conn_node_base::get_send_buf(MSG_ID_HEARTBEAT_NOTIFY, 0);
// 	size_t size = heartbeat_notify__pack(&nty, conn_node_base::get_send_data());	
// 	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);

// 	conn_node_client *client;		
// 	for (std::map<evutil_socket_t, conn_node_client *>::iterator it = conn_node_client::map_fd_nodes.begin();
// 		 it != conn_node_client::map_fd_nodes.end(); ++it)
// 	{
// 		client = it->second;
// 		if (!client) {// || client->player_id == 0) {
// 			continue;
// 		}
// //		LOG_DEBUG("%s %d: broadcast to playerid[%lu] openid[%u] fd[%u]", __FUNCTION__, __LINE__, client->player_id, client->open_id, client->fd);		
		
// 		if (client->send_one_msg(head, 1) != (int)(ENDION_FUNC_4(head->len))) {		
// //			LOG_ERR("%s: broadcast to client[%u] failed err[%d]", __FUNCTION__, client->player_id, errno);
// 			continue;
// 		}
// 	}
}

int main(int argc, char **argv)
{
	int ret = 0;
	FILE *file=NULL;
	char *line;
	int port;

	signal(SIGTERM, SIG_IGN);
	
	ret = log4c_init();
	if (ret != 0) {
		printf("log4c_init failed[%d]\n", ret);
		return (ret);
	}

	init_mycat();
	if (!mycat) {
		printf("log4c_category_get(\"six13log.log.app.application1\"); failed\n");
		return (0);
	}
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-d") == 0) {
			change_to_deamon();
//			break;
		}
	    else if(strcmp(argv[i], "-t") == 0) { /// test for mem check
			open_err_log_file();
		}
	}

	uint64_t pid = write_pid_file();		
	LOG_INFO("%s %d: conn_srv run %lu",	__FUNCTION__, __LINE__, pid);
	if (init_conn_client_map() != 0) {
		LOG_ERR("init client map failed");
		goto done;
	}

	ret = game_event_init();
	if (ret != 0)
		goto done;

	file = fopen("../server_info.ini", "r");
	if (!file) {
		LOG_ERR("open server_info.ini failed[%d]", errno);				
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"conn_srv_client_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_client_port");
		ret = -1;
		goto done;
	}

	ret = game_add_listen_event(port, conn_node_client::get_conn_node, conn_node_client::del_conn_node, "client");
 	if (ret < 0)
 		goto done;

	add_signal(SIGUSR1, NULL, cb_signal);
	add_signal(SIGUSR2, NULL, cb_signal2);		
	
	if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
		LOG_ERR("set sigpipe ign failed");		
		return (0);
	}

//	init_http_server();
	connsrv_timeout.tv_sec = 5;
	connsrv_event_timer.ev_callback = cb_connsrv_timer;
	add_timer(connsrv_timeout, &connsrv_event_timer, NULL);
	
//	ret = event_base_loop(base, 0);
	aeMain(global_el);
	aeDeleteEventLoop(global_el);
	
	LOG_INFO("srv loop stoped[%d]", ret);	

//	struct timeval tv;
//	event_base_gettimeofday_cached(base, &tv);

done:
	LOG_INFO("conn_srv stoped[%d]", ret);
	if (file)
		fclose(file);
	return (ret);
}

