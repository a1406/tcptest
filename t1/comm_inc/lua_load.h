#ifndef __LUA_LOAD_H__
#define __LUA_LOAD_H__

#include <map>
#include <vector>
#include <stdint.h>
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};

int traverse_array(lua_State *L, struct field *f, void *data, uint32_t *n_size);
int traverse_table(lua_State *L, struct sproto_type *sproto_type, void *data);
int traverse_main_table(lua_State *L, struct sproto_type *sproto_type, const char *lua_file_name, std::map<uint64_t, void *> *ret);
int traverse_vector_table(lua_State *L, struct sproto_type *sproto_type, const char *lua_file_name, std::vector<void *> *ret);
std::vector<struct SceneCreateMonsterTable *> *traverse_create_monster_table(lua_State *L, struct sproto_type *sproto_type, const char *lua_file_name);

#endif /* __LUA_LOAD_H__ */
