#ifndef __CGI_COMMON_H_
#define __CGI_COMMON_H_

#include "mysql_module.h"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <string>
#include "mysql_module.h"
#include <openssl/md5.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include "oper_config.h"
#include "ResMapTempete.h"
//#include "app_data_statis.h"
#include <json-c/json.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define ANET_IP_ONLY (1<<0)
#define ANET_OK 0
#define ANET_ERR -1

static int anetGenericResolve(const char *host, char *ipbuf, size_t ipbuf_len, int flags)
{
    struct addrinfo hints, *info;
    int rv;

    memset(&hints,0,sizeof(hints));
    if (flags & ANET_IP_ONLY) hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;  /* specify socktype to avoid dups */

    if ((rv = getaddrinfo(host, NULL, &hints, &info)) != 0) {
//        printf("error: %s", gai_strerror(rv));
        return ANET_ERR;
    }
    if (info->ai_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *)info->ai_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, ipbuf_len);
    } else {
        struct sockaddr_in6 *sa = (struct sockaddr_in6 *)info->ai_addr;
        inet_ntop(AF_INET6, &(sa->sin6_addr), ipbuf, ipbuf_len);
    }

    freeaddrinfo(info);
    return ANET_OK;
}

static inline int init_mysql(const char* configfile) {
	std::ifstream file;
	std::string szMysqlIp;
	std::string szMysqlDbName;
	std::string szMysqlDbUser;
	std::string szMysqlDbPwd;
	int nMysqlPort = 0;

	file.open(configfile, std::ifstream::in);
	if (!file.good()) {
//		printf("{\"code\":1, \"reason\": \"open config failed\"}");
		file.close();
		return -1;
	}
	
	char* line = get_first_key(file, (char *)"mysql_host");
	if (!line) {
//		printf("{\"code\":1, \"reason\": \"get mysql_host config failed\"}");
		file.close();
		return -1;
	}
	szMysqlIp = (get_value(line));

	line = get_first_key(file, (char *)"mysql_port");
	if (!line) {
//		printf("{\"code\":1, \"reason\": \"get mysql_port config failed\"}");
		file.close();
		return -1;
	}
	nMysqlPort = atoi(get_value(line));

	line = get_first_key(file, (char *)"mysql_db_name");
	if (!line) {
//		printf("{\"code\":1,\"reason\":\"get mysql_db_name config failed\"}");
		file.close();
		return -1;
	}
	szMysqlDbName = (get_value(line));

	line = get_first_key(file, (char *)"mysql_db_user");
	if (!line) {
//		printf("{\"code\":1, \"reason\": \"get mysql_db_user config failed\"}");
		file.close();
		return -1;
	}
	szMysqlDbUser = (get_value(line));

	line = get_first_key(file, (char *)"mysql_db_pwd");
	if (!line) {
//		printf("{\"code\":1, \"reason\": \"get mysql_db_pwd config failed\"}");
		file.close();
		return -1;
	}

	szMysqlDbPwd = (get_value(line));

	int ret = init_db(const_cast<char*>(szMysqlIp.c_str()), nMysqlPort,  const_cast<char*>(szMysqlDbName.c_str())
		, const_cast<char*>(szMysqlDbUser.c_str()), const_cast<char*>(szMysqlDbPwd.c_str()));
	if (0 != ret) {
//		printf("{\"code\":1, \"reason\":\"init db failed\"}");
		file.close();
		return -1;
	}

	file.close();

	return 0;
}

inline unsigned char ToHex(unsigned char x) {
        return  x > 9 ? x + 55 : x + 48;
}

inline unsigned char FromHex(unsigned char x) {
        unsigned char y;
        if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
        else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
        else if (x >= '0' && x <= '9') y = x - '0';
        else {
			assert(0);
			exit(0);
		}
        return y;
}

