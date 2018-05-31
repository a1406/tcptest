#include "redis_client.h"

CAutoRedisReply::~CAutoRedisReply()
{
	if (m_r)
	{
		freeReplyObject(m_r);
	}
}

void CAutoRedisReply::set(redisReply* r)
{
	if (m_r)
	{
		freeReplyObject(m_r);
	}

	m_r = r;
}

int CRedisClient::connect(const char* szIP, int port, int timeout)
{
	if (!szIP || port<=0)
		return -1;

	m_szIp = std::string(szIP);
	m_nPort = port;
	m_timeout = timeout;

	return connect();
}

	///@ 带键值判断key是否存在
	///@ param[in]  table键值
	///@ result: >0-存在; 0-不存在, <0-错误
int CRedisClient::exist(const char* table)
{
	if (!table)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[4096] = {0};
	snprintf(buffer, sizeof(buffer), "EXISTS %s", table);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

		//	if (!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK") == 0))
	if (r->type!=REDIS_REPLY_INTEGER)
		return -3;

	return r->integer;
}

	///@ 带键值获取db大小
	///@ param[in]	table健值
	///@ result: >=0: success,<0失败
int CRedisClient::size(const char* table)
{
	if (!table)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[4096] = {0};
	snprintf(buffer, sizeof(buffer), "HLEN %s", table);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

		//	if (!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK") == 0))
	if (r->type!=REDIS_REPLY_INTEGER)
		return -3;

	return r->integer;
}

	///@ 带键值判断key是否存在
	///@ param[in]  table键值
	///@ param[in]  playerid
	///@ result: >0-存在; 0-不存在, <0-错误
int CRedisClient::exist(const char* table, uint64_t playerid)
{
	if (!table)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[4096] = {0};
	snprintf(buffer, sizeof(buffer), "HEXISTS %s %lu", table, playerid);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

		//	if (!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK") == 0))
	if (r->type!=REDIS_REPLY_INTEGER)
		return -3;

	return r->integer;
}

	///@ 带键值判断key是否存在
	///@ param[in]  table键值
	///@ param[in]  key
	///@ result: >0-存在; 0-不存在, <0-错误
int CRedisClient::exist(const char* table, const char* key)
{
	if (!table || !key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[4096] = {0};
	snprintf(buffer, sizeof(buffer), "HEXISTS %s %s", table, key);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

		//	if (!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK") == 0))
	if (r->type!=REDIS_REPLY_INTEGER)
		return -3;

	return r->integer;
}

int CRedisClient::hkeys(const char* table, std::vector<std::string>& s1)
{
	if (!table)
		return 0;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "HKEYS %s", table);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -1;
	}

	autoR.set(r);
	if(r->type != REDIS_REPLY_ARRAY )
		return -1;

	for (size_t i = 0; i < r->elements; ++i) {
		redisReply* childReply = r->element[i];
		if (childReply->type == REDIS_REPLY_STRING) {
			std::string szVal;
			szVal.assign(childReply->str, childReply->len);
			s1.push_back(szVal);
		}
	}

	return 0;
}

int CRedisClient::hset_bin(const char* table, const char* key, const char* val, const int val_len)
{
	if (!table || !key || !val)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}
	const char *cmd[10];
	size_t  len[10];
	cmd[0] = "HSET";
	len[0] = strlen(cmd[0]);
	cmd[1] = table;
	len[1] = strlen(cmd[1]);
	cmd[2] = key;
	len[2] = strlen(cmd[2]);
	cmd[3] = val;
	len[3] = val_len;
	redisReply *r = (redisReply *)redisCommandArgv(m_redisCtx, 4, &cmd[0], &len[0]);
	if (!r)
	{
		return (-1);
	}

	if (r->type!=REDIS_REPLY_INTEGER)
	{
		freeReplyObject( r );
		return -2;
	}
	ret = r->integer;
	freeReplyObject( r );
	return ret;
}

redisReply *CRedisClient::hgetall_bin(const char* table, CAutoRedisReply &autoR)
{
	if (!table)
		return NULL;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return NULL;
	}
	const char *cmd[10];
	size_t  len[10];

	cmd[0] = "HGETALL";
	len[0] = strlen(cmd[0]);
	cmd[1] = table;
	len[1] = strlen(cmd[1]);
	redisReply *r = (redisReply *)redisCommandArgv(m_redisCtx, 2, &cmd[0], &len[0]);
	autoR.set(r);	
	return r;
