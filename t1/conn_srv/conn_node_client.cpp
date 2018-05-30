#include "conn_node_client.h"
#include "time_helper.h"
#include "game_event.h"
#include "tea.h"
#include "flow_record.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

conn_node_client::conn_node_client()
{
}

conn_node_client::~conn_node_client()
{
	send_player_exit();	
}

int conn_node_client::decode_and_check_crc(PROTO_HEAD *head)
{
/*
	int len = ENDION_FUNC_4(head->len) - sizeof(uint16_t);
	char *p = (char *)&head->msg_id;
	int pos = 0;
	while (pos + 8 <= len)
	{
		sg_decrypt((uint32_t *)&p[pos]);
		pos += 8;
	}
		//check seq

		//心跳包不检测
	if (ENDION_FUNC_2(head->msg_id) == 0)
	{
		++seq;
		return (0);
	}

	if (ENDION_FUNC_2(head->msg_id) == LOGIN_REQUEST)
		seq = ENDION_FUNC_2(head->seq);

	if (seq != ENDION_FUNC_2(head->seq))
	{
		LOG_ERR("%s %d: check seq fail, msg id = %d seq[%d] head->seq[%d]", __PRETTY_FUNCTION__, __LINE__, ENDION_FUNC_2(head->msg_id), seq, ENDION_FUNC_2(head->seq));
		return (-1);
	}

	uint32_t crc32 = crc32_long((u_char*)head->data, ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD));
	if (head->crc != ENDION_FUNC_4(crc32))
	{
		LOG_ERR("%s %d: check crc fail, msg id = %d seq[%d] head->seq[%d]", __PRETTY_FUNCTION__, __LINE__, ENDION_FUNC_2(head->msg_id), seq, ENDION_FUNC_2(head->seq));
		return (-10);
	}
	++seq;
*/
	return (0);
}

int conn_node_client::send_hello_resp()
{
	struct timeval tv;
	time_helper::get_current_time(&tv);
//	tv.tv_sec;
	return (0);
}

// void conn_node_client::memmove_data()
// {
// 	int len = buf_size();
// 	if (len == 0)
// 		return;
// 	memmove(&buf[0], buf_head(), len);
// 	pos_begin = 0;
// 	pos_end = len;

// 	LOG_DEBUG("%s %d: memmove happened, len = %d", __PRETTY_FUNCTION__, fd, len);
// }
// void conn_node_client::remove_buflen(int len)
// {
// 	assert(len <= buf_size());
// 	if (len == buf_size())
// 	{
// 		pos_begin = pos_end = 0;
// 		return;
// 	}
// 	pos_begin += len;
// 	return;
// }

int conn_node_client::get_listen_fd()
{
	return conn_node_client::listen_fd;
}

int conn_node_client::recv_func(evutil_socket_t fd)
{
	PROTO_HEAD *head;
//	uint32_t old_len;
//	EXTERN_DATA save_data;
//	EXTERN_DATA *extern_data;

	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)buf_head();

			send_one_msg(head);

// 			if (decode_and_check_crc(head) != 0) {
// 				LOG_INFO("%s %d: crc err, connect closed from fd %u, err = %d", __PRETTY_FUNCTION__, __LINE__, fd, errno);
// 				return (-1);
// 			}

// 			uint32_t cmd = ENDION_FUNC_2(head->msg_id);
// 			if (0 == cmd) {
// 				send_hello_resp();
// //				this->send_one_msg(head, 1);
// 			} else {
// 				old_len = ENDION_FUNC_4(head->len);
// 				extern_data = (EXTERN_DATA *)&head->data[old_len - sizeof(PROTO_HEAD)];
// 				memcpy(&save_data, extern_data, sizeof(EXTERN_DATA));
// 				head->len = ENDION_FUNC_4(old_len + sizeof(EXTERN_DATA));

// 				extern_data->player_id = player_id;
// 				extern_data->open_id = open_id;
// 				extern_data->fd = fd;
// 				extern_data->port = sock.sin_port;

// 				transfer_to_dumpserver(head);
// #ifdef FLOW_MONITOR
// 				add_one_client_request(head);
// #endif
// 				if (player_id == 0) {
// 					if (transfer_to_loginserver() != 0) {
// 						remove_listen_callback_event(this);
// 						return (0);
// 					}
// 				} else if (dispatch_message() != 0) {
// 				}

// //				transfer_to_gameserver();
// 				head->len = ENDION_FUNC_4(old_len);
// 				memcpy(extern_data, &save_data, sizeof(EXTERN_DATA));
//			}
		}

		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __PRETTY_FUNCTION__, fd, errno);
