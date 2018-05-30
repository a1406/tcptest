#include "conn_node.h"
#include "comm.h"
#include "game_event.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

conn_node_base::conn_node_base()
{
	fd = 0;
	pos_begin = pos_end = 0;
}

conn_node_base::~conn_node_base()
{
	assert(buf);
	assert(max_buf_len != 0);
	free(buf);
}

//uint8_t conn_node_base::global_send_buf[MAX_GLOBAL_SEND_BUF + sizeof(EXTERN_DATA)];
#ifdef CALC_NET_MSG
uint32_t send_buf_times[CS__MESSAGE__ID__MAX_MSG_ID];
uint32_t recv_buf_times[CS__MESSAGE__ID__MAX_MSG_ID];
uint32_t send_buf_size[CS__MESSAGE__ID__MAX_MSG_ID];
uint32_t recv_buf_size[CS__MESSAGE__ID__MAX_MSG_ID];
#endif

// int conn_node_base::send_one_msg(PROTO_HEAD *head, uint8_t force)
// {
// //	static int seq = 1;
// 	int ret;
// 	int send_num = 0;
// //	head->time_stamp = time(NULL);
// 	char *p = (char *)head;
// //	int len = ENDION_FUNC_2(head->len);
// 	int len = get_real_head_len(head);
// //	head->seq = ENDION_FUNC_2(seq++);
// 	for (;;) {
// 		ret = send(fd, p, len, 0);
// 		assert(ret <= len);
// 		if (ret == len)
// 			goto done;
// 		if (ret < 0) {
// 				//force可能会导致bug，如果一个数据包发送了一半然后发生eagain，这个时候如果失败可能导致后续的数据包错误
// 			if (errno != EAGAIN)// || force)
// 				goto fail;
// 				//ignore EINPROGRESS
// 			usleep(100000);
// 		} else if (ret < len) {
// 			len -= ret;
// 			p += ret;
// 			send_num += ret;
// 			usleep(100000);
// 		}
// 	}
// done:
// 	if (unlikely(head->msg_id == SERVER_PROTO_BROADCAST || head->msg_id == SERVER_PROTO_BROADCAST_ALL))
// 	{
// 		__attribute__((unused)) PROTO_HEAD *real_head;
// 		PROTO_HEAD_CONN_BROADCAST *t_head = (PROTO_HEAD_CONN_BROADCAST *)head;
// 		real_head = &t_head->proto_head;
// 		LOG_DEBUG("%s %d: %d send msg[%d] len[%d], seq[%d], ret [%d]", __PRETTY_FUNCTION__, fd, __LINE__, ENDION_FUNC_2(real_head->msg_id), ENDION_FUNC_4(real_head->len), ENDION_FUNC_2(real_head->seq), ret);
// #ifdef CALC_NET_MSG
// 	uint16_t id = ENDION_FUNC_2(real_head->msg_id);
// 	if (id < CS__MESSAGE__ID__MAX_MSG_ID) {
// 		send_buf_size[id] += len;
// 		++send_buf_times[id];
// 	}
// #endif
// 	}
// 	else if (likely(head->msg_id != 0))
// 		LOG_DEBUG("%s %d: %d send msg[%d] len[%d], seq[%d], ret [%d]", __PRETTY_FUNCTION__, fd, __LINE__, ENDION_FUNC_2(head->msg_id), ENDION_FUNC_4(head->len), ENDION_FUNC_2(head->seq), ret);
// #ifdef CALC_NET_MSG
// 	uint16_t id = ENDION_FUNC_2(head->msg_id);
// 	if (id < CS__MESSAGE__ID__MAX_MSG_ID) {
// 		send_buf_size[id] += len;
// 		++send_buf_times[id];
// 	}
// #endif

// //	return (ENDION_FUNC_2(head->len));
// 	return get_real_head_len(head);
// fail:
// 	LOG_ERR("%s fd[%d]: msg[%d] len[%u] seq[%d] ret[%d] errno[%d] send_num = %d",
// 		__PRETTY_FUNCTION__, fd, ENDION_FUNC_2(head->msg_id), get_real_head_len(head), ENDION_FUNC_2(head->seq), ret, errno, send_num);
// 	return ret;
// }


