#ifndef redis_client_h_
#define redis_client_h_

#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <strings.h>
#include <assert.h>
#include <vector>
#include <stdint.h>
#include <sstream>
#include <set>
#include "stl_relation.h"
#include "game_event.h"

class CAutoRedisReply
{
public:
	CAutoRedisReply(redisReply* r = NULL)
		: m_r(r) {}

	~CAutoRedisReply();
public:
	void set(redisReply* r);
// 	redisReply* get()
// 	{
// 		return m_r;
// 	}

private:
//	CAutoRedisReply operator=(const CAutoRedisReply&);
//	CAutoRedisReply (const CAutoRedisReply&);	

private:
	redisReply* m_r;
};

class CRedisClient
{
public:
	CRedisClient()
		: m_isConnect(false)
		, m_redisCtx(NULL){}

	virtual ~CRedisClient()
	{
		clear();
	}

public:
	int connect(const char* szIP, int port, int timeout=5000);
	///@ 带键值判断key是否存在
	///@ param[in]  table键值
	///@ result: >0-存在; 0-不存在, <0-错误
	int exist(const char* table);
	
	///@ 带键值获取db大小
	///@ param[in]	table健值
	///@ result: >=0: success,<0失败
	int size(const char* table);

	///@ 带键值判断key是否存在
	///@ param[in]  table键值
	///@ param[in]  playerid
	///@ result: >0-存在; 0-不存在, <0-错误
	int exist(const char* table, uint64_t playerid);

	///@ 带键值判断key是否存在
	///@ param[in]  table键值
	///@ param[in]  key
	///@ result: >0-存在; 0-不存在, <0-错误
	int exist(const char* table, const char* key);

	int hkeys(const char* table, std::vector<std::string>& s1);

	int hset_bin(const char* table, const char* key, const char* val, const int val_len);

	redisReply *hgetall_bin(const char* table, CAutoRedisReply &autoR);

		//调用者来给out_val分配内存
	int hget_bin(const char* table, const char* key, char* out_val, int *out_len);

		//使用者需要释放*out_val
	int hget_bin2(const char* table, const char* key, char **out_val, int *out_len);

	///@ 带键值插入操作
	///@ param[in]	table健值
	///@ param[in]	key值
	///@ param[in]	info用户信息
	///@ result: 0: success,<0失败
	int set(const char* table, const char* key, const char* val, uint32_t expire_ts=0);

	///@ 带键值的插入操作
	///@ param[in]	table健值
	///@ param[in]	playerid游戏中用户id
	///@ param[in]	info用户信息
	///@ result: 0: success,<0失败
	int set(const char* table, uint64_t playerid, const char* data, int len, uint32_t expire_ts=0);

	int set(const char* key, const char* val);

	///@ 修改名称
	///@ param[in]	table旧健值
	///@ param[in]	newtable新健值
	///@ result: 0: success,<0失败
	int rename(const char* table, const char* newtable);

	int mget_uint64(const char* table, int n, uint64_t *key, uint64_t *value);
	int mset_uint64(const char* table, int n, uint64_t *key, uint64_t *value);	
	 
	///@ 带健值读取
	int get(const char* table, std::vector<std::relation_three<uint64_t, char*, int> >& info);

	///@ 带健值读取
	std::string get(const char* table, const char* key);

	///@ 带健值读取
	std::string get(const char* key);

	///@ 带键值删除
	int hdel(const char* table, const std::vector<uint64_t>& info);

	///@ 带键值删除
	int hdel(const char* table, const char* key);
	int hdel(const char* table, uint64_t key);	

	///@ 带键值删除
	int del(const char* key);

	///@ 获取SortedSet的长度
	///@ param[in]	key
	///@ param[out]	out
	int zcard(const char* key, uint32_t& out);

	///@ 设置成员排行
	///@ param[in]	key
	///@ param[in]	member成员
	///@ param[in]	value成员值
	int zset(const char* key, uint64_t member, uint32_t value);

	///@ 设置成员排行
	///@ param[in]	key
	///@ param[in]	member成员
	///@ param[in]	value成员值
	int zset(const char* key, int n, uint64_t *member, uint32_t *value);

	///@ 获取排行
	///@ param[in]	key
	///@ param[in]	startNo开始排名,从0开始
	///@ param[in]	endNo结束排名
	///@ param[out]	out排名列表vector<pair< 用户名, 分数> >
	int zget(const char* key, int startNo, int endNo, std::vector<std::pair<uint64_t, uint32_t> >& out);


	///@ 获取成员的排名
	///@ param[in]	key
	///@ param[in]	member成员
	///@ param[out]	out
	int zget_rank(const char* key, uint64_t member, uint32_t& out);

	///@ 获取成员的积分
	///@ param[in]	key
	///@ param[in]	member成员
	///@ param[out]	out
	int zget_score(const char* key, uint64_t member, uint32_t& out);

	/// 删除某人的排行
	int zdel(const char* key, const std::vector<uint64_t>& dellist);

	///@ 删除指定范围的成员
	///@ param[in]	key
	///@ param[in]	startNo开始排名,从0开始
	///@ param[in]	endNo结束排名
	int zdel_rank(const char* key, int startNo, int endNo);


	/// 链表操作
	/// 插入链表尾部
	int list_insert(const char* key, const char* value);

	/// 链表操作
	/// 修改链表的值
	int list_set(const char* key, uint32_t idx,  const char* value);

	/// 获取指定元素
	int list_get(const char* key, int startNo, int endNo, std::vector<std::string>& out);

	/// 获取指定元素
	int get_score(const char* key, int startScore, int endScore, std::vector<uint64_t>& out);

	/// 从头部删除一个元素
	int list_header_pop(const char* key);

	int list_size(const char* key);

	int lock(const char* key, const char* val);

	int unlock(const char* key);

	int get_all_val(const char* table, std::vector<std::string>& s1);

	int incr(const char* key);

	//获取hash中的所有field,用于field都是数字的
	int hkeys(const char* table, std::set<uint64_t>& s1);

	//////////////////////////////////////////////////////////////////
	void clear();
private:
	bool			m_isConnect;
	redisContext*	m_redisCtx;
	std::string		m_szIp;
	int				m_nPort;
	uint32_t		m_timeout;
	int connect();
};
#endif//redis_client_h_

