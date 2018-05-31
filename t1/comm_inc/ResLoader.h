#ifndef __RESLOADER_H__
#define __RESLOADER_H__
/****************
 *策划配置数据 bin 加载解析
 *@author:Georgehu
 *@since:2013-12-01
 *@example:
 int arrCount = MAX_NUM;
 ResMonster astcfg[MAX_NUM];

 int iRet = LoadTemplateCfg<ResMonster,MAX_NUM>(CCFileUtils::sharedFileUtils()->fullPathForFilename("cfg/bin/Monster.bin").c_str(),astcfg,&arrCount );
 if(iRet<0){
 CCLOG("load cfg failed:%d.\n",iRet);

 }
*******************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ResConv.h"

#define RL_MAX_PATH_LEN		256

#define RL_LOADMODE_XMLV1	1		/*读取嵌套结构体以类型名标识的xml资源文件*/
#define RL_LOADMODE_XMLV2	2		/*读取嵌套结构体以成员名标识的xml资源文件*/
#define RL_LOADMODE_XMLV3	3		/*读取嵌套结构体以属性值标识的xml资源文件*/
#define RL_LOADMODE_BIN		4		/*读取二进制数据格式的资源文件*/

#ifndef RES_ID_ARRAY
#define RES_ID_ARRAY	1
#endif

/** @name TRC_LOADR错误ID
*  @{
*/
#define RL_ERROR_OPEN_FILE_FAILED		-1		/*打开文件读失败*/
#define RL_ERROR_READ_FILE_FAILED		-2		/*从文件读失败*/
#define RL_ERROR_CHECK_FAILE_FAILED		-3		/*检验资源文件失败，可能资源文件的版本，魔数与本接口定义的不一致*/
#define RL_ERROR_NO_MEMORY				-4		/*分配内存失败*/
#define RL_ERROR_UNIT_SIZE_TOO_SMALE		-5		/*指定的单个资源存储空间比预期的要小*/
#define RL_ERROR_BUFFER_SIZE_TO_SMALL	-6		/*加载资源的数据区空间不够*/
#define RL_ERROR_DIFF_HASH 	-7		/*资源文件中记录的DR元数据库hash值与当前指定的hash值不同，说明资源结构体可能已经变更*/
#define RL_ERROR_READ_XML_FILE_FAILED -8	/*读取xml文件失败*/
#define RL_ERROR_FAILED_TO_GET_HEAD_META -9	/*获取TResHead元数据描述句柄失败*/
#define RL_ERROR_FAILED_TO_READ_XRESHEAD -10	/*从xml中读取TResHead失败*/
#define RL_ERROR_INVALID_PARAM -11	/*传入接口的参数无效*/
/**   @}*/

/** @name TRC_LOADR资源加载标识位
*  @{
*/
#define RL_FLAG_INGORE_DIFF_HASH		0x00000001		/*忽略资源文件中元数据描述库的hash值与当前hash值的不同，继续加载资源*/
/**   @}*/




/*表示该参数只是输入参数*/
#ifndef IN
#define IN  
#endif


/*表示该参数只是输出参数*/
#ifndef OUT
#define OUT
#endif


/*表示该参数既是输入参数，又是输出参数*/
#ifndef INOUT
#define INOUT
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/** @defgroup TRC_LOADR_VFILE 虚拟文件系统支持接口
*  @{
*/
/** @defgroup TRC_LOADR_BIN 二进制格式的资源文件加载接口
*  @{
*/

/*TRC_LOADR_BIN 二进制资源文件加载接口*/