/*
		if (!r)
		{
		return (-1);
		}
		if (r->type!=REDIS_REPLY_ARRAY)
		{
		freeReplyObject( r );
		return -2;
		}
		if (*out_len < r->len)
		{
		freeReplyObject( r );
		return -3;
		}

		*out_len = r->len;
		memcpy(out_val, r->str, r->len);
		freeReplyObject( r );
		return (0);
*/
}

	//调用者来给out_val分配内存
int CRedisClient::hget_bin(const char* table, const char* key, char* out_val, int *out_len)
{
	if (!table || !key || !out_val || !out_len)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}
	const char *cmd[10];
	size_t  len[10];

	cmd[0] = "HGET";
	len[0] = strlen(cmd[0]);
	cmd[1] = table;
	len[1] = strlen(cmd[1]);
	cmd[2] = key;
	len[2] = strlen(cmd[2]);
	redisReply *r = (redisReply *)redisCommandArgv(m_redisCtx, 3, &cmd[0], &len[0]);
	if (!r)
	{
		return (-1);
	}
	if (r->type!=REDIS_REPLY_STRING)
	{
		freeReplyObject( r );
		return -2;
	}
	if (*out_len < r->len)
	{
		freeReplyObject( r );
		return -3;
	}

	*out_len = r->len;
	memcpy(out_val, r->str, r->len);
	freeReplyObject( r );
	return (0);
}

	//使用者需要释放*out_val
int CRedisClient::hget_bin2(const char* table, const char* key, char **out_val, int *out_len)
{
	if (!table || !key || !out_val || !out_len)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}
	const char *cmd[10];
	size_t  len[10];

	cmd[0] = "HGET";
	len[0] = strlen(cmd[0]);
	cmd[1] = table;
	len[1] = strlen(cmd[1]);
	cmd[2] = key;
	len[2] = strlen(cmd[2]);
	redisReply *r = (redisReply *)redisCommandArgv(m_redisCtx, 3, &cmd[0], &len[0]);
	if (!r)
	{
		return (-1);
	}
	if (r->type!=REDIS_REPLY_STRING)
	{
		freeReplyObject( r );
		return -2;
	}

	*out_val = (char *)malloc(r->len);
	if (!(*out_len))
	{
		freeReplyObject( r );
		return -3;
	}

	*out_len = r->len;
	memcpy(*out_val, r->str, r->len);
	freeReplyObject( r );
	return (0);
}


	///@ 带键值插入操作
	///@ param[in]	table健值
	///@ param[in]	key值
	///@ param[in]	info用户信息
	///@ result: 0: success,<0失败
int CRedisClient::set(const char* table, const char* key, const char* val, uint32_t expire_ts)
{
	if (!table || !key || !val)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[4096];// = { 0 };
	snprintf(buffer, sizeof(buffer), "hset %s %s %s", table, key, val);

	{
		CAutoRedisReply autoR;
		redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
		if (NULL == r) {
			LOG_ERR("[%s : %d]: call redisCommand[%s] error, redis break connection,m_redisCtx: %p",
				__FUNCTION__, __LINE__, buffer, m_redisCtx);
			m_isConnect = false;
			return -2;
		}

		autoR.set(r);

		if (r->type!=REDIS_REPLY_INTEGER)
		{
			LOG_ERR("[%s : %d]: call redisCommand[%s] error[%s]", __FUNCTION__, __LINE__, buffer, r->str);
			return -3;
		}

	}

	{
		if (expire_ts > 0) {
			snprintf(buffer, sizeof(buffer), "expire %s %u", table, expire_ts);
			CAutoRedisReply autoR;
			redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
			if (NULL == r) {
				LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
				m_isConnect = false;
				return -2;
			}

			autoR.set(r);

			if (r->type!=REDIS_REPLY_INTEGER)
				return -3;
		}
	}

	return 0;
}

	///@ 带键值的插入操作
	///@ param[in]	table健值
	///@ param[in]	playerid游戏中用户id
	///@ param[in]	info用户信息
	///@ result: 0: success,<0失败
