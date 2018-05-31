#ifndef _STR_COPY_H__
#define _STR_COPY_H__

#include <string.h>

#define safe_strcpy(dst, src) \
	do {					  \
	strncpy(dst, src, sizeof(dst));				\
	dst[sizeof(dst) - 1] = '\0';					\
	} while (0)

#define safe_strcpy1(dst, dst_len, src)		\
	do {					  \
	strncpy(dst, src, dst_len);				\
	dst[dst_len - 1] = '\0';					\
	} while (0)


#endif
