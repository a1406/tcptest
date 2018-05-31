#ifndef _MEM_POOL_H__
#define _MEM_POOL_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct comm_pool
{
	uint64_t map1;
	uint64_t map2[64];
	uint64_t map3[64][64];

	int size;
	int num;
	char *data;
};
void *comm_pool_alloc(struct comm_pool *pool);
int comm_pool_free(struct comm_pool *pool, void *data);
int init_comm_pool(int resume, int data_size, int data_num, unsigned long key, struct comm_pool *ret);
//int count_init_comm_pool_size(int data_size, int data_num);
void *get_next_inuse_comm_pool_entry(struct comm_pool *pool, int *index);
int get_inuse_comm_pool_num(struct comm_pool *pool);

void *alloc_shared_mem(int resume, unsigned long key, int size);

struct mass_pool
{
	uint64_t map1;
	uint64_t map2[64];
	uint64_t map3[64][64];
	uint64_t map4[64][64][64];	

	int size;
	int num;
	char *data;
};

void *mass_pool_alloc(struct mass_pool *pool);
int mass_pool_free(struct mass_pool *pool, void *data);
int init_mass_pool(int resume, int data_size, int data_num, unsigned long key, struct mass_pool *ret);
void *get_next_inuse_mass_pool_entry(struct mass_pool *pool, int *index);
int count_init_mass_pool_size(int data_size, int data_num);

#endif
