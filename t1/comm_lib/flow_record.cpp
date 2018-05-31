//统计连接服流量
#ifdef FLOW_MONITOR
#include <stdio.h>
#include "flow_record.h"
#include "mysql_module.h"
#include "game_event.h"
#include "oper_config.h"
#include "error_code.h"
std::map<uint32_t, std::pair<uint32_t,uint32_t> > record_client_request_msg;//记录客户端请求到连接服的消息数量和大小，消息id对应数量和大小
std::map<uint32_t, std::pair<uint32_t,uint32_t> > record_client_answer_msg;//记录连接服回复客户端的消息数量和大小，消息id对应数量和大小

std::map<uint32_t, std::pair<uint32_t,uint32_t> > record_other_server_request_msg;//记录其他服务器到连接服的消息数量和大小，消息id对应数量和大小
std::map<uint32_t, std::pair<uint32_t,uint32_t> > record_other_server_answer_msg;//记录连接服发给其他服务器的消息数量和大小，消息id对应数量和大小


std::map<uint32_t, std::pair<uint32_t,uint64_t> > gamesrv_msg_dealwith_time;//记录game服每条消息处理时间

//增加一条game服消息处理时间
void add_one_game_srv_msg_time(uint32_t cmd, uint64_t time)
{
	if(gamesrv_msg_dealwith_time.find(cmd) != gamesrv_msg_dealwith_time.end())
	{
		std::map<uint32_t,std::pair<uint32_t,uint64_t> >::iterator msg_time = gamesrv_msg_dealwith_time.find(cmd);
		msg_time->second.first += 1;
		msg_time->second.second += time;
	}
	else
	{
		gamesrv_msg_dealwith_time.insert(std::make_pair(cmd,std::make_pair(1,time)));
	}
}

//增加一条客户端到连接服的请求消息记录
void add_one_client_request(PROTO_HEAD *head)
{
	if (head == NULL)
	{
		return;
	}
	
	uint32_t cmd = ENDION_FUNC_2(head->msg_id);
	if (record_client_request_msg.find(cmd) != record_client_request_msg.end())
	{
		std::map<uint32_t,std::pair<uint32_t,uint32_t> >::iterator client_msg = record_client_request_msg.find(cmd);
		client_msg->second.first += 1;
		client_msg->second.second += head->len;
	}
	else
	{
		record_client_request_msg.insert(std::make_pair(cmd,std::make_pair(1,head->len)));
	}
}


//增加一条连接服发给客户端的消息记录
void add_one_client_answer(PROTO_HEAD *head)
{	
	if (head == NULL)
	{
		return;
	}

	uint32_t cmd = ENDION_FUNC_2(head->msg_id);

	if (record_client_answer_msg.find(cmd) != record_client_answer_msg.end())
	{
		std::map<uint32_t,std::pair<uint32_t,uint32_t> >::iterator client_msg = record_client_answer_msg.find(cmd);
		client_msg->second.first += 1;
		client_msg->second.second += head->len;
	}
	else
	{
		record_client_answer_msg.insert(std::make_pair(cmd,std::make_pair(1,head->len)));
	}
	
}

//增加一条从连接服发送到别的服务器的消息记录
void add_on_other_server_answer_msg(PROTO_HEAD *head)
{
	if (head == NULL)
	{
		return;
	}
	uint32_t cmd = ENDION_FUNC_2(head->msg_id);
	if (record_other_server_answer_msg.find(cmd) != record_other_server_answer_msg.end())
	{
		std::map<uint32_t,std::pair<uint32_t,uint32_t> >::iterator client_msg = record_other_server_answer_msg.find(cmd);
		client_msg->second.first += 1;
		client_msg->second.second += head->len;
	}
	else
	{
		record_other_server_answer_msg.insert(std::make_pair(cmd,std::make_pair(1,head->len)));
	}
}

