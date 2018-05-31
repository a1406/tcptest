#include "listen_node.h"

listen_node_base::listen_node_base()
{
}

listen_node_base::~listen_node_base()
{
}

int listen_node_base::listen_pre_func()
{
	return (0);
}

int listen_node_base::listen_after_func(evutil_socket_t fd)
{
	return (0);
}

