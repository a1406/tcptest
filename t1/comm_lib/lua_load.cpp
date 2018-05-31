#include "lua_load.h"
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};
#include "sproto.h"
#include "sprotoc_common.h"

#define UNUSED(x) (void)(x)

int traverse_array(lua_State *L, struct field *f, void *data, uint32_t *n_size)
{
	std::vector<uint64_t> vec_int;
	std::vector<char *> vec_str;
	std::vector<void *> vec_void;
	std::vector<double> vec_double;

	assert(f->type & SPROTO_TARRAY);
	int f_type = f->type & ~SPROTO_TARRAY;
	
	lua_pushnil(L);
	    // 现在的栈：-1 => nil; index => table
//	int index = -2;
	while (lua_next(L, -2))
	{
			// 现在的栈：-1 => value; -2 => key; index => table
			// 拷贝一份 key 到栈顶，然后对它做 lua_tostring 就不会改变原始的 key 值了
		lua_pushvalue(L, -2);
		lua_pop(L, 1);
		int type = lua_type(L, -1);
		switch (type)
		{
			case LUA_TNUMBER:
			{
				if (!f)
					break;
				assert(f_type == SPROTO_TINTEGER || f_type == SPROTO_TDOUBLE);

				if (f_type == SPROTO_TINTEGER)
				{
					uint64_t n = (uint64_t)(lua_tonumber(L, -1));
					vec_int.push_back(n);
				}
				else
				{
					double n = lua_tonumber(L, -1);
					vec_double.push_back(n);					
				}
//				printf("value = %d\n", (int)(lua_tonumber(L, -1)));
				break;
			}
			case LUA_TSTRING:
			{
				if (!f)
					break;				
				assert(f_type == SPROTO_TSTRING);
				char *n = strdup(lua_tostring(L, -1));				
				vec_str.push_back(n);
//				printf("value = %s\n", lua_tostring(L, -1));
				break;
			}
			case LUA_TTABLE:
			{
				if (!f)
					break;
				assert(f_type == SPROTO_TSTRUCT);
				void *data = malloc(f->st->c_size);
				assert(data);
				memset(data, 0, f->st->c_size);
				traverse_table(L, f->st, data);
				vec_void.push_back(data);
				break;
			}
			default:  //只需要支持数字和string的数组
			{
				assert(0);
				exit(0);
//				printf("value = none\n");
				break;
			}
				
		}
//		printf(": value type %d %s\n", type, lua_typename(L, type));

//		data[key]=value;
			// 弹出 value 和拷贝的 key，留下原始的 key 作为下一次 lua_next 的参数
		lua_pop(L, 1);
			// 现在的栈：-1 => key; index => table
	}
	if (f_type == SPROTO_TSTRING)
	{
		*n_size = vec_str.size();
		char ***p = (char ***)(data + f->offset + sizeof(uint32_t));
		*p = (char **)malloc(sizeof(char *) * (*n_size));
		for (size_t i = 0; i < (*n_size); ++i)
			(*p)[i] = vec_str[i];
	}
	else if (f_type == SPROTO_TDOUBLE)
	{
		*n_size = vec_double.size();
		double **p = (double **)(data + f->offset + sizeof(uint32_t));
		(*p) = (double *)malloc(sizeof(double) * (*n_size));
		for (size_t i = 0; i < (*n_size); ++i)
			(*p)[i] = vec_double[i];		
	}
	else if (f_type == SPROTO_TINTEGER)
	{
		*n_size = vec_int.size();
		uint64_t **p = (uint64_t **)(data + f->offset + sizeof(uint32_t));
		(*p) = (uint64_t *)malloc(sizeof(uint64_t) * (*n_size));
		for (size_t i = 0; i < (*n_size); ++i)
			(*p)[i] = vec_int[i];		
	}
	else if (f_type == SPROTO_TSTRUCT)
	{
		*n_size = vec_void.size();
		void ***p = (void ***)(data + f->offset + sizeof(uint32_t));
		(*p) = (void **)malloc(sizeof(void *) * (*n_size));
		for (size_t i = 0; i < (*n_size); ++i)
			(*p)[i] = vec_void[i];		
	}
	return (0);
}

int traverse_table(lua_State *L, struct sproto_type *sproto_type, void *data)
{
	lua_pushnil(L);
	    // 现在的栈：-1 => nil; index => table
//	int index = -2;
	while (lua_next(L, -2))
	{
			// 现在的栈：-1 => value; -2 => key; index => table
			// 拷贝一份 key 到栈顶，然后对它做 lua_tostring 就不会改变原始的 key 值了
		lua_pushvalue(L, -2);
			// 现在的栈：-1 => key; -2 => value; -3 => key; index => table

		struct field *f = NULL;

		const char* key = lua_tostring(L, -1);
		int i;
		for (i = 0; sproto_type && i < sproto_type->n; ++i)
		{
			const char *type_name = sproto_type->f[i].name;
			if (key[0] >= '0' && key[0] <= '9')
			{
				type_name++;
			}
			if (strcmp(type_name, key) == 0)
			{
				f = &sproto_type->f[i];
				break;
			}
		}
//		printf("key = %s\n", key);
		lua_pop(L, 1);

		int type = lua_type(L, -1);
		switch (type)
		{
			case LUA_TNUMBER:
				if (!f)
					break;
				assert(f->type == SPROTO_TINTEGER || f->type == SPROTO_TDOUBLE);

				if (f->type == SPROTO_TINTEGER)
					*(uint64_t *)(data + f->offset) = (uint64_t)(lua_tonumber(L, -1));
				else
				{
					double x = lua_tonumber(L, -1);
					*(double *)(data + f->offset) = x;
				}

//				printf("value = %d\n", (int)(lua_tonumber(L, -1)));
				break;
			case LUA_TSTRING:
				if (!f)
					break;				
				assert(f->type == SPROTO_TSTRING);
				*(char **)(data + f->offset) = strdup(lua_tostring(L, -1));				
//				printf("value = %s\n", lua_tostring(L, -1));
				break;
			case LUA_TTABLE:
				if (!f)
				{
					traverse_table(L, NULL, NULL);
					break;
				}
				if (f->type & SPROTO_TARRAY)
				{
					uint32_t *n_size = (uint32_t *)(data + f->offset);
					traverse_array(L, f, data, n_size);
				}
				else
				{
				
					assert(f->type == SPROTO_TSTRUCT);
					assert(f->st);
//				printf("value = new table\n");
					*(void **)(data + f->offset) = malloc(f->st->c_size);
					traverse_table(L, f->st, *(void **)(data + f->offset));
				}
				break;
			default:
				assert(0);
				exit(0);
//				printf("value = none\n");
				break;
				
		}
//		printf(": value type %d %s\n", type, lua_typename(L, type));

//		data[key]=value;
			// 弹出 value 和拷贝的 key，留下原始的 key 作为下一次 lua_next 的参数
		lua_pop(L, 1);
			// 现在的栈：-1 => key; index => table
	}
	    // 现在的栈：index => table （最后 lua_next 返回 0 的时候它已经把上一次留下的 key 给弹出了）
	    // 所以栈已经恢复到进入这个函数时的状态
	return 0;
}