int CRedisClient::set(const char* table, uint64_t playerid, const char* data, int len, uint32_t expire_ts)
{
	if (!data || !table ||len<=0)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[4096] = {0};

	{
		snprintf(buffer, sizeof(buffer), "hset %s %lu  %s", table, playerid, data);

		CAutoRedisReply autoR;
		redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
		if (NULL == r) {
			LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
			m_isConnect = false;
			return -2;
		}

		autoR.set(r);

		if (r->type!=REDIS_REPLY_INTEGER)
			return -3;
	}

	{
		if (expire_ts > 0) {
			snprintf(buffer, sizeof(buffer), "expire %s %u", table, expire_ts);
			CAutoRedisReply autoR;
			redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
			if (NULL == r) {
				LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
				m_isConnect = false;
				return -2;
			}

			autoR.set(r);

			if (r->type!=REDIS_REPLY_INTEGER)
				return -3;
		}
	}

	return 0;
}

int CRedisClient::set(const char* key, const char* val)
{
	if (!key || !val)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << " set " << std::string(key) << " " << std::string(val);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if (r->type!=REDIS_REPLY_STATUS || !r->str || 0!=strcasecmp(r->str, "OK"))
		return -3;

	return 0;
}

	///@ 修改名称
	///@ param[in]	table旧健值
	///@ param[in]	newtable新健值
	///@ result: 0: success,<0失败
int CRedisClient::rename(const char* table, const char* newtable)
{
	if (!table || !newtable)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("[%s : %d]: connect redis failed", __FUNCTION__, __LINE__);
		return -1;
	}

	char cmd[64];
	snprintf(cmd, sizeof(cmd), "rename %s %s", table, newtable);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, cmd);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if (r->type!=REDIS_REPLY_STATUS || !r->str || 0!=strcasecmp(r->str, "OK"))
		return -3;

	return 0;
}

int CRedisClient::mset_uint64(const char* table, int n, uint64_t *key, uint64_t *value)
{
	if (n<=0 || !table || !key || !value)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << " hmset " << std::string(table) ;

	for (int i=0; i<n; ++i)
	{
		os << " " << key[i] << " " << value[i];
	}

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if (r->type!=REDIS_REPLY_STATUS || !r->str || 0!=strcasecmp(r->str, "OK"))	
		return -3;

	return 0;
}

int CRedisClient::mget_uint64(const char* table, int n, uint64_t *key, uint64_t *value)
{
	if (n<=0 || !table || !key || !value)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << " hmget " << std::string(table) ;

	for (int i=0; i<n; ++i)
	{
		os << " " << key[i];
	}

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if (r->type != REDIS_REPLY_ARRAY)
		return -3;

	for (size_t i = 0; i < r->elements; ++i) {
		std::string name;
		redisReply* childReply = r->element[i];
		if (childReply->type == REDIS_REPLY_STRING)
		{
			value[i] = strtoul(childReply->str, 0, 0);
		}
		else
		{
			value[i] = 0;
		}
	}

	return 0;
}

	///@ 带健值读取
int CRedisClient::get(const char* table, std::vector<std::relation_three<uint64_t, char*, int> >& info)
{
	if (info.size()<=0 || !table)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << " hmget " << std::string(table) ;

	for (size_t i=0; i<info.size(); ++i)
	{
		os << " " << info[i].first ;
	}

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if (r->type != REDIS_REPLY_ARRAY)
		return -3;

	for (size_t i = 0; i < r->elements; ++i) {
		std::string name;
		redisReply* childReply = r->element[i];
		if (childReply->type == REDIS_REPLY_STRING)
		{
			char* p = (char*)malloc(childReply->len+1);
			if (NULL == p) {
				for (size_t i=0; i<info.size(); ++i) {
					if (info[i].second) {
						free(info[i].second);
						info[i].second = NULL;
					}
				}
				return -4;
			}

			memcpy(p, childReply->str, childReply->len);
			info[i].second = p;
			info[i].three = childReply->len;
			p[childReply->len] = '\0';
		}
	}

	return 0;
}

	///@ 带健值读取
