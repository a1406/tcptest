#ifndef _COMM_H__
#define _COMM_H__

#ifndef container_of
#define container_of(ptr, type, member) ({            \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#ifndef likely
#define likely(x)	__builtin_expect((x),1)
#define unlikely(x)	__builtin_expect((x),0)
#endif

#ifdef FloatMin
#undef FloatMin
#endif // FloatMin

#define FloatMin(a,b)            ((a-b > 0.000001) ? (b) : (a))

#ifdef FloatMax
#undef FloatMax
#endif // FloatMax
#define FloatMax(a,b)			 ((a-b > 0.000001) ? (a) : (b))	


#endif
