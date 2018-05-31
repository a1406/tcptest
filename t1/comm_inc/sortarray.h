#ifndef _SORTARRAY_H__
#define _SORTARRAY_H__
#include <stdint.h>

typedef int (*comp_func)(const void *, const void *);

//在排序数组里面做二分查找
//key:  关键字
//base: 数组起始点
//nmemb: 数组元素个数
//size: 数组单个元素长度
//find：返回是否找到
//compar： 比较关键字的函数
//返回值: 返回查找到的元素在数组中的下标.如果没有查找到，返回应该插入的位置。
//        也可以简单的认为，返回的就是新数据要插入的位置，但是如果已经存在有同样大小的key，
//        不保证插入位置在原有key的前面或后面，如果有需要，得自己计算
//        返回值取值范围在[0, nmemb], 也就是说，如果插入的key比原有数组的key都大，那么返回
//        的位置是数组最大长度的后一个位置, 这里需要小心溢出
int array_bsearch (const void *key, const void *base, int nmemb, int size, int *find, comp_func func);
int array_delete_index (const void *base, int *pnmemb, int size, int index);
int array_delete (const void *key, const void *base, int *pnmemb, int size, comp_func func);
int array_insert (const void *key, const void *base, int *pnmemb, int size, int unique, comp_func func);

int comp_int64(const void *a, const void *b);
int comp_uint64(const void *a, const void *b);
int comp_int32(const void *a, const void *b);
int comp_uint32(const void *a, const void *b);

#endif
