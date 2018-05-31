#ifndef SEND_MAIL_H
#define SEND_MAIL_H

#include <stdint.h>
#include <vector>
#include <map>
#include "conn_node.h"
#include "game_event.h"
#include "mail_db.pb-c.h"


int send_mail(conn_node_base *connecter, uint64_t player_id, uint32_t type,
	char *title, char *sender_name, char *content, std::vector<char *> *args,
	std::map<uint32_t, uint32_t> *attachs, uint32_t statis_id)
{
	EXTERN_DATA ext_data;
	ext_data.player_id = player_id;

	MailDBInfo resp;
	mail_dbinfo__init(&resp);

	MailAttach attach_data[MAX_MAIL_ATTACH_NUM];
	MailAttach* attach_data_point[MAX_MAIL_ATTACH_NUM];

	resp.type = type;
	resp.title = title;
	resp.sendername = sender_name;
	resp.content = content;
	if (args)
	{
		resp.args = &(*args)[0];
		resp.n_args = args->size();
	}

	if (attachs)
	{
		resp.n_attach = 0;
		resp.attach = attach_data_point;
		std::map<uint32_t, uint32_t>::iterator iter = attachs->begin();
		for (int j = 0; j < MAX_MAIL_ATTACH_NUM && iter != attachs->end(); ++j, ++iter)
		{
			attach_data_point[resp.n_attach] = &attach_data[resp.n_attach];
			mail_attach__init(&attach_data[resp.n_attach]);
			attach_data[resp.n_attach].id = iter->first;
			attach_data[resp.n_attach].num = iter->second;
			resp.n_attach++;
		}
		resp.statisid = statis_id;
	}

	PROTO_MAIL_INSERT* proto = (PROTO_MAIL_INSERT*)conn_node_base::get_send_buf(SERVER_PROTO_MAIL_INSERT, 0);
	proto->player_id = player_id;
	size_t pack_size = mail_dbinfo__pack(&resp, (uint8_t*)proto->data);
	if (pack_size != (size_t)-1)
	{
		proto->head.len = ENDION_FUNC_4(sizeof(PROTO_MAIL_INSERT) + pack_size);
		proto->data_size = pack_size;
		conn_node_base::add_extern_data(&proto->head, &ext_data);
		int ret = connecter->send_one_msg(&proto->head, 1);
		if (ret != (int)ENDION_FUNC_4(proto->head.len))
		{
			LOG_ERR("[%s:%d] send to mail_srv failed err[%d]", __FUNCTION__, __LINE__, errno);
			return -1;
		}
	}

	return 0;
}

#endif /* SEND_MAIL_H */