//增加一条别的服务器到连接服的消息记录
void add_one_other_server_request_msg(PROTO_HEAD *head)
{
	if (head == NULL)
	{
		return;
	}
	uint32_t cmd = ENDION_FUNC_2(head->msg_id);
	if (record_other_server_request_msg.find(cmd) != record_other_server_request_msg.end())
	{
		std::map<uint32_t,std::pair<uint32_t,uint32_t> >::iterator client_msg = record_other_server_request_msg.find(cmd);
		client_msg->second.first += 1;
		client_msg->second.second += head->len;
	}
	else
	{
		record_other_server_request_msg.insert(std::make_pair(cmd,std::make_pair(1,head->len)));
	}
}

//初始化
void init_flow_reord_map()
{
	record_client_request_msg.clear();
	record_client_answer_msg.clear();
	record_other_server_answer_msg.clear();
	record_other_server_request_msg.clear();
}

//连接流量数据库
int connect_flow_record_mysql()
{
	int ret = 0;
	FILE* file = NULL;
	char* line;
	std::string szMysqlIp;
	std::string szMysqlDbName;
	std::string szMysqlDbUser;
	std::string szMysqlDbPwd;
	int nMysqlPort = 0;

	file = fopen("../server_info.ini","r");
	if (!file)
	{
		LOG_ERR("[%s:%d] open server_info.ini failed[%d]", __FUNCTION__, __LINE__, errno);
		return -1;
	}

	line = get_first_key(file, (char*)"mysql_host");
	if (!line)
	{
		LOG_DEBUG("[%s:%d] get mysql_host failed", __FUNCTION__, __LINE__);
		return -1;
	}
	szMysqlIp = (get_value(line));

	line = get_first_key(file, (char*)"mysql_port");
	if (!line)
	{
		LOG_DEBUG("[%s:%d] get mysql_port failed", __FUNCTION__, __LINE__);
		return -1;
	}
	nMysqlPort = atoi(get_value(line));

	line = get_first_key(file, (char*)"mysql_db_flow_record");
	if (!line)
	{
		LOG_DEBUG("[%s:%d] get mysql_db_flow_record failed", __FUNCTION__, __LINE__);
		return -1;
	}
	szMysqlDbName = (get_value(line));
	
	line = get_first_key(file, (char*)"mysql_db_user");
	if (!line)
	{
		LOG_DEBUG("[%s:%d] get mysql_db_user failed", __FUNCTION__, __LINE__);
		return -1;
	}
	szMysqlDbUser = (get_value(line));

	line = get_first_key(file, (char*)"mysql_db_pwd");
	if (!line)
	{
		LOG_DEBUG("[%s:%d] get mysql_db_pwd failed", __FUNCTION__, __LINE__);
		return -1;
	}
	szMysqlDbPwd = (get_value(line));

	ret = init_db(const_cast<char*>(szMysqlIp.c_str()), nMysqlPort,  const_cast<char*>(szMysqlDbName.c_str())
		, const_cast<char*>(szMysqlDbUser.c_str()), const_cast<char*>(szMysqlDbPwd.c_str()));
	if (0 != ret) {
		LOG_ERR("[%s : %d]: init db failed, ip: %s, port: %u, abname: %s, user name: %s, pwd: %s",
			__FUNCTION__, __LINE__, szMysqlIp.c_str(), nMysqlPort, szMysqlDbName.c_str(), szMysqlDbUser.c_str(), szMysqlDbPwd.c_str());
		return -1;
	}

	if (file)
		fclose(file);

	return 0;
}

//请求存数据到数据库
int visit_flow_record_mysql(char* s)
{
	uint64_t effect = 0;
	query(s, 1, &effect);
	if (effect == 0)
	{
		LOG_ERR("[%s:%d] query failed, sql: %s, error:%s", __FUNCTION__, __LINE__, s, mysql_error());
		return ERROR_ID_MYSQL_QUERY;
	}


	return 0;
}


