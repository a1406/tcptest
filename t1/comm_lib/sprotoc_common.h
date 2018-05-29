#ifndef _SPROTO_COMMON_H__
#define _SPROTO_COMMON_H__

#define SPROTO_TARRAY 0x80
#define CHUNK_SIZE 1000
#define SIZEOF_LENGTH 4
#define SIZEOF_HEADER 2
#define SIZEOF_FIELD 2

struct field {
	int tag;
	int type;
	const char * name;
	struct sproto_type * st;
	int key;
	int offset;
};

struct sproto_type {
	const char * name;
	int n;
	int base;
	int maxn;
	struct field *f;
	int c_size;
};

struct protocol {
	const char *name;
	int tag;
	struct sproto_type * p[2];
};

struct chunk {
	struct chunk * next;
};

struct pool {
	struct chunk * header;
	struct chunk * current;
	int current_used;
};

struct sproto {
	struct pool memory;
	int type_n;
	int protocol_n;
	struct sproto_type * type;
	struct protocol * proto;
};

void pool_init(struct pool *p);
void pool_release(struct pool *p);
int sprotoc_encode(const struct sproto_arg *args);

#endif
