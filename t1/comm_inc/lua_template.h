#ifndef __LUA_TEMPLATE_H__
#define __LUA_TEMPLATE_H__

#include <stdint.h>
#include <stddef.h>
#include <map>

template <class T1>
T1 *get_config_by_id(uint32_t id, std::map<uint64_t, T1 *> *config)
{
	typename std::map<uint64_t, T1 *>::iterator iter = config->find(id);
	if (iter != config->end())
	{
		return iter->second;
	}
	return NULL;
}

template <class T1>
T1 *get_config_by_name(char *name, std::map<char *, T1 *> *config)
{
	typename std::map<char *, T1 *>::iterator iter = config->find(name);
	if (iter != config->end())
	{
		return iter->second;
	}
	return NULL;
}

#endif /* __LUA_TEMPLATE_H__ */
