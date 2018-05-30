#ifndef SHM_IPC_H
#define SHM_IPC_H

#include <assert.h>
#include "server_proto.h"
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct SHM_IPC_OBJ
{
	uint32_t size;
	uint32_t read;
	uint32_t write;
//	void *mem;
//	void *end;
//	int shmid;
//	void *read_ptr;
//	void *write_ptr;
	char data[0];
} shm_ipc_obj;
#define SHM_DATA_OFFSET sizeof(struct SHM_IPC_OBJ)
#define READ_DATA(obj) (PROTO_HEAD *)(obj->data + obj->read)
#define WRITE_DATA(obj) (PROTO_HEAD *)(obj->data + obj->write)

//shm_ipc_obj *init_shm_ipc_obj(int key, int size, bool create);
//void rm_shm_ipc_obj(int shmid);

shm_ipc_obj *init_shm_from_config(const char *prefix, FILE *file);

PROTO_HEAD *read_from_shm_ipc(shm_ipc_obj *obj);
void try_read_reset(shm_ipc_obj *obj);

int write_to_shm_ipc(shm_ipc_obj *obj, PROTO_HEAD *head);
#endif /* SHM_IPC_H */
