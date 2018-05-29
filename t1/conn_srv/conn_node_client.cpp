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
	open_id = 0;
	player_id = 0;
	handshake = false;
	conn_step = 1;
	raidsrv_id = -1;

	send_buffer_begin_pos = 0;
	send_buffer_end_pos = 0;

	max_buf_len = 10 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_client::~conn_node_client()
{
	LOG_DEBUG("%s %d: openid[%u] fd[%u] playerid[%lu]", __PRETTY_FUNCTION__, __LINE__, open_id, fd, player_id);
	map_fd_nodes.erase(fd);
	if (open_id > 0) {
		map_open_id_nodes.erase(open_id);
	}
	if (player_id > 0) {
		map_player_id_nodes.erase(player_id);
	} else {
		return;
	}

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

int conn_node_client::respond_websocket_request()
{
	return (0);
}

void conn_node_client::memmove_data()
{
	int len = buf_size();
	if (len == 0)
		return;
	memmove(&buf[0], buf_head(), len);
	pos_begin = 0;
	pos_end = len;

	LOG_DEBUG("%s %d: memmove happened, len = %d", __PRETTY_FUNCTION__, fd, len);
}
void conn_node_client::remove_buflen(int len)
{
	assert(len <= buf_size());
	if (len == buf_size())
	{
		pos_begin = pos_end = 0;
		return;
	}
	pos_begin += len;
	return;
}

void conn_node_client::on_recv_frame()
{
}

int conn_node_client::frame_read_cb(evutil_socket_t fd)
{
	return (0);
}

int conn_node_client::recv_from_fd()
{
	int ret = -1;
	
	ret = recv(fd, buf_tail(), buf_leave(), 0);
	if (ret == 0) {
		LOG_INFO("%s %d %d: recv ret [%d] err [%d] buf[%p] pos_begin[%d] pos_end[%d]", __PRETTY_FUNCTION__, __LINE__, fd, ret, errno, buf, pos_begin, pos_end);
		return (-1);
	}
	else if (ret < 0) {
		if (errno != EAGAIN && errno != EINTR) {
			LOG_ERR("%s %d %d: recv ret [%d] err [%d] buf[%p] pos_begin[%d] pos_end[%d]", __PRETTY_FUNCTION__, __LINE__, fd, ret, errno, buf, pos_begin, pos_end);
			return (-1);
		}
		else {
//			LOG_DEBUG("%s %d %d: recv ret [%d] err [%d] buf[%p] pos_begin[%d] pos_end[%d]", __PRETTY_FUNCTION__, __LINE__, fd, ret, errno, buf, pos_begin, pos_end);
			return 2;
		}
	}
	else {
		pos_end += ret;
	}
	assert((int32_t)pos_end>=ret);
	return (0);
}

int conn_node_client::recv_handshake(evutil_socket_t fd)
{
	int ret = recv_from_fd();
	if (ret != 0)
		return ret;
	int len = buf_size();

	char *end = (char *)buf_tail();
	if (len >= 4 && end[-1] == '\n' && end[-2] == '\r' && end[-3] == '\n' && end[-4] == '\r') {
		if (respond_websocket_request() != 0) //send websocket response
			return -1;
//		LOG_INFO("[%s : %d]: packet header error, len: %d, leave: %d", __PRETTY_FUNCTION__, __LINE__, len, buf_leave());
		pos_begin = pos_end = 0;
		handshake = true;
		return (0);
	}
	return (0);
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

			send_one_msg(head, 1);

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

int conn_node_client::send_one_buffer(char *buffer, uint32_t len)
{
#if 1
#ifdef FLOW_MONITOR
	add_one_client_answer(head);
#endif
	if (send_buffer_end_pos+len >= MAX_CLIENT_SEND_BUFFER_SIZE) {  ///缓冲区溢出, 关闭连接
		LOG_DEBUG("[%s: %d]: fd: %d: send msg len[%d], begin[%d]end[%d] buffer full", __PRETTY_FUNCTION__, __LINE__, fd, len, send_buffer_begin_pos, send_buffer_end_pos);
		return -1;
	}

	memcpy(send_buffer+send_buffer_end_pos, buffer, len);
	encoder_data((PROTO_HEAD*)(send_buffer+send_buffer_end_pos));

	send_buffer_end_pos += len;

	if (send_buffer_begin_pos == 0) {
		int result = event_add(&this->ev_write, NULL);
		if (0 != result) {
			LOG_ERR("[%s : %d]: event add failed, result: %d", __PRETTY_FUNCTION__, __LINE__, result);
			return result;
		}
	}

	return 0;
#else
	return conn_node_base::send_one_msg(head, force);
#endif
}


int conn_node_client::send_one_msg(PROTO_HEAD *head, uint8_t force) {
	{
		static uint8_t dump_buf[MAX_GLOBAL_SEND_BUF + sizeof(EXTERN_DATA)];
		memcpy(dump_buf, head, ENDION_FUNC_4(head->len));
		PROTO_HEAD *head1 = (PROTO_HEAD *)dump_buf;
		EXTERN_DATA ext_data;
		ext_data.open_id = open_id;
		ext_data.player_id = player_id;
		ext_data.fd = fd;
		ext_data.port = sock.sin_port;
		add_extern_data(head1, &ext_data);
		transfer_to_dumpserver(head1);
	}
#if 1

#ifdef FLOW_MONITOR
	add_one_client_answer(head);
#endif

//	static int seq = 1;
	char *p = (char *)head;
	int len = ENDION_FUNC_4(head->len);
//	head->seq = ENDION_FUNC_2(seq++);

	if (send_buffer_end_pos+len >= MAX_CLIENT_SEND_BUFFER_SIZE) {  ///缓冲区溢出, 关闭连接
		LOG_DEBUG("[%s: %d]: fd: %d: send msg[%d] len[%d], seq[%d] begin[%d]end[%d] buffer full", __PRETTY_FUNCTION__, __LINE__, fd, ENDION_FUNC_2(head->msg_id), ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq), send_buffer_begin_pos, send_buffer_end_pos);
		return -1;
	}


	memcpy(send_buffer+send_buffer_end_pos, p, len);
	encoder_data((PROTO_HEAD*)(send_buffer+send_buffer_end_pos));

	send_buffer_end_pos += len;

	if (send_buffer_begin_pos == 0) {
		int result = event_add(&this->ev_write, NULL);
		if (0 != result) {
			LOG_ERR("[%s : %d]: event add failed, result: %d", __PRETTY_FUNCTION__, __LINE__, result);
			return result;
		}
	}

	return 0;
#else
	return conn_node_base::send_one_msg(head, force);
#endif
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
	if (it != map_fd_nodes.end() && it->second->sock.sin_port == port)
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


void on_client_write(int fd, short ev, void *arg) {
	assert(arg);
	conn_node_client *client = (conn_node_client *)arg;
	client->send_data_to_client();
}

void conn_node_client::send_data_to_client() {
	if (send_buffer_end_pos-send_buffer_begin_pos<=0)
		return;

	int len = write(this->fd, send_buffer + send_buffer_begin_pos, send_buffer_end_pos-send_buffer_begin_pos);

	LOG_DEBUG("%s %d: write to fd: %u: ret %d, end pos = %d, begin pos = %d", __PRETTY_FUNCTION__, __LINE__, fd, len, send_buffer_end_pos, send_buffer_begin_pos);

	if (len == -1) {  //发送失败
		if (errno == EINTR || errno == EAGAIN) {
			int result = event_add(&this->ev_write, NULL);
			if (0 != result) {
				LOG_ERR("[%s : %d]: event add failed, result: %d", __PRETTY_FUNCTION__, __LINE__, result);
				return;
			}
		}
	}
	else if (send_buffer_begin_pos + len < send_buffer_end_pos) {  //没发完
		send_buffer_begin_pos += len;
		int result = event_add(&this->ev_write, NULL);
		if (0 != result) {
			LOG_ERR("[%s : %d]: event add failed, result: %d", __PRETTY_FUNCTION__, __LINE__, result);
			return;
		}
	}
	else {  //发完了
		send_buffer_begin_pos = send_buffer_end_pos = 0;
		return;
	}


	/// 当数据发送完毕后pos归0
//	if (send_buffer_begin_pos == send_buffer_end_pos) {
//		send_buffer_begin_pos = send_buffer_end_pos = 0;
//		return;
//	}

	if (send_buffer_end_pos>=MAX_CLIENT_SEND_BUFFER_SIZE/2 && (send_buffer_begin_pos/1024) > 0 && send_buffer_end_pos>send_buffer_begin_pos) {
		int sz = send_buffer_end_pos - send_buffer_begin_pos;
		memmove(send_buffer, send_buffer+send_buffer_begin_pos, sz);
		send_buffer_begin_pos = 0;
		send_buffer_end_pos = sz;
	}
}
