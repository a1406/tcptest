#include "conn_node_gamesrv.h"
#include "time_helper.h"
#include "game_event.h"
#include "tea.h"
#include "flow_record.h"
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

conn_node_gamesrv::conn_node_gamesrv()
{
}

conn_node_gamesrv::~conn_node_gamesrv()
{
}

void conn_node_gamesrv::send_data_to_connsrv()
{
}

void conn_node_gamesrv::on_connected(conn_node_base *node)
{
}
void conn_node_gamesrv::on_disconnected(conn_node_base *node)
{
}

int conn_node_gamesrv::get_listen_fd()
{
	return fd;
}

int conn_node_gamesrv::recv_func(evutil_socket_t fd)
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
		}

		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __PRETTY_FUNCTION__, fd, errno);
			return (-1);
		} else if (ret > 0) {
			break;
		}

		ret = remove_one_buf();
	}
	return (0);
}


int conn_node_gamesrv::dispatch_message()
{
	return (0);
}