std::string CRedisClient::get(const char* table, const char* key)
{
	if (!key || !table)
		return "";

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return "";
	}

	std::ostringstream os;
	os << " hget " << std::string(table) << " " << std::string(key);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return "";
	}

	autoR.set(r);
	if (r->type != REDIS_REPLY_STRING)
		return "";

	return std::string(r->str);
}

	///@ 带健值读取
std::string CRedisClient::get(const char* key)
{
	if (!key)
		return "";

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return "";
	}

	std::ostringstream os;
	os << " get " << std::string(key);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return "";
	}

	autoR.set(r);
	if (r->type != REDIS_REPLY_STRING)
		return "";

	return std::string(r->str);
}

	///@ 带键值删除
int CRedisClient::hdel(const char* table, const std::vector<uint64_t>& info)
{
	if (!table || info.size()<=0)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << "hdel " << std::string(table);

	for (size_t i=0; i<info.size(); ++i)
	{
		os << " " << info[i];
	}

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if( r->type == REDIS_REPLY_INTEGER )
		return r->integer==1 ? 0 : -1;

	return -1;
}

int CRedisClient::hdel(const char* table, uint64_t key)
{
	if (!table || !key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << "hdel " << std::string(table) << " " << key;

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if( r->type == REDIS_REPLY_INTEGER )
		return r->integer==1 ? 0 : -1;

	return -1;
}

	///@ 带键值删除
int CRedisClient::hdel(const char* table, const char* key)
{
	if (!table || !key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << "hdel " << std::string(table) << " " << std::string(key);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if( r->type == REDIS_REPLY_INTEGER )
		return r->integer==1 ? 0 : -1;

	return -1;
}

	///@ 带键值删除
int CRedisClient::del(const char* key)
{
	if (!key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << "del " << std::string(key);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if( r->type == REDIS_REPLY_INTEGER )
		return r->integer==1 ? 0 : -1;

	return -1;
}

	///@ 获取SortedSet的长度
	///@ param[in]	key
	///@ param[out]	out
int CRedisClient::zcard(const char* key, uint32_t& out)
{
	char command[1024];
	if (!key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	snprintf(command, sizeof(command), "zcard %s", key); /// get number of members
	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, command);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if( r->type != REDIS_REPLY_INTEGER )
		return -3;

	out = (uint32_t)(r->integer);

	return 0;
}

int CRedisClient::zset(const char* key, int n, uint64_t *member, uint32_t *value)
{
//	char command[1024];
	if (!key || !member || !value)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << "zadd " << key;
	for (int i = 0; i < n; ++i)
	{
		uint64_t tmp = value[i];
		tmp <<= 32;

		uint32_t tm = (0xffffffff-(uint32_t)time(NULL));
		tmp |= (uint64_t)tm;
		os << tmp << " " << member[i];
	}


//	snprintf(command, sizeof(command), "zadd %s %lu %lu", key, tmp, member);
	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

		//if (!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK") == 0))
	if (r->type != REDIS_REPLY_INTEGER)
		return -3;

	return 0;
}

	///@ 设置成员排行
	///@ param[in]	key
	///@ param[in]	member成员
	///@ param[in]	value成员值
int CRedisClient::zset(const char* key, uint64_t member, uint32_t value)
{
	char command[1024];
	if (!key || !member)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	uint64_t tmp = value;
	tmp <<= 32;

	uint32_t tm = (0xffffffff-(uint32_t)time(NULL));
	tmp |= (uint64_t)tm;

	snprintf(command, sizeof(command), "zadd %s %lu %lu", key, tmp, member);
	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, command);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

		//if (!(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK") == 0))
	if (r->type != REDIS_REPLY_INTEGER)
		return -3;

	return 0;
}

	///@ 获取排行
	///@ param[in]	key
	///@ param[in]	startNo开始排名,从0开始
	///@ param[in]	endNo结束排名
	///@ param[out]	out排名列表vector<pair< 用户名, 分数> >
int CRedisClient::zget(const char* key, int startNo, int endNo, std::vector<std::pair<uint64_t, uint32_t> >& out)
{
	char command[1024];
	if (!key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	snprintf(command, sizeof(command), "ZREVRANGE %s %d %d WITHSCORES", key, startNo, endNo);
	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, command);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if( r->type != REDIS_REPLY_ARRAY )
		return -3;

	out.clear();
	for (size_t i = 0; i < r->elements/2; ++i) {
		uint64_t playerid;
		uint64_t score=0;
		redisReply* childReply = r->element[i*2];
		if (childReply->type == REDIS_REPLY_STRING)
			playerid = strtoull(childReply->str, NULL, 0);
		else
			continue;

		childReply = r->element[i*2+1];
		if (childReply->type == REDIS_REPLY_STRING)
			score = strtod(childReply->str, NULL);
		else
			continue;

		out.push_back(std::make_pair(playerid, (uint32_t)(score>>32)));
	}

	return 0;
}


	///@ 获取成员的排名
	///@ param[in]	key
	///@ param[in]	member成员
	///@ param[out]	out
int CRedisClient::zget_rank(const char* key, uint64_t member, uint32_t& out)
{
	char command[1024];
	if (!key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	snprintf(command, sizeof(command), "zrevrank %s %lu", key, member);
	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, command);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if (r->type != REDIS_REPLY_INTEGER)
		return -3;

	out = (uint32_t)(r->integer);

	return 0;
}

	///@ 获取成员的积分
	///@ param[in]	key
	///@ param[in]	member成员
	///@ param[out]	out
int CRedisClient::zget_score(const char* key, uint64_t member, uint32_t& out)
{
	char command[1024];
	if (!key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	snprintf(command, sizeof(command), "zscore %s %lu", key, member); /// get score
	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, command);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);

	if( r->type != REDIS_REPLY_STRING )
		return -3;

	uint64_t score = strtod(r->str, NULL);
	out = (uint32_t)(score>>32);

	return 0;
}

	/// 删除某人的排行
int CRedisClient::zdel(const char* key, const std::vector<uint64_t>& dellist)
{
	if (!key || dellist.size()<=0)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	std::ostringstream os;
	os << " zrem " << key;

	for (size_t i=0; i<dellist.size(); ++i)
		os << "  " << dellist[i];

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, os.str().c_str());
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if( !(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK")==0))
		return -3;

	return 0;
}

	///@ 删除指定范围的成员
	///@ param[in]	key
	///@ param[in]	startNo开始排名,从0开始
	///@ param[in]	endNo结束排名
int CRedisClient::zdel_rank(const char* key, int startNo, int endNo)
{
	if (!key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char command[1024];
	snprintf(command, sizeof(command), "ZREMRANGEBYRANK %s %d %d", key, startNo, endNo);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, command);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if(r->type != REDIS_REPLY_INTEGER)
		return -3;

	return 0;
}


	/// 链表操作
	/// 插入链表尾部
int CRedisClient::list_insert(const char* key, const char* value)
{
	if (!key || !value)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "lpush %s %s", key, value);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if(r->type == REDIS_REPLY_INTEGER)
		return 0;

	return -1;
}

	/// 链表操作
	/// 修改链表的值
int CRedisClient::list_set(const char* key, uint32_t idx,  const char* value)
{
	if (!key || !value)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "LSET %s %u %s", key, idx, value);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if(r->type == REDIS_REPLY_INTEGER)
		return 0;

	return -1;
}

	/// 获取指定元素