static inline std::string UrlEncode(const std::string& str)
{
        std::string strTemp = "";
        size_t length = str.length();
        for (size_t i = 0; i < length; i++) {
                if (isalnum((unsigned char)str[i]) ||
                        (str[i] == '-') ||
                        (str[i] == '_') ||
                        (str[i] == '.') ||
                        (str[i] == '~'))
                        strTemp += str[i];
                else if (str[i] == ' ')
                        strTemp += "+";
                else {
                        strTemp += '%';
                        strTemp += ToHex((unsigned char)str[i] >> 4);
                        strTemp += ToHex((unsigned char)str[i] % 16);
                }
        }
        return strTemp;
}


static inline std::string UrlDecode(const std::string& str)
{
        std::string strTemp = "";
        size_t length = str.length();
        for (size_t i = 0; i < length; i++)
        {
                if (str[i] == '+') strTemp += ' ';
                else if (str[i] == '%')
                {
                        assert(i + 2 < length);
                        unsigned char high = FromHex((unsigned char)str[++i]);
                        unsigned char low = FromHex((unsigned char)str[++i]);
                        strTemp += high*16 + low;
                }
                else strTemp += str[i];
        }
        return strTemp;
}  

static inline int uri_parse(const std::string& szTxt, std::map<std::string, std::string>& out, const char section='&', const char separator='=') {
	size_t posBeg = 0;
	size_t posEnd = 0;

	while ( (posEnd = szTxt.find_first_of(section, posBeg))!=std::string::npos ) {
		std::string tmp = szTxt.substr(posBeg, posEnd-posBeg);

		size_t pos = tmp.find(separator);
		std::string szKey = tmp.substr(0, pos);
		std::string szVal = tmp.substr(pos+1);

		out[szKey] = szVal;
		posBeg = posEnd + 1;
	}

	if (posBeg<szTxt.length()) {
		std::string tmp = szTxt.substr(posBeg, szTxt.length() - posBeg);
		size_t pos = tmp.find(separator);

		std::string szKey = tmp.substr(0, pos);
		std::string szVal = tmp.substr(pos+1);

		out[szKey] = szVal;
	}

	return 0;
}

static inline int item_parse(const std::string& szTxt, std::vector<uint32_t>& outid, std::vector<uint32_t>& outnum, const char section='&', const char separator='=') {
	size_t posBeg = 0;
	size_t posEnd = 0;

	while ( (posEnd = szTxt.find_first_of(section, posBeg))!=std::string::npos ) {
		std::string tmp = szTxt.substr(posBeg, posEnd-posBeg);

		size_t pos = tmp.find(separator);
		std::string szKey = tmp.substr(0, pos);
		std::string szVal = tmp.substr(pos+1);

		uint32_t id = strtoul(szKey.c_str(), NULL, 0);
		uint32_t num = strtoul(szVal.c_str(), NULL, 0);

		outid.push_back(id);
		outnum.push_back(num);
//		out[id] = num;
		posBeg = posEnd + 1;
	}

	if (posBeg<szTxt.length()) {
		std::string tmp = szTxt.substr(posBeg, szTxt.length() - posBeg);
		size_t pos = tmp.find(separator);

		std::string szKey = tmp.substr(0, pos);
		std::string szVal = tmp.substr(pos+1);

		uint32_t id = strtoul(szKey.c_str(), NULL, 0);
		uint32_t num = strtoul(szVal.c_str(), NULL, 0);

//		out[id] = num;
		outid.push_back(id);
		outnum.push_back(num);
	}

	return 0;
}

static inline std::string encoder_md5(const std::string& src) {
	unsigned char buffer[1024] = { 0 };
	MD5_CTX ctx;	
	MD5_Init(&ctx);	
	MD5_Update(&ctx, (void *)src.c_str(), src.length());
	MD5_Final(buffer, &ctx);

	char md5Buffer[33] = {0};
	snprintf(md5Buffer, sizeof(md5Buffer), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
		(uint32_t)buffer[0], (uint32_t)buffer[1], (uint32_t)buffer[2], (uint32_t)buffer[3], 
		(uint32_t)buffer[4], (uint32_t)buffer[5], (uint32_t)buffer[6], (uint32_t)buffer[7], 
		(uint32_t)buffer[8], (uint32_t)buffer[9], (uint32_t)buffer[10], (uint32_t)buffer[11], 
		(uint32_t)buffer[12], (uint32_t)buffer[13], (uint32_t)buffer[14], (uint32_t)buffer[15]);
	md5Buffer[32] = '\0';

	return std::string(md5Buffer);
}