//		send_logout_request();
//		del_client_map_by_fd(fd, &client_maps[0], (int *)&num_client_map);
			return (-1);
		} else if (ret > 0) {
			break;
		}

		ret = remove_one_buf();
	}
	return (0);
}

void encoder_data(PROTO_HEAD *head) {
/*
	size_t sz = ENDION_FUNC_4(head->len) - sizeof(uint16_t);
	if (sz <= 0)
		return;

	char* pData = (char*)head+sizeof(uint16_t);
	uint32_t crc32 = crc32_long((u_char*)head->data, ENDION_FUNC_4(head->len) - sizeof(PROTO_HEAD));
	head->crc = ENDION_FUNC_4(crc32);

	char * p = (char *)(pData);

	for (size_t i=0; i<sz/8; ++i) {
		sg_encrypt((uint32_t *)p);
		p += 8;
	}
*/
}

int conn_node_client::send_player_exit(bool again/* = false*/)
{
	return (0);
}

int conn_node_client::dispatch_message()
{
	return (0);
}

int conn_node_client::transfer_to_raidserver()
{
	return (0);
}

int conn_node_client::transfer_to_gameserver()
{
	return (0);
}

int conn_node_client::transfer_to_loginserver()
{
	return (0);	
}

int conn_node_client::transfer_to_dumpserver(PROTO_HEAD *head)
{
	return (0);
}

int conn_node_client::transfer_to_mailsrv()
{
	return (0);
}
int conn_node_client::transfer_to_friendsrv()
{
	return (0);	
}

int conn_node_client::transfer_to_guildsrv()
{
	return (0);		
}

int conn_node_client::transfer_to_ranksrv()
{
	return (0);			
}

int conn_node_client::transfer_to_doufachang()
{
	return (0);
}

int conn_node_client::transfer_to_tradesrv()
{
	return (0);	
}

int conn_node_client::transfer_to_activitysrv()
{
	return (0);
}

//////////////////////////// 下面是static 函数
std::map<evutil_socket_t, conn_node_client *> conn_node_client::map_fd_nodes;
std::map<uint64_t, conn_node_client *> conn_node_client::map_player_id_nodes;
std::map<uint32_t, conn_node_client *> conn_node_client::map_open_id_nodes;
int conn_node_client::listen_fd;

conn_node_base *conn_node_client::get_conn_node(int fd)
{
	conn_node_client *ret = new conn_node_client;
	ret->buf = (uint8_t *)malloc(128 * 1024);
	ret->max_buf_len = 128 * 1024;
	ret->send_buffer = (char *)malloc(128 * 1024);
	ret->send_buffer_size = 128 * 1024;
	ret->open_id = 0;
	ret->player_id = 0;

	assert(map_fd_nodes.find(fd) == map_fd_nodes.end());
	map_fd_nodes[fd] = ret;
	return ret;
}

int conn_node_client::del()
{
	assert(fd > 0);
	free(buf);
	free(send_buffer);	
		
	map_fd_nodes.erase(fd);
	if (player_id > 0)
		map_player_id_nodes.erase(player_id);
	if (open_id > 0)
		map_open_id_nodes.erase(open_id);	
	delete this;
	return (0);
}

int conn_node_client::add_map_fd_nodes(conn_node_client *client)
{
	map_fd_nodes[client->fd]=  client;
	return (0);
}
int conn_node_client::add_map_player_id_nodes(conn_node_client *client)
{
		//todo check duplicate
	if (client->player_id > 0)
		map_player_id_nodes[client->player_id] = client;
	return (0);
}

int conn_node_client::add_map_open_id_nodes(conn_node_client *client)
{
		//todo check duplicate
	if (client->open_id > 0)
		map_open_id_nodes[client->open_id] = client;
	return (0);
}

conn_node_client *conn_node_client::get_nodes_by_fd(evutil_socket_t fd, uint16_t port)
{
	std::map<evutil_socket_t, conn_node_client *>::iterator it = map_fd_nodes.find(fd);
	if (it != map_fd_nodes.end() && it->second->port == port)
		return it->second;
	return NULL;
}

conn_node_client *conn_node_client::get_nodes_by_player_id(uint64_t player_id)
{
	std::map<uint64_t, conn_node_client *>::iterator it = map_player_id_nodes.find(player_id);
	if (it != map_player_id_nodes.end())
		return it->second;
	return NULL;
}

conn_node_client *conn_node_client::get_nodes_by_open_id(uint32_t open_id)
{
	std::map<uint32_t, conn_node_client *>::iterator it = map_open_id_nodes.find(open_id);
	if (it != map_open_id_nodes.end())
		return it->second;
	return NULL;
}