int CRedisClient::list_get(const char* key, int startNo, int endNo, std::vector<std::string>& out)
{
	if (!key || startNo>endNo)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "LRANGE %s %d %d", key, startNo, endNo);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if( r->type != REDIS_REPLY_ARRAY )
		return -3;

	for (size_t i = 0; i < r->elements; ++i) {
		redisReply* childReply = r->element[i];
		if (childReply->type == REDIS_REPLY_STRING) {
			std::string tmp;
			tmp.assign(childReply->str, childReply->len);
			out.push_back(tmp);
		}
	}

	return 0;
}

	/// 获取指定元素
int CRedisClient::get_score(const char* key, int startScore, int endScore, std::vector<uint64_t>& out)
{
	if (!key || startScore>endScore)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "ZREVRANGE  %s %d %d", key, startScore, endScore);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if( r->type != REDIS_REPLY_ARRAY )
		return -3;

	for (size_t i = 0; i < r->elements; ++i) {
		redisReply* childReply = r->element[i];
		if (childReply->type == REDIS_REPLY_STRING) {
			std::string tmp;
			tmp.assign(childReply->str, childReply->len);
			uint64_t uid = strtoull(const_cast<char*>(tmp.c_str()), NULL, 0);
			out.push_back(uid);
		}
	}

	return 0;
}

	/// 从头部删除一个元素
