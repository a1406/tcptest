#ifndef _LISTEN_NODE_H__
#define _LISTEN_NODE_H__
#include "conn_node.h"

class listen_node_base
{
public:
	listen_node_base();
	~listen_node_base();
	virtual int listen_pre_func();
	virtual int listen_after_func(evutil_socket_t fd);
	virtual conn_node_base * get_conn_node(evutil_socket_t fd) = 0;
	
	struct event event_accept;	
};


#endif
