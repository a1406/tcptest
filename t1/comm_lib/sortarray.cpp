#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "sortarray.h"

int comp_uint32(const void *a, const void *b)
{
	if (*(uint32_t *)a == *(uint32_t *)b)
		return (0);
	if (*(uint32_t *)a > *(uint32_t *)b)
		return (1);
	return (-1);
}

int comp_int32(const void *a, const void *b)
{
	if (*(int32_t *)a == *(int32_t *)b)
		return (0);
	if (*(int32_t *)a > *(int32_t *)b)
		return (1);
	return (-1);
}

int comp_uint64(const void *a, const void *b)
{
	if (*(uint64_t *)a == *(uint64_t *)b)
		return (0);
	if (*(uint64_t *)a > *(uint64_t *)b)
		return (1);
	return (-1);
}

int comp_int64(const void *a, const void *b)
{
	if (*(int64_t *)a == *(int64_t *)b)
		return (0);
	if (*(int64_t *)a > *(int64_t *)b)
		return (1);
	return (-1);
}

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
int array_bsearch (const void *key, const void *base, int nmemb, int size, int *find, int (*compar) (const void *, const void *))
{
	int l, u, idx;
	const void *p, *p2;
	int comparison, comparison2;

	*find = 0;
	if (!nmemb) return 0;
	l = 0;
	u = nmemb;

	while (l < u)
	{
		idx = (l + u) / 2;
		p = (void *) (((const char *) base) + (idx * size));
		comparison = (*compar) (key, p);

		if (comparison == 0)
		{
			*find = 1;
			return idx;
		}
		else if (comparison < 0)
		{
			if (idx == 0) return idx;

			p2 = (void *) (((const char *) base) + ((idx - 1) * size));
			comparison2 = (*compar) (key, p2);

			if (comparison2 > 0) return idx;

			u = idx;
		}
		else /*if (comparison > 0)*/
		{
			l = idx + 1;
		}
	}

	return u;
}

int array_delete_index (const void *base, int *pnmemb, int size, int index)
{
	if (0 > index || index >= *pnmemb)
		return -1;
	memmove((char*)base + index * size, (char *)base + (index + 1)*size,  (*pnmemb - index - 1)*size);
	(*pnmemb)--;
	return 0;
}

int array_delete (const void *key, const void *base, int *pnmemb, int size, int (*compar) (const void *, const void *))
{
	int index, iEqu;

	index = array_bsearch(key, base, *pnmemb, size, &iEqu, compar);
	if (!iEqu) return -1;
	return array_delete_index(base, pnmemb, size, index);
}

int array_insert (const void *key, const void *base, int *pnmemb, int size, int unique, int (*compar) (const void *, const void *))
{
	int index, find;

	index = array_bsearch(key, base, *pnmemb, size, &find, compar);

	if (find && unique)
	{
		return -1;
	}

	if (index < (*pnmemb))
	{
		memmove((char*)base + (index+1)*size, (char*)base + index*size, (*pnmemb-index)*size);
	}

	memcpy((char*)base + index* size, key, size);
	++(*pnmemb);

	return index;

}
