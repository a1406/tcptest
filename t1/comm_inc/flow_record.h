//统计连接服流量
#ifndef _FLOW_RECORD_H__
#define _FLOW_RECORD_H__

#ifdef FLOW_MONITOR
//#include "flow_record.cpp"
#include "conn_node.h"

extern std::map<uint32_t, std::pair<uint32_t,uint32_t> > record_client_request_msg;//记录客户端请求到连接服的消息数量和大小，消息id对应数量和大小
extern std::map<uint32_t, std::pair<uint32_t,uint32_t> > record_client_answer_msg;//记录连接服回复客户端的消息数量和大小，消息id对应数量和大小

extern std::map<uint32_t, std::pair<uint32_t,uint32_t> > record_other_server_request_msg;//记录其他服务器到连接服的消息数量和大小，消息id对应数量和大小
extern std::map<uint32_t, std::pair<uint32_t,uint32_t> > record_other_server_answer_msg;//记录连接服发给其他服务器的消息数量和大小，消息id对应数量和大小

extern std::map<uint32_t, std::pair<uint32_t,uint64_t> > gamesrv_msg_dealwith_time;//记录game服每条消息处理时间
//初始化
void init_flow_reord_map();
//增加一条客户端到连接服的请求消息记录
void add_one_client_request(PROTO_HEAD *head);
//增加一条连接服发给客户端的消息
void add_one_client_answer(PROTO_HEAD *head);
//增加一条从连接服发送到别的服务器的消息记录
void add_on_other_server_answer_msg(PROTO_HEAD *head);
//增加一条别的服务器到连接服的消息记录
void add_one_other_server_request_msg(PROTO_HEAD *head);
//连接流量数据库
int connect_flow_record_mysql();
//将流量监控数据存入数据库
int save_flow_record_data();
//插入数据到数据库
int visit_flow_record_mysql(char* s);
//连接服断开时调用存流量数据
int save_flow_record_to_mysql();
//清流量数据库
int delete_flow_record_mysql();
//增加一条game服消息处理时间
void add_one_game_srv_msg_time(uint32_t cmd, uint64_t time);
//将game服消息处理时间数据存入数据库
int save_gamesrv_msg_time_data();
//清game服消息处理时间数据库
int delete_gamesrv_msg_time_mysql();
//gamesrv服断开时调用保消息处理时间
int save_gamesrv_msg_time_to_mysql();
#endif
#endif