// static inline int get_server_info(const char* filename, uint32_t server_id, std::string& szIP, uint32_t& port) {
// 	static bool isInit = false;
// 	static ResMapTempete<tagServerSet> serverData;	
// 	if (!isInit) {
// 		serverData.loadData(filename);
// 		isInit = true;
// 	}
// 
// 	tagServerSet* config = serverData.getDataById(server_id);
// 	if (!config) {
// 		return -1;
// 	}
// 
// 	szIP = std::string(config->szIPAddr);
// 	port = config->iGmPort;
// 
// 	return 0;	
// }

static inline int get_server_db_info(uint32_t serverid, const char* configName, std::string& szIp, int& port, std::string& dbName, std::string& userName, std::string& userPwd) {

	char srv_mysql[1024] = {0};
	int result = init_mysql(configName);
	if (0 != result) {
//		printf ("{\"code\":1,\"reason\": \"init db failed\"}");
		return -1;
	}

	snprintf(srv_mysql, sizeof(srv_mysql), "select srvid, ip, port, mysql_db_name, mysql_db_user, mysql_db_pwd from mysql_server where srvid=%u", serverid);

	MYSQL_RES* res = query(srv_mysql, 0, NULL);
	if (!res) {
//		printf ("{\"code\":1,\"reason\": \"select server db failed 1\"}");
		return -1;
	}

	MYSQL_ROW row = fetch_row(res);
	if (!row) {
//		printf ("{\"code\":1,\"reason\": \"select server db failed 2\"}");
		return -1;
	}

	szIp = std::string(row[1]);
	port = atoi(row[2]);
	dbName = std::string(row[3]);
	userName = std::string(row[4]);
	userPwd = std::string(row[5]);

	close_db();
	return 0;
}

static inline size_t __curl_process_data__(void *buffer, size_t size, size_t nmemb, void *userdata) {
	strcat((char *)userdata, (char *)buffer);
	return size * nmemb;
}

static inline bool get_server_info(uint32_t srvid, std::string& szIp, int& port, std::string& serverName) {
	char szUrl[1024] = { 0 };
	char outStr[2048] = { 0 };

	memset(outStr, 0, sizeof(outStr));
//	snprintf(szUrl, sizeof(szUrl), "http://ghzlist.ycatgame.com:8080/game_manage/web/server_list.php?id=%u", srvid);
	snprintf(szUrl, sizeof(szUrl), "http://ghzlist.ycatgame.com:8080/newweb/game_manage/web/server_list_for_server.php?id=%u", srvid);

	CURL* curl_obj = curl_easy_init();	
	if (!curl_obj) {
		printf ("{\"code\": 1, \"reason\": \"call curl_easy_init failed\"}");
		return false;	
	}

	curl_easy_setopt(curl_obj, CURLOPT_URL, szUrl);
	curl_easy_setopt(curl_obj, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEFUNCTION, &__curl_process_data__);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEDATA, outStr);

	int ret = curl_easy_perform(curl_obj);
	if (0 != ret) {
		printf ("{\"code\": 1, \"reason\": \"call curl_easy_perform failed!\"}");
		fprintf(stderr, "curl_easy_perform failed ret: %u", ret);
		curl_easy_cleanup(curl_obj);
		return false;	
	}

	curl_easy_cleanup(curl_obj);

	json_object *new_obj = json_tokener_parse(outStr);

	if (!new_obj) {
		fprintf(stderr, "resp is not json, server_id: %u", srvid);
		return false;
	}

	json_object *obj = json_object_array_get_idx(new_obj, 0);
	if (!obj) {
		fprintf(stderr, "resp is not json array, server_id: %u", srvid);
		return false;
	}

	std::map<std::string, std::string>  jsonMap;
	json_object_object_foreach(obj, key, val) 
	{
		jsonMap[std::string(key)] = std::string(json_object_to_json_string(val));
	}

	std::map<std::string, std::string>::iterator it;
	it = jsonMap.find("serverIp");
	if (it == jsonMap.end())
		return false;

	szIp.assign(it->second.c_str()+1, it->second.length()-2);
	char buf[100];
	anetGenericResolve(szIp.c_str(), buf, 100, 0);
	szIp.assign(buf);

	it = jsonMap.find("port2");
	if (it == jsonMap.end()) {
		json_object_put(new_obj);
		return false;
	}

	std::string szTmpPort;
	szTmpPort.assign(it->second.c_str()+1, it->second.length()-2);
	port = atoi(szTmpPort.c_str());

	it = jsonMap.find("serverName");
	if (it != jsonMap.end()) {
		serverName.assign(it->second.c_str()+1, it->second.length()-2);
	}	 

	json_object_put(new_obj);

	return true;
}

