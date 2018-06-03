#include "shm_ipc.h"
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include "game_event.h"
#include "oper_config.h"

static shm_ipc_obj *init_shm_ipc_obj(int key, int size, bool create)
{
	int flag = 0666;
	if (create)
	{
		flag |= IPC_CREAT|IPC_EXCL;
	}
	int shmid = shmget(key, size, flag);
	if (shmid == -1) {
		LOG_ERR("%s %d: shmget fail[%d]", __FUNCTION__, __LINE__, errno);
		return NULL;
	}
	
	shm_ipc_obj *ret;
	ret = (shm_ipc_obj *)shmat(shmid, NULL, 0);
	if (ret == (void *)-1) {
		LOG_ERR("%s %d: shmat fail[%d]", __FUNCTION__, __LINE__, errno);
		return NULL;
	}

	if (create)
	{
		ret->size = size;
		ret->read = ret->write = 0;
//		ret->mem = (void *)(ret->data);
//		ret->read_ptr = ret->write_ptr = ret->mem;
//		ret->end = ret->mem + size;
	}
	
	return (ret);
}

shm_ipc_obj *init_shm_from_config(const char *prefix, FILE *file, bool create)
{
	char *line;
	int addr;
	int size;
	char t[128];

	sprintf(t, "%s_addr", prefix);
	line = get_first_key(file, t);
	if (!line) {
		LOG_ERR("config file wrong, no %s", t);
		return NULL;		
	}
	addr = strtol(get_value(line), NULL, 0);
	if (addr <= 0) {
		LOG_ERR("config file wrong, no %s", t);
		return NULL;
	}
    sprintf(t, "%s_size", prefix);									  
    line = get_first_key(file, t);
	if (!line) {
		LOG_ERR("config file wrong, no %s", t);
		return NULL;		
	}
	size = strtol(get_value(line), NULL, 0);
	if (size <= 0) {
		LOG_ERR("config file wrong, no %s", t);
		return NULL;
	}

	return init_shm_ipc_obj(addr, size, create);
}

void rm_shm_ipc_obj(int shmid)
{
	shmctl(shmid, IPC_RMID, NULL);	
}

PROTO_HEAD *read_from_shm_ipc(shm_ipc_obj *obj)
{
	if (obj->read == obj->write)
		return NULL;
	
	PROTO_HEAD *head = READ_DATA(obj);
	obj->read += head->len;
	return head;
}

void try_read_reset(shm_ipc_obj *obj)
{
	uint32_t t = obj->read;
	if (__atomic_compare_exchange_n(&obj->write, &t, 0, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
	{
		obj->read = 0;
		LOG_DEBUG("reset read and write");
	}
}

static void shm_ipc_move(shm_ipc_obj *obj, PROTO_HEAD *head)
{
//	int *p = (int *)&head->data[0];
//	LOG_DEBUG("%s: write = %u, read = %u, size = %u, data = %d", __FUNCTION__, obj->write, obj->read, head->len, *p);
	
	assert(obj->read == 0);
	memmove(obj->data, head, head->len);
	__atomic_store_n(&obj->write, head->len, __ATOMIC_SEQ_CST);
}

static int shm_ipc_obj_avaliable_size(shm_ipc_obj *obj)
{
	return obj->size - obj->write - SHM_DATA_OFFSET;
}

static void shm_ipc_obj_write_impl(shm_ipc_obj *obj, PROTO_HEAD *head)
{
		//如果被重置了，那么要做memmove
		//如果没有重置，随时可能被重置
//	void *addr1 = (void *)head + head->len;  //未重置地址
//	void *addr2 = obj->mem + head->len;   //重置地址
	uint32_t addr1 = obj->write + head->len;
//	uint32_t addr2 = head->len;
	for (;;)
	{
//		void *t1 = head;
//		void *t2 = obj->mem;
		uint32_t t1 = (char *)head - (char *)obj - SHM_DATA_OFFSET;
//		uint32_t t2 = 0;
		if (__atomic_compare_exchange_n(&obj->write, &t1, addr1, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		{
			return;
		}
//		else if (__atomic_compare_exchange_n(&obj->write, &t2, addr2, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		else
		{
			assert(obj->write == 0);
//			assert(obj->read == 0);
			shm_ipc_move(obj, head);
			return;
		}
//		else
//		{
//			LOG_ERR("%s: %d  it should be a bug\n", __FUNCTION__, __LINE__);
//		}
			
	}
	return;
}

static shm_ipc_write_failed write_failed_callback;
void set_shm_ipc_write_failed(shm_ipc_write_failed callback)
{
	write_failed_callback = callback;
}

int write_to_shm_ipc_start(shm_ipc_obj *obj, int len)
{
	if (shm_ipc_obj_avaliable_size(obj) < len)
	{
		if (write_failed_callback)
			return write_failed_callback(obj, NULL, len);
		return -10;
	}
	return (0);	
}
int write_to_shm_ipc_end(shm_ipc_obj *obj, int len)
{
	assert(shm_ipc_obj_avaliable_size(obj) >= len);
	PROTO_HEAD *head = WRITE_DATA(obj);
	assert((int)head->len == len);

	shm_ipc_obj_write_impl(obj, head);	
	return (0);
}

int write_to_shm_ipc(shm_ipc_obj *obj, PROTO_HEAD *head)
{
	if (shm_ipc_obj_avaliable_size(obj) < (int)head->len)
	{
		if (write_failed_callback)
			return write_failed_callback(obj, head, head->len);
		return -10;
	}

	PROTO_HEAD *shm_head = WRITE_DATA(obj);
	memcpy(shm_head, head, head->len);

	shm_ipc_obj_write_impl(obj, shm_head);
	return (0);
}

