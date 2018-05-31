#ifndef MINHEAP_H
#define MINHEAP_H

#include <stdbool.h>

typedef bool (*minheap_cmp)(void *a, void *b);
typedef int (*minheap_getindex)(void *a);
typedef void (*minheap_setindex)(int index, void *a);	

struct minheap
{
	unsigned max_size;
	unsigned cur_size;
	void **nodes;
	minheap_cmp cmp;
	minheap_getindex get;
	minheap_setindex set;
};

int init_heap(struct minheap *heap, int max_size, minheap_cmp cmp, minheap_getindex get, minheap_setindex set);
int get_node_index(struct minheap *heap, void *node);

int push_heap(struct minheap *heap, void *node);
void *get_heap_first(struct minheap *heap);
void *pop_heap(struct minheap *heap);
int adjust_heap_node(struct minheap *heap, void *node);
int erase_heap_node(struct minheap* heap, void *node);
bool is_node_in_heap(struct minheap* heap, void *node);
#endif /* MINHEAP_H */
