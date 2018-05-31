#include "conn_node_buf.h"
#include <assert.h>

static	inline int buf_size(CONN_NODE *node)
{
	return node->pos_end - node->pos_begin;
}
uint8_t * buf_head(CONN_NODE *node)
{ 
	return node->buf + node->pos_begin;
}
static 	inline uint8_t * buf_tail(CONN_NODE *node)
{ 
	return node->buf + node->pos_end;
}
static 	inline int buf_leave(CONN_NODE *node)
{ 
	return node->max_buf_len - node->pos_end;		
}

static inline bool is_full_packet(CONN_NODE *node)
{
	uint32_t len = buf_size(node);

	if (len < sizeof(PROTO_HEAD))  //没有够一个包头
		return (false);

	PROTO_HEAD *head = (PROTO_HEAD *)buf_head(node);
	uint32_t real_len = head->len;
	if (len >= real_len)
		return true;
	return false;
}	

int get_one_buf(CONN_NODE *node)
{
	int ret = -1;
	PROTO_HEAD *head;
	int len;

	if (is_full_packet(node)) {
		len = buf_size(node);
		head = (PROTO_HEAD *)buf_head(node);
		uint32_t real_len = head->len;

		if (real_len < sizeof(PROTO_HEAD)) {
			printf("[%s : %d]: get data failed, size = %d\n", __PRETTY_FUNCTION__, __LINE__, real_len);
			return -1;
		}
		return (0);
	}

	ret = recv(node->fd, buf_tail(node), buf_leave(node), 0);
	if (ret == 0) {
		return (-1);
	}
	else if (ret < 0) {
		if (errno != EAGAIN && errno != EINTR) {
			return (-1);
		}
		else {
			return 2;
		}
	}
	else {
		node->pos_end += ret;
	}
	len = buf_size(node);
	assert((int32_t)node->pos_end>=ret);

	if (len < (int)sizeof(PROTO_HEAD)) {  //没有够一个包头
		return (1);
	}

	head = (PROTO_HEAD *)buf_head(node);
	int real_len = head->len;
	if (len >= real_len) { //读完了
		if (head->msg_id != 0) {
		}

		if (real_len < (int)sizeof(PROTO_HEAD)) {
			return -1;
		}
		return (0);
	}

	printf("%s %d: len not enough, len[%d], max_len [%d], buf leave: %d\n",	__PRETTY_FUNCTION__, node->fd, real_len, len, buf_leave(node));
	return (1);    //没有读完
}

int remove_one_buf(CONN_NODE *node)
{
	PROTO_HEAD *head;
	int buf_len;
	int len = buf_size(node);
	assert(len >= (int)sizeof(PROTO_HEAD));

	head = (PROTO_HEAD *)buf_head(node);
//	buf_len = ENDION_FUNC_2(head->len);
	buf_len = head->len;
//	msg_id = ENDION_FUNC_2(head->msg_id);
	assert(len >= buf_len);

	if (len == buf_len) {
		node->pos_begin = node->pos_end = 0;
		return (0);
	}

	node->pos_begin += buf_len;
	if (is_full_packet(node)) {
		return (0);
	}

	len = buf_size(node);
	memmove(&node->buf[0], buf_head(node), len);
	node->pos_begin = 0;
	node->pos_end = len;

	printf("%s %d: memmove happened, len = %d\n", __PRETTY_FUNCTION__, node->fd, len);

	return (1);

}