static inline bool get_server_max_conn(uint32_t srvid, uint32_t& maxConn, std::string& channel) {
	char szUrl[1024] = { 0 };
	char outStr[2048] = { 0 };

	memset(outStr, 0, sizeof(outStr));
//	snprintf(szUrl, sizeof(szUrl), "http://ghzlist.ycatgame.com:8080/game_manage/web/server_list.php?id=%u", srvid);
	snprintf(szUrl, sizeof(szUrl), "http://ghzlist.ycatgame.com:8080/newweb/game_manage/web/server_list_for_server.php?id=%u", srvid);

	CURL* curl_obj = curl_easy_init();	
	if (!curl_obj) {
		printf ("{\"code\": 1, \"reason\": \"call curl_easy_init failed\"}");
		return false;	
	}

	curl_easy_setopt(curl_obj, CURLOPT_URL, szUrl);
	curl_easy_setopt(curl_obj, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEFUNCTION, &__curl_process_data__);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEDATA, outStr);

	int ret = curl_easy_perform(curl_obj);
	if (0 != ret) {
		printf ("{\"code\": 1, \"reason\": \"call curl_easy_perform failed!\"}");
		curl_easy_cleanup(curl_obj);
		return false;	
	}

	curl_easy_cleanup(curl_obj);

	json_object *new_obj = json_tokener_parse(outStr);
	if (!new_obj ) {
		printf ("{\"code\": 1, \"reason\": \"result error: out str 1: %s!\"}", outStr);
		return false;
	}

	json_object *obj = json_object_array_get_idx(new_obj, 0);
	if (!obj) {
		printf ("{\"code\": 1, \"reason\": \"result error: out str 2: %s!\"}", outStr);
		return false;
	}

	std::map<std::string, std::string>  jsonMap;
	json_object_object_foreach(obj, key, val) 
	{
		jsonMap[std::string(key)] = std::string(json_object_to_json_string(val));
	}

	std::map<std::string, std::string>::iterator it;

	it = jsonMap.find("maxUser");
	if (it == jsonMap.end()) {
		json_object_put(new_obj);
		return false;
	}

	std::string szTmpPort;
	szTmpPort.assign(it->second.c_str()+1, it->second.length()-2);
	maxConn = atoi(szTmpPort.c_str());

	it = jsonMap.find("channel");
	if (it!=jsonMap.end()) {
		std::string szTmpPort;
		szTmpPort.assign(it->second.c_str()+1, it->second.length()-2);
		channel = szTmpPort;
	}

	json_object_put(new_obj);

	return true;
}