//将流量监控数据存入数据库
int save_flow_record_data()
{
	
	char sql[1024];
	uint32_t msg_sum_num = 0;
	uint32_t msg_sum_size = 0;
	//存储客户端到连接服数据
	if (record_client_request_msg.size() != 0)
	{
		for (std::map<uint32_t,std::pair<uint32_t,uint32_t> >::iterator itr = record_client_request_msg.begin(); itr != record_client_request_msg.end(); itr++)
		{
			sprintf(sql, "insert into client_to_conn_srv set `msg_id` = %u, `msg_num` = %u, `msg_size` = %u", itr->first, itr->second.first, itr->second.second);
			if (visit_flow_record_mysql(sql) != 0)
			{
				LOG_ERR("[%s:%d] 存储客户端到连接服流量数据失败", __FUNCTION__, __LINE__);
				return -1;
			}
			msg_sum_num += itr->second.first;
			msg_sum_size += itr->second.second;
		}

		//最后统计所有消息的总量
		sprintf(sql, "insert into client_to_conn_srv set `msg_sum_num` = %u, `msg_sum_size` = %u",  msg_sum_num, msg_sum_size);
		if (visit_flow_record_mysql(sql) != 0)
		{
			LOG_ERR("[%s:%d] 存储客户端到连接服总流量数据失败", __FUNCTION__, __LINE__);
			return -1;
		}
	}

	msg_sum_num = 0;
	msg_sum_size = 0;

	//存储连接服到客户端数据
	if (record_client_answer_msg.size() != 0)
	{
		for (std::map<uint32_t,std::pair<uint32_t,uint32_t> >::iterator ite = record_client_answer_msg.begin(); ite != record_client_answer_msg.end(); ite++)
		{
			sprintf(sql, "insert into conn_srv_to_client set `msg_id` = %u, `msg_num` = %u, `msg_size` = %u", ite->first, ite->second.first, ite->second.second);
			if (visit_flow_record_mysql(sql) != 0)
			{
				LOG_ERR("[%s:%d] 存储连接服到客户端流量数据失败", __FUNCTION__, __LINE__);
				return -1;
			}
			msg_sum_num += ite->second.first;
			msg_sum_size += ite->second.second;
		}

		//最后统计所有消息的总量
		sprintf(sql, "insert into conn_srv_to_client set `msg_sum_num` = %u, `msg_sum_size` = %u",  msg_sum_num, msg_sum_size);
		if (visit_flow_record_mysql(sql) != 0)
		{
			LOG_ERR("[%s:%d] 存储连接服到客户端总流量数据失败", __FUNCTION__, __LINE__);
			return -1;
		}
	}

	msg_sum_num = 0;
	msg_sum_size = 0;
	//存储其他服到连接服数据
	if (record_other_server_request_msg.size() != 0)
	{
		for (std::map<uint32_t,std::pair<uint32_t,uint32_t> >::iterator it = record_other_server_request_msg.begin(); it != record_other_server_request_msg.end(); it++)
		{
			sprintf(sql, "insert into other_srv_to_conn_srv set `msg_id` = %u, `msg_num` = %u, `msg_size` = %u", it->first, it->second.first, it->second.second);
			if (visit_flow_record_mysql(sql) != 0)
			{
				LOG_ERR("[%s:%d] 存储其他服到连接服流量数据失败", __FUNCTION__, __LINE__);
				return -1;
			}
			msg_sum_num += it->second.first;
			msg_sum_size += it->second.second;
		}
		//最后统计所有消息的总量
		sprintf(sql, "insert into other_srv_to_conn_srv set `msg_sum_num` = %u, `msg_sum_size` = %u",  msg_sum_num, msg_sum_size);
		if (visit_flow_record_mysql(sql) != 0)
		{
			LOG_ERR("[%s:%d] 存储其他服到连接服总流量数据失败", __FUNCTION__, __LINE__);
			return -1;
		}
	}

	msg_sum_num = 0;
	msg_sum_size = 0;
	//存储连接服到其他服数据
	if (record_other_server_answer_msg.size() != 0)
	{
		for (std::map<uint32_t,std::pair<uint32_t,uint32_t> >::iterator ita = record_other_server_answer_msg.begin(); ita != record_other_server_answer_msg.end(); ita++)
		{
			sprintf(sql, "insert into conn_srv_to_other_srv set `msg_id` = %u, `msg_num` = %u, `msg_size` = %u", ita->first, ita->second.first, ita->second.second);
			if (visit_flow_record_mysql(sql) != 0)
			{
				LOG_ERR("[%s:%d] 存储连接服到其他服流量数据失败", __FUNCTION__, __LINE__);
				return -1;
			}
			msg_sum_num += ita->second.first;
			msg_sum_size += ita->second.second;
		}
		//最后统计所有消息的总量
		sprintf(sql, "insert into conn_srv_to_other_srv set `msg_sum_num` = %u, `msg_sum_size` = %u",  msg_sum_num, msg_sum_size);
		if (visit_flow_record_mysql(sql) != 0)
		{
			LOG_ERR("[%s:%d] 存储连接服到其他服总流量数据失败", __FUNCTION__, __LINE__);
			return -1;
		}
	}
	
	return 0;
}