//返回0表示接收完毕，返回大于0表示没接收完毕。返回小于零表示断开
int conn_node_base::get_one_buf()
{
	int ret = -1;
	PROTO_HEAD *head;
	int len;

	if (is_full_packet()) {
		len = buf_size();
		head = (PROTO_HEAD *)buf_head();
		uint32_t real_len = get_real_head_len(head);
		if (head->msg_id != 0) {
			LOG_DEBUG("%s %d: get msg[%d] len[%u], seq[%d], max_len [%d], is full packet",
				__PRETTY_FUNCTION__, fd, ENDION_FUNC_2(head->msg_id), real_len, ENDION_FUNC_2(head->seq), len);
		}
	//	assert(ENDION_FUNC_2(head->len) >= sizeof(PROTO_HEAD));
		if (real_len < sizeof(PROTO_HEAD)) {
//			remove_listen_callback_event(this);
			LOG_ERR("[%s : %d]: get data failed, size = %d", __PRETTY_FUNCTION__, __LINE__, real_len);
			return -1;
		}
#ifdef CALC_NET_MSG
		uint16_t id = ENDION_FUNC_2(head->msg_id);
		if (id < CS__MESSAGE__ID__MAX_MSG_ID) {
			++recv_buf_times[id];
			uint32_t real_len = get_real_head_len(head);
			recv_buf_size[id] += real_len;
		}
#endif
		return (0);
	}

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
	len = buf_size();
	assert((int32_t)pos_end>=ret);

	if (len < (int)sizeof(PROTO_HEAD)) {  //没有够一个包头
//		LOG_INFO("[%s : %d]: packet header error, len: %d, leave: %d", __PRETTY_FUNCTION__, __LINE__, len, buf_leave());
		return (1);
	}

	head = (PROTO_HEAD *)buf_head();
	int real_len = get_real_head_len(head);
	if (len >= real_len) { //读完了
		if (head->msg_id != 0) {
			LOG_DEBUG("%s %d: get msg[%d] len[%u], seq[%d], max_len [%d]",
				__PRETTY_FUNCTION__, fd, ENDION_FUNC_2(head->msg_id), real_len, ENDION_FUNC_2(head->seq), len);
		}

		if (real_len < (int)sizeof(PROTO_HEAD)) {
			//			remove_listen_callback_event(this);
	//		LOG_ERR("[%s : %d]: get data failed 2, leave: %d", __PRETTY_FUNCTION__, __LINE__, buf_leave());
			return -1;
		}
#ifdef CALC_NET_MSG
		uint16_t id = ENDION_FUNC_2(head->msg_id);
		if (id < CS__MESSAGE__ID__MAX_MSG_ID) {
			++recv_buf_times[id];
			recv_buf_size[id] += real_len;
		}
#endif
		return (0);
	}

	LOG_INFO("%s %d: len not enough, msg[%d] len[%d], max_len [%d], buf leave: %d",	__PRETTY_FUNCTION__, fd, ENDION_FUNC_2(head->msg_id), real_len, len, buf_leave());
	return (1);    //没有读完
}