static inline bool get_server_open_service_time(uint32_t srvid, uint32_t& ts) {
	char szUrl[1024] = { 0 };
	char outStr[2048] = { 0 };

	memset(outStr, 0, sizeof(outStr));
	snprintf(szUrl, sizeof(szUrl), "http://ghzlist.ycatgame.com:8080/newweb/game_manage/web/server_list_for_server.php?id=%u", srvid);

	CURL* curl_obj = curl_easy_init();	
	if (!curl_obj) {
		printf ("{\"code\": 1, \"reason\": \"call curl_easy_init failed\"}");
		return false;	
	}

	curl_easy_setopt(curl_obj, CURLOPT_URL, szUrl);
	curl_easy_setopt(curl_obj, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEFUNCTION, &__curl_process_data__);
	curl_easy_setopt(curl_obj, CURLOPT_WRITEDATA, outStr);

	int ret = curl_easy_perform(curl_obj);
	if (0 != ret) {
		printf ("{\"code\": 1, \"reason\": \"call curl_easy_perform failed!\"}");
		curl_easy_cleanup(curl_obj);
		return false;	
	}

	curl_easy_cleanup(curl_obj);

	json_object *new_obj = json_tokener_parse(outStr);
	if (!new_obj ) {
		printf ("{\"code\": 1, \"reason\": \"result error: out str 1: %s!\"}", outStr);
		return false;
	}

	json_object *obj = json_object_array_get_idx(new_obj, 0);
	if (!obj) {
		printf ("{\"code\": 1, \"reason\": \"result error: out str 2: %s!\"}", outStr);
		return false;
	}

	std::map<std::string, std::string>  jsonMap;
	json_object_object_foreach(obj, key, val) 
	{
		jsonMap[std::string(key)] = std::string(json_object_to_json_string(val));
	}

	std::map<std::string, std::string>::iterator it;

	it = jsonMap.find("opentime");
	if (it == jsonMap.end()) {
		json_object_put(new_obj);
		return false;
	}

	std::string szTmpPort;
	szTmpPort.assign(it->second.c_str()+1, it->second.length()-2);
	ts = atoi(szTmpPort.c_str());

	json_object_put(new_obj);

	return true;
}

static inline std::string create_md5_val(const std::string& src) {
	unsigned char buffer[1024] = { 0 };

	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, (void *)src.c_str(), src.length());

	MD5_Final(buffer, &ctx);

	char md5Buffer[33] = {0};
	snprintf(md5Buffer, sizeof(md5Buffer), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
		, (uint32_t)buffer[0], (uint32_t)buffer[1], (uint32_t)buffer[2], (uint32_t)buffer[3], (uint32_t)buffer[4], (uint32_t)buffer[5]
	, (uint32_t)buffer[6], (uint32_t)buffer[7], (uint32_t)buffer[8], (uint32_t)buffer[9], (uint32_t)buffer[10], (uint32_t)buffer[11]
	, (uint32_t)buffer[12], (uint32_t)buffer[13], (uint32_t)buffer[14], (uint32_t)buffer[15]);

	md5Buffer[32] = '\0';

	return std::string(md5Buffer);
}

static inline int parse_json(std::string szdata, std::map<std::string, std::string>& out) {
	size_t nS = 0;
	size_t nE = 0;

	size_t posBeg = 0;
	size_t posEnd = 0;

	if ((szdata[0] != '{' && szdata[szdata.length()-1] != '}') || szdata.length()<=2)
		return -1;

	std::string tmpData;
	tmpData.assign(szdata.c_str()+1, szdata.length()-2);

	while ( (posEnd = tmpData.find_first_of('{', posBeg) ) != std::string::npos) {
		++nS;
		posBeg = posEnd+1;
	}

	while ( (posEnd = tmpData.find_first_of('}', posBeg) ) != std::string::npos) {
		++nE;
		posBeg = posEnd+1;
	}

	if (nS != nE && nS!=0) 
		return -1;
	posBeg = posEnd = 0;

	while ( (posEnd = tmpData.find_first_of(',', posBeg))!=std::string::npos ) {
		std::string tmp = tmpData.substr(posBeg, posEnd-posBeg);	
		size_t pos = tmp.find(':');
		std::string szKey = tmp.substr(0, pos);
		std::string szVal = tmp.substr(pos+1);	
		out[szKey] = szVal;
		posBeg = posEnd + 1;	
	}

	if (posBeg<tmpData.length()) {
		std::string tmp = tmpData.substr(posBeg, tmpData.length() - posBeg);		
		size_t pos = tmp.find(':');		
		std::string szKey = tmp.substr(0, pos);
		std::string szVal = tmp.substr(pos+1);
		out[szKey] = szVal;
	}

	return 0;
}

#endif ///__CGI_COMMON_H_
