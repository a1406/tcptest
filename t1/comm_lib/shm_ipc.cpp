#include "shm_ipc.h"
#include "game_event.h"

shm_ipc_obj *init_shm_ipc_obj(int key, int size, bool create)
{
	int flag = 0666;
	if (create)
	{
		flag |= IPC_CREAT|IPC_EXCL;
	}
	int shmid = shmget(key, size, flag);
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
	int *p = (int *)&head->data[0];
	LOG_DEBUG("%s: write = %u, read = %u, size = %u, data = %d", __FUNCTION__, obj->write, obj->read, head->len, *p);
	
	assert(obj->read == 0);
	memmove(obj->data, head, head->len);
	__atomic_store_n(&obj->write, head->len, __ATOMIC_SEQ_CST);
}

static int shm_ipc_obj_avaliable_size(shm_ipc_obj *obj)
{
	return obj->size - obj->write - SHM_DATA_OFFSET;
}

int write_to_shm_ipc(shm_ipc_obj *obj, PROTO_HEAD *head)
{
	assert(shm_ipc_obj_avaliable_size(obj) >= (int)head->len);

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
			return (0);
		}
//		else if (__atomic_compare_exchange_n(&obj->write, &t2, addr2, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		else
		{
			assert(obj->write == 0);
//			assert(obj->read == 0);
			shm_ipc_move(obj, head);
			return (0);
		}
//		else
//		{
//			LOG_ERR("%s: %d  it should be a bug\n", __FUNCTION__, __LINE__);
//		}
			
	}
	return (0);
}

