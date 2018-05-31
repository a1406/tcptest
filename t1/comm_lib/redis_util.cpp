#include "redis_util.h"

AutoReleaseRedisPlayer::AutoReleaseRedisPlayer()
{
	pointer = NULL;
}

AutoReleaseRedisPlayer::~AutoReleaseRedisPlayer()
{
	if (pointer)	
		player_redis_info__free_unpacked(pointer, NULL);

}
void AutoReleaseRedisPlayer::set(PlayerRedisInfo* r)
{
	if (pointer)
	{
		player_redis_info__free_unpacked(pointer, NULL);		
	}
	pointer = r;
}


AutoReleaseBatchRedisPlayer::AutoReleaseBatchRedisPlayer()
{
}

AutoReleaseBatchRedisPlayer::~AutoReleaseBatchRedisPlayer()
{
	for (std::vector<PlayerRedisInfo*>::iterator iter = pointer_vec.begin(); iter != pointer_vec.end(); ++iter)
	{
		player_redis_info__free_unpacked(*iter, NULL);
	}
}

void AutoReleaseBatchRedisPlayer::push_back(PlayerRedisInfo *player)
{
	pointer_vec.push_back(player);
}

PlayerRedisInfo *get_redis_player(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseRedisPlayer &_pool)
{
	static uint8_t data_buffer[32 * 1024];
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(player_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		PlayerRedisInfo *ret = player_redis_info__unpack(NULL, data_len, data_buffer);
		if (ret)
			_pool.set(ret);
		return ret;
	}

	return NULL;
}

PlayerRedisInfo *get_redis_player(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseBatchRedisPlayer &_pool)
{
	static uint8_t data_buffer[32 * 1024];
	int data_len = sizeof(data_buffer);
	char field[64];
	sprintf(field, "%lu", player_id);
	int ret = rc.hget_bin(player_key, field, (char *)data_buffer, &data_len);
	if (ret == 0)
	{
		PlayerRedisInfo *ret = player_redis_info__unpack(NULL, data_len, data_buffer);
		if (ret)
			_pool.push_back(ret);
		return ret;
	}

	return NULL;
}

int get_more_redis_player(std::set<uint64_t> &player_ids, std::map<uint64_t, PlayerRedisInfo*> &redis_players,
	char *player_key, CRedisClient &rc, AutoReleaseBatchRedisPlayer &_pool)
{
	if (player_ids.size() == 0)
	{
		return 0;
	}

	std::vector<std::relation_three<uint64_t, char*, int> > player_infos;
	for (std::set<uint64_t>::iterator iter = player_ids.begin(); iter != player_ids.end(); ++iter)
	{
		std::relation_three<uint64_t, char*, int> tmp(*iter, NULL, 0);
		player_infos.push_back(tmp);
	}

	int ret = rc.get(player_key, player_infos);
	if (ret != 0)
	{
		LOG_ERR("[%s:%d] hmget failed, ret:%d", __FUNCTION__, __LINE__, ret);
		return -1;
	}

	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = player_infos.begin(); iter != player_infos.end(); ++iter)
	{
		PlayerRedisInfo *redis_player = player_redis_info__unpack(NULL, iter->three, (uint8_t*)iter->second);
		if (!redis_player)
		{
			ret = -1;
			LOG_ERR("[%s:%d] unpack redis failed, player_id:%lu", __FUNCTION__, __LINE__, iter->first);
			continue;
		}

		redis_players[iter->first] = redis_player;
		_pool.push_back(redis_player);
	}

	for (std::vector<std::relation_three<uint64_t, char*, int> >::iterator iter = player_infos.begin(); iter != player_infos.end(); ++iter)
	{
		free(iter->second);
	}

	return ret;
}