//返回0表示正常，返回大于0表示还有包没有处理完, 不会小于0
int conn_node_base::remove_one_buf()
{
	PROTO_HEAD *head;
	int buf_len;
	int msg_id;
	int len = buf_size();
	assert(len >= (int)sizeof(PROTO_HEAD));

	head = (PROTO_HEAD *)buf_head();
//	buf_len = ENDION_FUNC_2(head->len);
	buf_len = get_real_head_len(head);
	msg_id = ENDION_FUNC_2(head->msg_id);
	assert(len >= buf_len);

	if (msg_id != 0) {
		LOG_DEBUG("%s %d: msg[%d] len[%d], reset_len [%d] move_len[%d]",
			__PRETTY_FUNCTION__, fd, msg_id, buf_len, len, len - buf_len);
	}

	if (len == buf_len) {
		pos_begin = pos_end = 0;
		return (0);
	}

	pos_begin += buf_len;
	if (is_full_packet()) {
		return (0);
	}

	len = buf_size();
	memmove(&buf[0], buf_head(), len);
	pos_begin = 0;
	pos_end = len;

	LOG_DEBUG("%s %d: memmove happened, len = %d", __PRETTY_FUNCTION__, fd, len);

//	memmove(head,
//		head + buf_len,
//		len - buf_len);
//	pos_begin -= buf_len;
//	pos_end -= buf_len;
	return (1);

//	head = (PROTO_HEAD *)buf_head();

//	len = buf_size();

//	LOG_DEBUG("%s %d: msg[%d] len[%d], reset_len [%d] move_len[%d]",
//		__PRETTY_FUNCTION__, fd, msg_id, buf_len, len, len - buf_len);

//	if (len >= (int)sizeof(PROTO_HEAD)
//		&& len >= ENDION_FUNC_2(head->len))
//		return (1);
//	return (0);
}

void conn_node_base::send_data_to_client()
{
	if (send_buffer_end_pos-send_buffer_begin_pos<=0)
		return;

	int len = write(this->fd, send_buffer + send_buffer_begin_pos, send_buffer_end_pos-send_buffer_begin_pos);

	LOG_DEBUG("%s %d: write to fd: %u: ret %d, end pos = %d, begin pos = %d", __PRETTY_FUNCTION__, __LINE__, fd, len, send_buffer_end_pos, send_buffer_begin_pos);

	if (len == -1) {  //发送失败
		if (errno == EINTR || errno == EAGAIN) {
		}
	}
	else if (send_buffer_begin_pos + len < send_buffer_end_pos) {  //没发完
		send_buffer_begin_pos += len;
	}
	else {  //发完了
		send_buffer_begin_pos = send_buffer_end_pos = 0;
		aeDeleteFileEvent(global_el, fd, AE_WRITABLE);		
		return;
	}

	if (send_buffer_end_pos >= send_buffer_size / 2
		&& (send_buffer_begin_pos / 1024) > 0
		&& send_buffer_end_pos>send_buffer_begin_pos)
	{
		int sz = send_buffer_end_pos - send_buffer_begin_pos;
		memmove(send_buffer, send_buffer+send_buffer_begin_pos, sz);
		send_buffer_begin_pos = 0;
		send_buffer_end_pos = sz;
	}
}

int conn_node_base::send_one_msg(PROTO_HEAD *head)
{
	char *p = (char *)head;
	int len = ENDION_FUNC_4(head->len);
	return send_one_buffer(this, p, len);
}

void conn_node_base::on_connected()
{
}
void conn_node_base::on_disconnected()
{
}
int  conn_node_base::disconnect()
{
	on_disconnected();
	
	flag |= NODE_DISCONNECTING;
		//ondisconnected();
    if (fd > 0)
	{
        close(fd);
	}

	aeDeleteFileEvent(global_el, fd, AE_READABLE);
	aeDeleteFileEvent(global_el, fd, AE_WRITABLE);
	fd = 0;
	return (0);
}
int  conn_node_base::del()
{
	return (0);
}


// int fast_send_msg_base(conn_node_base* node, EXTERN_DATA *extern_data, uint16_t msg_id, size_t size, uint16_t seq)
// {
// 	if (size != (size_t)-1)
// 	{
// 		PROTO_HEAD *head = node->get_send_buf(msg_id, seq);
// 		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
// 		head->seq = node->get_seq();
// 		node->add_extern_data(head, extern_data);
// 		int ret = node->send_one_msg(head, 1);
// 		if (ret != (int)ENDION_FUNC_4(head->len))
// 		{
// 			LOG_ERR("[%s:%d] send to client failed err[%d]", __FUNCTION__, __LINE__, errno);
// 			return -1;
// 		}

// 		return 0;
// 	}

// 	return -1;
// }