int traverse_vector_table(lua_State *L, struct sproto_type *sproto_type, const char *lua_file_name, std::vector<void *> *ret)
{
	if (1 == luaL_dofile(L, lua_file_name))
	{
		printf("do file failed, file_name:%s\n", lua_file_name);
		return -1;
	}
	lua_pushnil(L);
	    // 现在的栈：-1 => nil; index => table
//	int index = -2;
	while (lua_next(L, -2))
	{
			// 现在的栈：-1 => value; -2 => key; index => table
			// 拷贝一份 key 到栈顶，然后对它做 lua_tostring 就不会改变原始的 key 值了
		lua_pushvalue(L, -2);
			// 现在的栈：-1 => key; -2 => value; -3 => key; index => table

		double key = lua_tonumber(L, -1);
		UNUSED(key);
//		printf("key = %d\n", (int)key);
		lua_pop(L, 1);

		int type = lua_type(L, -1);
		assert(type == LUA_TTABLE);

		void *data = malloc(sproto_type->c_size);
		memset(data, 0, sproto_type->c_size);

		traverse_table(L, sproto_type, data);
		
		lua_pop(L, 1);

		(ret)->push_back(data);

//		struct ActiveSkillTable *t = (struct ActiveSkillTable *)data;
//		printf("\n");
	}
	return 0;
}

int traverse_main_table(lua_State *L, struct sproto_type *sproto_type, const char *lua_file_name, std::map<uint64_t, void *> *ret)
{
	ret->clear();
	if (1 == luaL_dofile(L, lua_file_name))
	{
		printf("do file failed, file_name:%s\n", lua_file_name);
		return -1;
	}
	lua_pushnil(L);
	    // 现在的栈：-1 => nil; index => table
//	int index = -2;
	while (lua_next(L, -2))
	{
			// 现在的栈：-1 => value; -2 => key; index => table
			// 拷贝一份 key 到栈顶，然后对它做 lua_tostring 就不会改变原始的 key 值了
		lua_pushvalue(L, -2);
			// 现在的栈：-1 => key; -2 => value; -3 => key; index => table

		double key = lua_tonumber(L, -1);
//		printf("key = %d\n", (int)key);
		lua_pop(L, 1);

		int type = lua_type(L, -1);
		assert(type == LUA_TTABLE);

		void *data = malloc(sproto_type->c_size);
		memset(data, 0, sproto_type->c_size);

		traverse_table(L, sproto_type, data);
		
		lua_pop(L, 1);

		(*ret)[(uint64_t)key] = data;

//		struct ActiveSkillTable *t = (struct ActiveSkillTable *)data;
//		printf("\n");
	}
	return 0;
}

std::vector<struct SceneCreateMonsterTable *> *traverse_create_monster_table(lua_State *L, struct sproto_type *sproto_type, const char *lua_file_name)
{
	std::vector<struct SceneCreateMonsterTable *> *ret = new std::vector<struct SceneCreateMonsterTable *>;
	if (1 == luaL_dofile(L, lua_file_name))
	{
		printf("do file failed, file_name:%s\n", lua_file_name);
		return NULL;
	}
	lua_pushnil(L);
	    // 现在的栈：-1 => nil; index => table
//	int index = -2;
	while (lua_next(L, -2))
	{
			// 现在的栈：-1 => value; -2 => key; index => table
			// 拷贝一份 key 到栈顶，然后对它做 lua_tostring 就不会改变原始的 key 值了
		lua_pushvalue(L, -2);
			// 现在的栈：-1 => key; -2 => value; -3 => key; index => table

//		double key = lua_tonumber(L, -1);
//		printf("key = %d\n", (int)key);
		lua_pop(L, 1);

		int type = lua_type(L, -1);
		assert(type == LUA_TTABLE);

		void *data = malloc(sproto_type->c_size);
		memset(data, 0, sproto_type->c_size);

		traverse_table(L, sproto_type, data);
		
		lua_pop(L, 1);

		ret->push_back((struct SceneCreateMonsterTable *)data);
//		(*ret)[(uint64_t)key] = data;

//		struct ActiveSkillTable *t = (struct ActiveSkillTable *)data;
//		printf("\n");
	}
	return ret;
}








