#ifndef UTF8_LEN_H
#define UTF8_LEN_H

//计算str字符数目
int getUtf8Length(char *str);

//ascii算一个字符，其他算两个字符
int getContentLen(char *str);

//get子串
char* subUtfString(char *str, unsigned int start, unsigned int end);

#endif /* UTF8_LEN_H */
