#include "listen_node_client.h"
#include "game_event.h"
#include <errno.h>
#include "game_event.h"

extern void on_client_write(int fd, short ev, void *arg);

listen_node_client::listen_node_client()
{
}

listen_node_client::~listen_node_client()
{
}

int listen_node_client::listen_pre_func()
{
	return (0);	
}

int listen_node_client::listen_after_func(evutil_socket_t fd)
{
	return (0);		
}

//static uint16_t global_login_seq = 10;
//static uint32_t static_open_id = 100;
//static uint64_t static_player_id = 100000;
conn_node_base * listen_node_client::get_conn_node(evutil_socket_t fd)
{
	conn_node_client *ret = new conn_node_client();
	if (!ret) {
		LOG_ERR("[%s: %d]: alloc conn_node_client object memory failed", __FUNCTION__, __LINE__);
		return NULL;
	}
	
	ret->fd = fd;
	//ret->open_id = static_open_id++;
	//ret->player_id = static_player_id++;	
	//ret->login_seq = global_login_seq++;
	LOG_DEBUG("[%s: %d]: alloc conn_node_client fd[%u] login_seq[%u] ret[%p]", __FUNCTION__, __LINE__, ret->fd, ret->login_seq, ret);
	
	/* 创建写事件，但是在我们有数据可写之前，不要添加它。 */
//	event_set(&ret->ev_write, fd, EV_WRITE, on_write, ret);
	if (0 != event_assign(&ret->ev_write, base, fd, EV_WRITE, on_client_write, (void *)ret)) {
		LOG_ERR("%s: event_new failed[%d]", __FUNCTION__, errno);
		delete ret;
		return NULL;
	}

	conn_node_client::add_map_fd_nodes(ret);
//	conn_node_client::add_map_player_id_nodes(ret);
//	conn_node_client::add_map_open_id_nodes(ret);		

/*
	PROTO_ROLE_LOGIN data;
	data.head.len = ENDION_FUNC_4(sizeof(data));
	data.head.msg_id = ENDION_FUNC_2(SERVER_PROTO_FETCH_ROLE_REQUEST);
	data.data.player_id = player_id;
	data.data.open_id = open_id;	

	if (conn_node_server::server_node->send_one_msg(&data.head, 1) != ENDION_FUNC_4(data.head.len)) {		
		log4c_category_log(mycat, LOG4C_PRIORITY_ERROR,
			"%s: send to gameserver failed err[%d]", __FUNCTION__, errno);
		delete ret;
		ret = NULL;
	}
*/	
	return ret;
}

