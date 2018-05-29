#ifndef _CONN_NODE_H__
#define _CONN_NODE_H__

#include "server_proto.h"
#include "ae_net.h"
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <map>

#define ENDION_FUNC_2 htole16
#define ENDION_FUNC_4 htole32
#define ENDION_FUNC_8 htole64
/*
//前面已经定义了resp，直接打包发送
#define FAST_SEND_TO_CLIENT(MSGID, PACK_FUNC) \
	size_t size = PACK_FUNC(&resp, get_send_data());					\
	if (size != (size_t)-1) {													\
		PROTO_HEAD *head = get_send_buf(MSGID, 0);						\
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
		head->seq = get_seq();											\
		add_extern_data(head, extern_data);								\
		ret = connecter.send_one_msg(head, 1);							\
		if (ret != ENDION_FUNC_4(head->len)) {									\
			LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
		}																\
	}

//在别的模块打包resp，然后发送
#define FAST_SEND_TO_CLIENT2(MSGID, PACK_FUNC) \
	size_t size = PACK_FUNC(get_send_data());							\
	if (size != (size_t)-1) {													\
		PROTO_HEAD *head = get_send_buf(MSGID, 0);						\
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
		head->seq = get_seq();											\
		add_extern_data(head, extern_data);								\
		ret = connecter.send_one_msg(head, 1);							\
		if (ret != ENDION_FUNC_4(head->len)) {									\
			LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
		}																\
	}

//在别的模块打包resp，然后发送，多一个参数传递
#define FAST_SEND_TO_CLIENT3(MSGID, PACK_FUNC, PARAM)							\
	size_t size = PACK_FUNC(get_send_data(), PARAM);					\
	if (size != (size_t)-1) {											\
		PROTO_HEAD *head = get_send_buf(MSGID, 0);						\
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
		head->seq = get_seq();											\
		add_extern_data(head, extern_data);								\
		ret = connecter.send_one_msg(head, 1);							\
		if (ret != ENDION_FUNC_4(head->len)) {									\
			LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
		}																\
	}


//在别的模块打包resp，然后发送，多二个参数传递
#define FAST_SEND_TO_CLIENT4(MSGID, PACK_FUNC, PARAM1, PARAM2)			\
	size_t size = PACK_FUNC(get_send_data(), PARAM1, PARAM2);					\
	if (size != (size_t)-1) {											\
	PROTO_HEAD *head = get_send_buf(MSGID, 0);						\
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
	head->seq = get_seq();											\
	add_extern_data(head, extern_data);								\
	ret = connecter.send_one_msg(head, 1);							\
	if (ret != ENDION_FUNC_4(head->len)) {									\
	LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
	}																\
	}

//在别的模块打包resp，然后发送
#define FAST_SEND_TO_CLIENT5(MSGID, PACK_FUNC, PARAM1, PARAM2, PARAM3) \
	size_t size = PACK_FUNC(get_send_data(), PARAM1, PARAM2, PARAM3);  \
	if (size != (size_t)-1) {										\
	PROTO_HEAD *head = get_send_buf(MSGID, 0);						\
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
	head->seq = get_seq();											\
	add_extern_data(head, extern_data);								\
	ret = connecter.send_one_msg(head, 1);							\
	if (ret != ENDION_FUNC_4(head->len)) {									\
	LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
	}																\
	}

//////////////////
//前面已经定义了resp，直接打包发送
#define SO_FAST_SEND_TO_CLIENT(CONN, MSGID, PACK_FUNC)					\
	size_t size = PACK_FUNC(&resp, CONN->get_send_data());					\
	if (size != (size_t)-1) {													\
		PROTO_HEAD *head = CONN->get_send_buf(MSGID, 0);						\
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
		head->seq = CONN->get_seq();											\
		CONN->add_extern_data(head, extern_data);								\
		ret = CONN->connecter.send_one_msg(head, 1);							\
		if (ret != ENDION_FUNC_4(head->len)) {									\
			LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
		}																\
	}

//在别的模块打包resp，然后发送
#define SO_FAST_SEND_TO_CLIENT2(MSGID, PACK_FUNC) \
	size_t size = PACK_FUNC(&resp, CONN->get_send_data());							\
	if (size != (size_t)-1) {													\
		PROTO_HEAD *head = CONN->get_send_buf(MSGID, 0);						\
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
		head->seq = CONN->get_seq();											\
		CONN->add_extern_data(head, extern_data);								\
		ret = CONN->connecter.send_one_msg(head, 1);							\
		if (ret != ENDION_FUNC_4(head->len)) {									\
			LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
		}																\
	}

//在别的模块打包resp，然后发送，多一个参数传递
#define SO_FAST_SEND_TO_CLIENT3(MSGID, PACK_FUNC, PARAM)							\
	size_t size = PACK_FUNC(&resp, CONN->get_send_data(), PARAM);					\
	if (size != (size_t)-1) {											\
		PROTO_HEAD *head = CONN->get_send_buf(MSGID, 0);						\
		head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
		head->seq = CONN->get_seq();											\
		CONN->add_extern_data(head, extern_data);								\
		ret = CONN->connecter.send_one_msg(head, 1);							\
		if (ret != ENDION_FUNC_4(head->len)) {									\
			LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
		}																\
	}


//在别的模块打包resp，然后发送，多二个参数传递
#define SO_FAST_SEND_TO_CLIENT4(MSGID, PACK_FUNC, PARAM1, PARAM2)			\
	size_t size = PACK_FUNC(&resp, CONN->get_send_data(), PARAM1, PARAM2);					\
	if (size != (size_t)-1) {											\
	PROTO_HEAD *head = CONN->get_send_buf(MSGID, 0);						\
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
	head->seq = CONN->get_seq();											\
	CONN->add_extern_data(head, extern_data);								\
	ret = CONN->connecter.send_one_msg(head, 1);							\
	if (ret != ENDION_FUNC_4(head->len)) {									\
	LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
	}																\
	}

//在别的模块打包resp，然后发送
#define SO_FAST_SEND_TO_CLIENT5(MSGID, PACK_FUNC, PARAM1, PARAM2, PARAM3) \
	size_t size = PACK_FUNC(&resp, CONN->get_send_data(), PARAM1, PARAM2, PARAM3);  \
	if (size != (size_t)-1) {										\
	PROTO_HEAD *head = CONN->get_send_buf(MSGID, 0);						\
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);					\
	head->seq = CONN->get_seq();											\
	CONN->add_extern_data(head, extern_data);								\
	ret = CONN->connecter.send_one_msg(head, 1);							\
	if (ret != ENDION_FUNC_4(head->len)) {									\
	LOG_ERR("%s: send to client failed err[%d]", __FUNCTION__, errno); \
	}																\
	}
*/