//清流量数据库
int delete_flow_record_mysql()
{
	char sql[1024];
	sprintf(sql, "delete from client_to_conn_srv");
	if (clean_mysql_table(sql) != 0)
	{
		LOG_ERR("[%s:%d] 清除客户端到连接服流量表失败", __FUNCTION__, __LINE__);
		return -1;
	}

	sprintf(sql, "delete from conn_srv_to_client");
	if (clean_mysql_table(sql) != 0)
	{
		LOG_ERR("[%s:%d] 清除连接服到客户端流量表失败", __FUNCTION__, __LINE__);
		return -1;
	}


	sprintf(sql, "delete from conn_srv_to_other_srv");
	if (clean_mysql_table(sql) != 0)
	{
		LOG_ERR("[%s:%d] 清除连接服到其他服流量表失败", __FUNCTION__, __LINE__);
		return -1;
	}

	sprintf(sql, "delete from other_srv_to_conn_srv");
	if (clean_mysql_table(sql) != 0)
	{
		LOG_ERR("[%s:%d] 清除其他服到连接服流量表失败", __FUNCTION__, __LINE__);
		return -1;
	}
	return 0;
}

//连接服断开时调用存流量数据
int save_flow_record_to_mysql()
{
	if (connect_flow_record_mysql() != 0)
	{
		LOG_ERR("[%s:%d] connect flow record mysql", __FUNCTION__, __LINE__);
		return -1;
	}
	if (delete_flow_record_mysql() !=0 )
	{
		LOG_ERR("[%s:%d] delete flow record mysq failed", __FUNCTION__, __LINE__);
		return -1;
	}
	
	if (save_flow_record_data() != 0)
	{
		LOG_ERR("[%s:%d] save flow record data failed", __FUNCTION__, __LINE__);
		return -1;
	}
	return 0;
}

//将game服消息处理时间数据存入数据库
int save_gamesrv_msg_time_data()
{
	
	char sql[1024];
	if (gamesrv_msg_dealwith_time.size() != 0)
	{
		for (std::map<uint32_t,std::pair<uint32_t,uint64_t> >::iterator it = gamesrv_msg_dealwith_time.begin(); it != gamesrv_msg_dealwith_time.end(); it++)
		{
			sprintf(sql, "insert into game_srv_msg_time set `msg_id` = %u, `msg_num` = %u, `msg_time` = %lu", it->first, it->second.first, it->second.second);
			if (visit_flow_record_mysql(sql) != 0)
			{
				LOG_ERR("[%s:%d] 存储game服消息处理时间数据失败", __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}
	return 0;
}

//清game服消息处理时间数据库
int delete_gamesrv_msg_time_mysql()
{
	char sql[1024];
	sprintf(sql, "delete from game_srv_msg_time");

	if (clean_mysql_table(sql) != 0)
	{
		LOG_ERR("[%s:%d] 清game服消息处理时间数据表失败", __FUNCTION__, __LINE__);
		return -1;
	}
	return 0;
}

//gamesrv服断开时调用存储消息处理时间
int save_gamesrv_msg_time_to_mysql()
{

	if (connect_flow_record_mysql() != 0)
	{
		LOG_ERR("[%s:%d] connect flow record mysql", __FUNCTION__, __LINE__);
		return -1;
	}

	if(delete_gamesrv_msg_time_mysql() != 0)
	{
		LOG_ERR("[%s:%d] delete gamesrv msg time mysql failed", __FUNCTION__, __LINE__);
		return -1;
	}

	
	if(save_gamesrv_msg_time_data() != 0)
	{
		LOG_ERR("[%s:%d] save gamesrv msg time data failed", __FUNCTION__, __LINE__);
		return -1;
	}

	return 0;

}
#endif
