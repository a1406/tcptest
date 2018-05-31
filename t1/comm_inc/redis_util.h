#ifndef REDIS_UTIL_H
#define REDIS_UTIL_H
#include <vector>
#include <set>
#include <map>
#include "player_redis_info.pb-c.h"
#include "redis_client.h"

class AutoReleaseRedisPlayer
{
public:
	AutoReleaseRedisPlayer();
	~AutoReleaseRedisPlayer();
	void set(PlayerRedisInfo* r);
private:
	PlayerRedisInfo *pointer;
};

class AutoReleaseBatchRedisPlayer
{
public:
	AutoReleaseBatchRedisPlayer();
	~AutoReleaseBatchRedisPlayer();

	void push_back(PlayerRedisInfo *player);
private:
	std::vector<PlayerRedisInfo *> pointer_vec;
};

class AutoReleaseProtobufStruct
{
	public:
		typedef void (*free_func)(void *message, void *allocator);
		AutoReleaseProtobufStruct(void *message, void *allocator, free_func free_handler) : message(message), allocator(allocator), free_handler(free_handler) {}
		~AutoReleaseProtobufStruct()
		{
			if (free_handler && message)
			{
				(*free_handler)(message, allocator);
			}
		}
	private:
		void *message;
		void *allocator;
		free_func free_handler;
};

PlayerRedisInfo *get_redis_player(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseRedisPlayer &_pool);
PlayerRedisInfo *get_redis_player(uint64_t player_id, char *player_key, CRedisClient &rc, AutoReleaseBatchRedisPlayer &_pool);
int get_more_redis_player(std::set<uint64_t> &player_ids, std::map<uint64_t, PlayerRedisInfo*> &redis_players,
	char *player_key, CRedisClient &rc, AutoReleaseBatchRedisPlayer &_pool);

#endif /* REDIS_UTIL_H */