//#define MAX_BUF_PER_CLIENT (128 * 1024 - 1 - sizeof(EXTERN_DATA))
#define MAX_GLOBAL_SEND_BUF (256 * 1024 - 1 - sizeof(EXTERN_DATA))
class conn_node_base
{
public:
	conn_node_base();
	virtual ~conn_node_base();

	virtual int recv_func(evutil_socket_t fd) = 0;
	virtual int send_one_msg(PROTO_HEAD *head, uint8_t force);	
	virtual struct event* get_write_event() { return NULL; }
	
	evutil_socket_t fd;
	uint16_t flag;
	uint16_t port;
//	struct sockaddr_in sock;
	uint32_t pos_begin;	
	uint32_t pos_end;	
	uint8_t *buf;//[MAX_BUF_PER_CLIENT + sizeof(EXTERN_DATA)];
	uint32_t max_buf_len;

	char *send_buffer;
	int32_t send_buffer_begin_pos;
	int32_t send_buffer_end_pos;
	int32_t send_buffer_size;

	aeFileProc *on_write;
	
	static inline PROTO_HEAD *get_send_buf(uint16_t msg_id, uint16_t seq)
	{
		PROTO_HEAD *head = (PROTO_HEAD *)global_send_buf;
		head->msg_id = ENDION_FUNC_2(msg_id);
		head->seq = seq;
		return head;
	}

	static inline uint8_t *get_send_data()
	{
		PROTO_HEAD *head = (PROTO_HEAD *)global_send_buf;
		return (uint8_t *)&head->data[0];
	}
	
