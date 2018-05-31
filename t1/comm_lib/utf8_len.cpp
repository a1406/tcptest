#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//utf8字符长度1-6，可以根据每个字符第一个字节判断整个字符长度
//0xxxxxxx
//110xxxxx 10xxxxxx
//1110xxxx 10xxxxxx 10xxxxxx
//11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
//111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
//1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
//
//定义查找表，长度256，表中的数值表示以此为起始字节的utf8字符长度
static unsigned char utf8_look_for_table[] =
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
};

#define UTFLEN(x)  utf8_look_for_table[(x)]

//计算str字符数目
int getUtf8Length(char *str)
{
	int clen = strlen(str);
	int len = 0;
	char *ptr;

	for(ptr = str;
		*ptr!=0 && len<clen;
		len++, ptr+=UTFLEN((unsigned char)*ptr));

	return len;
}

//ascii算一个字节，其他算两个字节
int getContentLen(char *str)
{
	int clen = strlen(str);
	int len = 0;
	char *ptr;
	int t;

	for(ptr = str;
		*ptr!=0 && len<clen;)
	{
		t = UTFLEN((unsigned char)*ptr);
		if (t == 1)
			len += 1;
		else
			len += 2;
		ptr += t;
	
	}
	return len;
}

//get子串
char* subUtfString(char *str, unsigned int start, unsigned int end)
{
	unsigned int len = getUtf8Length(str);
	char *sptr = str;
	char *eptr = sptr;
	unsigned int i;
	int retLen;
	char *retStr;

	if(start >= len) return NULL;
	if(end > len) end = len;

	for(i = 0; i < start; ++i,sptr+=UTFLEN((unsigned char)*sptr));

	for(i = start; i < end; ++i,eptr += UTFLEN((unsigned char)*eptr));

	retLen = eptr - sptr;
	retStr = (char*)malloc(retLen+1);
	memcpy(retStr, sptr, retLen);
	retStr[retLen] = 0;

	return retStr;
}

// int main()
// {
// 	char *str = "我，的a,测试工具阿斯顿aaab123阿斯顿个流氓了卡斯！";
// 	char *ptr;
	
// 	printf("%s\n", str);

// 	for(ptr=str; *ptr!=0;) {
// 		unsigned char c = (unsigned char)*ptr;
// 		printf("str[%d] is a word character with %d bytes\n", c, UTFLEN(c));
// 		ptr += UTFLEN(c);
// 	}

// 	printf("%d\n", getUtf8Length(str));

// 	char *sub = subUtfString(str, 2, 100);
// 	if(sub) {
// 		printf("%s\n", sub);
// 		free(sub);
// 		sub = NULL;
// 	}

// 	return 0;
// }