/** 指定保存资源数据的缓冲大小并指定单个资源数据的内存大小,从资源文件中将资源大小缓冲区中，缓冲区的空间在函数外分配
*@param[in] pszBuff 保存数据的缓冲区首地址
*@param[in] iBuff	缓冲区的可用字节数
*@param[in] iUnit	指定单个资源数据结构体的在内存中的存储空间，通过这个参数调用者可以为每个资源分配比实际需要存储空间更大的空间。
如果此参数的值为0，则单个资源信息结构体的存储空间为实际所需空间
*@param[in] pszFilePath	资源文件名
*@param[in] pszMetalibHash 此资源文件中资源结构对应DR元数据描述库的hash值
*@param[in] iFlag	资源加载控制标识位，此参数可以为0或下列标识位的位'与'值:
*	-	RL_FLAG_INGORE_DIFF_HASH	忽略元数据描述库hash的不同
*
*note	保存资源信息的缓冲区必须调用trl_unload进行释放
*note 目前资源文件的头部记录了生成资源文件时此资源结构体对应应DR
*	元数据库的hash值。pszMetalibHash参数传入应用当前使用的资源结构DR 元数据库
*	的hash值，通过检测hash值是否相同，可以判断资源文件中资源结构的版本
*	与当前结构的版本是否相同；如果资源结构版本不同，则加载资源
*	失败，这样能避免因资源结构体变更而加载错误的资源
*note	如果pszMetalibHash参数置为NULL, 则不检测元数据库的hash值，这时不能检测
*	资源文件中结构体与当前使用的资源结构是否一致
*note 	当前资源结构DR 元数据描述hash值的获取途径有:
*	-	通过使用md5sum工具利用当前二进制DR元数据库文件生成
*	-	利用tdr工具，将资源结构描述转换成.h头文件时，会将此资源结构
*		DR元数据库的hash值写道头文件中，
*
*note 如果使用虚拟文件来存储存储资源文件，则调用本接口之前必须调用trl_set_fileio_interface，以
*	设置文件IO操作接口
*@see trl_set_fileio_interface
*@retval 正数  读取的资源个数
*@retval 0	文件中没有资源
*@retval 负数， 加载资源失败，可能返回值和错误原因：
*	－ RL_ERROR_OPEN_FILE_FAILED 打开文件读失败
*	-  RL_ERROR_READ_FILE_FAILED	从文件读失败
*	- RL_ERROR_CHECK_FAILE_FAILED		检验资源文件失败，可能资源文件的版本，魔数与本接口定义的不一致
*	- RL_ERROR_NO_MEMORY				分配内存失败
*	- RL_ERROR_DIFF_HASH	资源结构元数据描述库hash不同
*/
int trl_specail_load(IN char* pszBuff, IN int iBuff, IN int iUnit, IN const char* pszFilePath, 
					IN const char *pszMetalibHash, IN int iFlag);

/** 从二进行资源文件中读取出资源信息头部
*@param[in] a_pszFilePath	资源文件名
*@param[in] a_pstResHead 保存资源信息头部信息的结构体指针
*
*note 如果使用虚拟文件来存储存储资源文件，则调用本接口之前必须调用trl_set_fileio_interface，以
*	设置文件IO操作接口
*@see trl_set_fileio_interface
*retval 0	成功
*@retval 负数  失败,可能返回值和错误原因：
*	－ RL_ERROR_OPEN_FILE_FAILED 打开文件读失败
*	-  RL_ERROR_READ_FILE_FAILED	从文件读失败
*	- RL_ERROR_CHECK_FAILE_FAILED		检验资源文件失败，可能资源文件的版本，魔数与本接口定义的不一致
*/
int trl_read_reshead(IN const char* a_pszFilePath, IN LPTRESHEAD a_pstResHead);


/**  @}*/
#ifdef __cplusplus
}
#endif

//template <typename T, int N>
//int LoadTemplateCfg(const char * pCfgFilePath, T (&astTArrar)[N], int *piNum)
template <typename T>
int LoadTemplateCfg(const char * pCfgFilePath, T** astTArrar, int *piNum)
{
    TRESHEAD stHead;

    int iRet;
   
  
	/** 申请内存大小 **/
 //   memset(astTArrar, 0, sizeof(astTArrar));
	if (!astTArrar || !piNum)
		return -1;

	 *piNum = 0;

    /** 读取文件头信息 **/
    iRet = trl_read_reshead(pCfgFilePath, &stHead);
    if (iRet < 0)
    {        
        return iRet;
    }

	/*
    if (stHead.iCount > N)
    {
        return -5;
    }
	*/

	*astTArrar = new T [stHead.iCount];
	if (*astTArrar == NULL)
		return -2;

    iRet = trl_specail_load((char*)(*astTArrar), stHead.iCount * sizeof(T), sizeof(T),
        pCfgFilePath, 0, RL_FLAG_INGORE_DIFF_HASH);
    if (iRet < 0)
    {
        return -6;
    }

    *piNum = stHead.iCount;

    return 0;
}

#endif /* __RESLOADER_H__ */