	static inline void add_extern_data(PROTO_HEAD *head, EXTERN_DATA *data)
	{
		EXTERN_DATA *extern_data;
		int old_len = ENDION_FUNC_4(head->len);
		extern_data = (EXTERN_DATA *)(&head->data[0] + old_len - sizeof(PROTO_HEAD));
		memcpy(extern_data, data, sizeof(EXTERN_DATA));
		head->len = ENDION_FUNC_4(old_len + sizeof(EXTERN_DATA));		
/*		
		int old_crc = ENDION_FUNC_4(head->crc);
		if (old_len == 0)
			extern_data = (EXTERN_DATA *)(&head->data[0] + old_crc - sizeof(PROTO_HEAD));
		else
			extern_data = (EXTERN_DATA *)(&head->data[0] + old_len - sizeof(PROTO_HEAD));
		memcpy(extern_data, data, sizeof(EXTERN_DATA));

		if (old_len == 0)
			head->crc = ENDION_FUNC_4(old_crc + sizeof(EXTERN_DATA));
		else
			head->len = ENDION_FUNC_4(old_len + sizeof(EXTERN_DATA));
*/			
	}
	static uint8_t global_send_buf[MAX_GLOBAL_SEND_BUF + sizeof(EXTERN_DATA)];
//protected:

	inline uint32_t get_real_head_len(PROTO_HEAD *head)
	{
//	if (!head)
//		head = (PROTO_HEAD *)buf_head();
		assert(head);

		uint32_t real_len;
//	if (head->len == 0)
//	{
//		real_len = ENDION_FUNC_4(head->crc);
//	}
//	else
		{
			real_len = ENDION_FUNC_4(head->len);
		}
		return real_len;
	}

	inline int buf_size() {
		return pos_end - pos_begin;
	}
	inline uint8_t * buf_head() { 
		return buf + pos_begin;
	}
	inline uint8_t * buf_tail() { 
		return buf + pos_end;
	}
	inline int buf_leave() { 
//		return MAX_BUF_PER_CLIENT - pos_end;
		return max_buf_len - pos_end;		
	}

	inline int get_cmd()
	{
		PROTO_HEAD *head;
		head = get_head();
		return ENDION_FUNC_2(head->msg_id);
	}
	inline PROTO_HEAD *get_head()
	{
		return (PROTO_HEAD *)buf_head();		
	}
	inline int get_len()
	{
		PROTO_HEAD *head;
		head = get_head();
		return ENDION_FUNC_4(head->len);
	}
	inline uint8_t *get_data()
	{
		PROTO_HEAD *head;
		head = get_head();
		return (uint8_t *)&head->data[0];
	}
	inline uint16_t get_seq()
	{
		PROTO_HEAD *head;
		head = get_head();
		return head->seq;
	}
	inline int get_data_len()
	{
		return get_real_head_len(get_head()) - sizeof(PROTO_HEAD) - sizeof(EXTERN_DATA);
	}

	inline EXTERN_DATA *get_extern_data(PROTO_HEAD *head)
	{
		return (EXTERN_DATA *)(&head->data[get_real_head_len(head) - sizeof(PROTO_HEAD) - sizeof(EXTERN_DATA)]);
	}

	inline bool is_full_packet()
	{
		uint32_t len = buf_size();

		if (len < sizeof(PROTO_HEAD))  //没有够一个包头
			return (false);

		PROTO_HEAD *head = (PROTO_HEAD *)buf_head();
		uint32_t real_len = get_real_head_len(head);
		if (len >= real_len)
			return true;
		return false;
	}

	int get_one_buf();
	int remove_one_buf();
};


int fast_send_msg_base(conn_node_base* node, EXTERN_DATA *extern_data, uint16_t msg_id, size_t size, uint16_t seq);

template<typename PACK_STRUCT>
int fast_send_msg(conn_node_base* node, EXTERN_DATA *extern_data, uint16_t msg_id, size_t(*func)(const PACK_STRUCT *, uint8_t *), PACK_STRUCT& obj, uint16_t seq = 0)
{
	size_t size = (*func)(&obj, node->get_send_data());
	return fast_send_msg_base(node, extern_data, msg_id, size, seq);
}



#endif