int CRedisClient::list_header_pop(const char* key)
{
	if (!key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "rpop %s", key);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if( !(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK")==0))
		return -3;

	return 0;
}

int CRedisClient::list_size(const char* key)
{
	if (!key)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "llen %s", key);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -2;
	}

	autoR.set(r);
	if(r->type != REDIS_REPLY_INTEGER)
		return -3;

	return r->integer;
}

int CRedisClient::lock(const char* key, const char* val)
{
	if (!key || !val)
		return 0;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "SETNX %s %s", key, val);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return 0;
	}

	autoR.set(r);
	if(r->type == REDIS_REPLY_INTEGER )
		return r->integer;

	return 0;
}

int CRedisClient::unlock(const char* key)
{
	if (!key)
		return 0;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "del %s", key);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return 0;
	}

	autoR.set(r);
	if(r->type == REDIS_REPLY_INTEGER )
		return r->integer;

	return 0;
}

int CRedisClient::get_all_val(const char* table, std::vector<std::string>& s1)
{
	if (!table)
		return 0;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "hgetall %s", table);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -1;
	}

	autoR.set(r);
	if(r->type != REDIS_REPLY_ARRAY )
		return -1;

	for (size_t i = 0; i < r->elements; ++i) {
		std::string name;
		redisReply* childReply = r->element[i];
		if (childReply->type == REDIS_REPLY_STRING) {
			std::string szVal;
			szVal.assign(childReply->str, childReply->len);
			s1.push_back(szVal);
		}
	}

	return 0;
}

int CRedisClient::incr(const char* key)
{
	if (!key)
		return 0;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "incr %s", key);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return 0;
	}

	autoR.set(r);
	if(r->type != REDIS_REPLY_INTEGER )
		return -1;

	return r->integer>0 ? 1 : 0;
}

	//////////////////////////////////////////////////////////////////
void CRedisClient::clear()
{
	if (m_redisCtx)
	{
		redisFree(m_redisCtx);
		m_redisCtx = NULL;
	}
}


int CRedisClient::connect()
{
	if (m_isConnect)
		return 0;

	clear();

	struct timeval tv;
	tv.tv_sec = m_timeout/1000;
	tv.tv_usec = m_timeout%1000;

	m_redisCtx = redisConnectWithTimeout(m_szIp.c_str(), m_nPort, tv);
	if (NULL == m_redisCtx || m_redisCtx->err) {
		LOG_ERR("[%s : %d]: connect redis server failed, err: %d, errmsg: %s", __FUNCTION__, __LINE__, m_redisCtx->err, m_redisCtx->errstr);
		clear();
		return -2;
	}

	m_isConnect = true;
	redisSetTimeout(m_redisCtx, tv);

	return 0;
}

int CRedisClient::hkeys(const char* table, std::set<uint64_t>& s1)
{
	if (!table)
		return -1;

	int ret = connect();
	if (0 != ret) {
		LOG_ERR("connect redis failed\n");
		return -1;
	}

	char buffer[1024] = { 0 };
	snprintf(buffer, sizeof(buffer), "HKEYS %s", table);

	CAutoRedisReply autoR;
	redisReply* r = (redisReply*)redisCommand(m_redisCtx, buffer);
	if (NULL == r) {
		LOG_ERR("[%s : %d]: call redisCommand error, redis break connection,m_redisCtx: %p", __FUNCTION__, __LINE__, m_redisCtx);
		m_isConnect = false;
		return -1;
	}

	autoR.set(r);
	if(r->type != REDIS_REPLY_ARRAY )
		return -1;

	for (size_t i = 0; i < r->elements; ++i) {
		redisReply* childReply = r->element[i];
		if (childReply->type == REDIS_REPLY_STRING) {
			uint64_t playerid = strtoull(childReply->str, NULL, 0);
			s1.insert(playerid);
		}
	}

	return 0;
}
