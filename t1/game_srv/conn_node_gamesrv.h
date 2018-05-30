#ifndef CONN_NODE_GAMESRV_H
#define CONN_NODE_GAMESRV_H

#include "conn_node.h"
#include "game_event.h"

class conn_node_gamesrv: public conn_node_base
{
public:
	conn_node_gamesrv();
	virtual ~conn_node_gamesrv();

	virtual int recv_func(evutil_socket_t fd);

	void send_data_to_connsrv();

private:
	int dispatch_message();
};


#endif /* CONN_NODE_GAMESRV_H */

