#include <signal.h>
#include "shm_ipc.h"
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

static int init_conn_client_map()
{
	return (0);
}

static int cb_connsrv_timer(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
	LOG_DEBUG("%s: id = %lld", __FUNCTION__, id);
	printf("%s id = %lld\n", __FUNCTION__, id);
	return (3000);
}

static shm_ipc_obj *ipc_game_rd;
shm_ipc_obj *ipc_game_wr;
static void cb_recv_shm_ipcs(struct aeEventLoop *eventLoop)
{
	for (;;)
	{
		PROTO_HEAD *head = read_from_shm_ipc(ipc_game_rd);
		if (!head)
		{
			break;
		}
		try_read_reset(ipc_game_rd);		
	}
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

	ipc_game_rd = init_shm_from_config("conn_game_shm", file);
	ipc_game_wr = init_shm_from_config("game_conn_shm", file);
	if (!ipc_game_rd || !ipc_game_wr)
	{
		LOG_ERR("init ipc shm failed");
		ret = -1;
		goto done;
	}

	line = get_first_key(file, (char *)"conn_client_port");
	port = atoi(get_value(line));
	if (port <= 0) {
		LOG_ERR("config file wrong, no conn_srv_client_port");
		ret = -1;
		goto done;
	}

	ret = game_add_listen_event(port, conn_node_client::get_conn_node, "client");
 	if (ret < 0)
 		goto done;
	conn_node_client::listen_fd = ret;

	
	if (SIG_ERR == signal(SIGPIPE,SIG_IGN)) {
		LOG_ERR("set sigpipe ign failed");		
		return (0);
	}

	aeSetAfterSleepProc(global_el, cb_recv_shm_ipcs);
	aeCreateTimeEvent(global_el, 3000, cb_connsrv_timer, NULL, NULL);

	aeMain(global_el);
	aeDeleteEventLoop(global_el);
	
	LOG_INFO("srv loop stoped[%d]", ret);	

done:
	LOG_INFO("conn_srv stoped[%d]", ret);
	if (file)
		fclose(file);
	return (ret);
}

