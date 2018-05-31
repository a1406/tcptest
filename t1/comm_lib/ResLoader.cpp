#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include "ResConv.h"
#include "ResLoader.h"
#include "tea.h"

#define TRES_FILE_MAGIC 1397052237

#define RES_HEAD_ALL_META_NAME "TResHeadAll"

#define RES_HEAD_SIZE sizeof(TRESHEADALL)
#define RES_HEAD_V5_SIZE sizeof(TRESHEAD)
#define RES_HEAD_V4_SIZE offsetof(TRESHEAD,szContentHash)
#define RES_HEAD_V3_SIZE  offsetof(TRESHEAD,tCreateTime)

typedef struct _file_data
{
	unsigned long size;
	unsigned char *buf;
} FILE_DATA;

static int getDataFromFile(const char *path, FILE_DATA *ret)
{
	int fd;
	struct stat stat_buf;
	fd = open(path, O_RDONLY);
	if (fd < 0) {
		printf("open file %s fail, err = %d", path, errno);
		return (-1);
	}
	fstat(fd, &stat_buf);
	ret->size = stat_buf.st_size;
	ret->buf = (unsigned char *)malloc(ret->size);
	if (!ret->buf)
		return (-2);
	if (read(fd, ret->buf, ret->size) != (ssize_t)(ret->size))
		return (-3);

	for (size_t i = 0; i < ret->size / 8; ++i) {
		sg_decrypt((uint32_t *)&ret->buf[i * 8]);
	}

	return (0);
}

void freeFileData(FILE_DATA *data)
{
	free(data->buf);
}

int trl_check_head(LPTRESHEADALL pstHead, int iData)
{
	assert(pstHead);

	if (TRES_FILE_MAGIC != pstHead->stResHead.iMagic)
	{
		printf("error: TRES_FILE_MAGIC(%d) != pstHead->stResHead.iMagic(%d)\n",TRES_FILE_MAGIC,pstHead->stResHead.iMagic);
		return -1;
	}


	/* if (TDR_METALIB_RESCONV_VERSION != pstHead->iVersion)
	 *         return -1;*/

	if (pstHead->stResHead.iUnit <= 0)
	{
		printf("error: pstHead->stResHead.iUnit(%d) <= 0\n",pstHead->stResHead.iUnit);
		return -1;
	}

	if (pstHead->stResHead.iCount < 0)
	{
		printf("pstHead->stResHead.iCount(%d) < 0\n",pstHead->stResHead.iCount);
		return -1;
	}


	if(pstHead->stResHead.iVersion == 3)
	{
		if ((int) (RES_HEAD_V3_SIZE + (pstHead->stResHead.iUnit) * (pstHead->stResHead.iCount))
			!= iData)
		{
			printf("error: RES_HEAD_V3_SIZE(%d) + pstHead->stResHead.iUnit(%d) * pstHead->stResHead.iCount(%d)\
            != iData(%d)\r\n",(int)RES_HEAD_V3_SIZE, pstHead->stResHead.iUnit,pstHead->stResHead.iCount, iData);

			return -1;
		}
	}

	else if (pstHead->stResHead.iVersion == 4)
	{
		if ((int) (RES_HEAD_V4_SIZE + (pstHead->stResHead.iUnit) * (pstHead->stResHead.iCount))
			!= iData)
		{
			printf("error: RES_HEAD_V4_SIZE(%d) + pstHead->stResHead.iUnit(%d) * pstHead->stResHead.iCount(%d)\
            != iData(%d)\r\n",(int)RES_HEAD_V4_SIZE, pstHead->stResHead.iUnit,pstHead->stResHead.iCount, iData);

			return -1;
		}
	}
	else if (pstHead->stResHead.iVersion == 5)
	{

		if ((int) (RES_HEAD_V5_SIZE + (pstHead->stResHead.iUnit) * (pstHead->stResHead.iCount))
			!= iData)
		{\
			printf("error: RES_HEAD_V5_SIZE(%d) + pstHead->stResHead.iUnit(%d) * pstHead->stResHead.iCount(%d)\
            != iData(%d)\r\n",(int)RES_HEAD_V5_SIZE, pstHead->stResHead.iUnit,pstHead->stResHead.iCount, iData);

			return -1;
		}

	}
	else if(pstHead->stResHead.iVersion == 6)
	{
		if ((int) (pstHead->stResHeadExt.iDataOffset + (pstHead->stResHead.iUnit) * (pstHead->stResHead.iCount))
			!= iData)
		{
			printf("error: pstHead->stResHeadExt.iDataOffset(%d) + pstHead->stResHead.iUnit(%d) * pstHead->stResHead.iCount(%d)\
            != iData(%d)\r\n",pstHead->stResHeadExt.iDataOffset, pstHead->stResHead.iUnit,pstHead->stResHead.iCount, iData);
			return -1;
		}
	}
	else if(pstHead->stResHead.iVersion == 7)
	{
		int count = (int) (pstHead->stResHeadExt.iDataOffset + pstHead->stResHeadExt.iBuff +
			(pstHead->stResHead.iUnit) * (pstHead->stResHead.iCount));
		if (count != iData && (iData - count >= 4 || iData - count < 0))
		{
			printf("error: pstHead->stResHeadExt.iDataOffset(%d) + pstHead->stResHeadExt.iBuff(%d) + pstHead->stResHead.iUnit(%d) * pstHead->stResHead.iCount(%d)\
            != iData(%d)\r\n",pstHead->stResHeadExt.iDataOffset,pstHead->stResHeadExt.iBuff, pstHead->stResHead.iUnit,pstHead->stResHead.iCount, iData);

			return -1;
		}
	}
	return 0;
}


int trl_read_resheadAll(const char *a_pszFilePath, LPTRESHEADALL a_pstResHead)
{
	//char* pszData = NULL;
	//FILE *fpFile = NULL;
	int iHeadLen = 0;

	if ((NULL == a_pstResHead) || (NULL == a_pszFilePath))
	{
		return RL_ERROR_INVALID_PARAM;
	}
	/** 申请内存 **/
	memset(a_pstResHead,0,RES_HEAD_SIZE);

	FILE_DATA d;
	getDataFromFile(a_pszFilePath, &d);
//	unsigned long iDataLen=d.size;
	unsigned char* buff=d.buf;

	/*fpFile = fopen(a_pszFilePath, "rb");
	if (NULL == fpFile)
	{
	return RL_ERROR_OPEN_FILE_FAILED;
	}*/

	memcpy(a_pstResHead,buff,RES_HEAD_V3_SIZE);
	//读取资源版本号
	/*if (1!= fread((char*)a_pstResHead,RES_HEAD_V3_SIZE, 1,fpFile))
	{
		fclose(fpFile);
		return RL_ERROR_READ_FILE_FAILED;
	}
	if (0 != fseek(fpFile,0,SEEK_SET))
	{
		fclose(fpFile);
		return RL_ERROR_READ_FILE_FAILED;
	}*/

	if (a_pstResHead->stResHead.iVersion < 4)
	{
		/*if (1!= fread((char*)a_pstResHead,RES_HEAD_V3_SIZE, 1,fpFile))
		{
			fclose(fpFile);
			return RL_ERROR_READ_FILE_FAILED;
		}*/

		a_pstResHead->stResHeadExt.iDataOffset = RES_HEAD_V3_SIZE;

	}
	else if (a_pstResHead->stResHead.iVersion == 4)
	{
		/*if (1!= fread((char*)a_pstResHead,RES_HEAD_V4_SIZE, 1,fpFile))
		{
			fclose(fpFile);
			return RL_ERROR_READ_FILE_FAILED;
		}*/

		a_pstResHead->stResHeadExt.iDataOffset = RES_HEAD_V4_SIZE;

	}
	else if (a_pstResHead->stResHead.iVersion == 5)
	{
		/*if (1!= fread((char*)a_pstResHead, RES_HEAD_V5_SIZE, 1,fpFile))
		{
			fclose(fpFile);
			return RL_ERROR_READ_FILE_FAILED;
		}*/

		a_pstResHead->stResHeadExt.iDataOffset = RES_HEAD_V5_SIZE;

	}
	else
	{
		memcpy(a_pstResHead,buff,RES_HEAD_V5_SIZE + 4);
		

		iHeadLen = a_pstResHead->stResHeadExt.iDataOffset;

		memcpy(a_pstResHead,buff,iHeadLen);
		

		//add other version set
	}
	freeFileData(&d);
	return 0;
}

int trl_read_reshead(IN const char* a_pszFilePath, IN LPTRESHEAD a_pstResHead)
{
//	FILE *fpFile = NULL;
	int iRet = 0;

	/** 头文件全部信息 **/
	TRESHEADALL stHead;
	memset(&stHead,0,RES_HEAD_SIZE);
	if ((NULL == a_pstResHead) || (NULL == a_pszFilePath))
	{
		return RL_ERROR_INVALID_PARAM;
	}
	/*获得文件长度*/
	//fpFile = fopen(a_pszFilePath, "rb");
	//if (NULL == fpFile)
	//{
	//	return RL_ERROR_OPEN_FILE_FAILED;
	//}
	//fseek(fpFile, 0, SEEK_END);
	//iDataLen = (int)ftell(fpFile);
	//fclose(fpFile);


	/*读取 头部*/
	iRet= trl_read_resheadAll(a_pszFilePath,&stHead);

	if (0 == iRet)
	{
		/*检查头部*/
			memcpy(a_pstResHead,&stHead,sizeof(TRESHEAD));
	}

	return iRet;
}


int trl_load_file(const char* pszPath, char** ppszBuff, int* piSize)
{
	FILE *fp;

	long lSize;
	int iRead;
	int iRet = 0;
	char* pszData;

	if ((NULL == pszPath) || (NULL == piSize) || (NULL == ppszBuff))
	{
		return RL_ERROR_INVALID_PARAM;
	}

	fp = fopen(pszPath, "rb");
	if (NULL == fp)
	{
		*ppszBuff = NULL;
		*piSize = 0;
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	lSize = ftell(fp);
	if (0 >= lSize)
	{
		*ppszBuff = NULL;
		*piSize = 0;
		fclose(fp);
		return -2;
	}
	fseek(fp, 0, SEEK_SET);

	*piSize = (int) lSize;
	pszData = (char*) calloc(1, (size_t)(lSize + 1));
	if (NULL == pszData)
	{
		*ppszBuff = NULL;
		*piSize = 0;
		fclose(fp);
		return -3;
	}

	iRead = fread(pszData, 1, lSize, fp);
	if (iRead == (int) lSize)
	{
		*ppszBuff = pszData;
		pszData[lSize] = '\0';
	}
	else
	{
		free(pszData);
		iRet = -4;
	}

	fclose(fp);

	return iRet;
}


/** 开始加载文件 **/
int trl_specail_load(char* pszBuff, int iBuff, int iUnit, const char* pszPath,
	IN const char *pszMetalibHash, IN int iFlag)
{
	TRESHEADALL stHead;

	unsigned char* pszData = NULL;
	unsigned char* pszSrc = NULL;
	 char* pszDst = NULL;
	unsigned long iData = 0;
	int iCount = 0;
	int iSrcUnit = 0;
	int i = 0;
	int iRet = 0;
	memset(&stHead, 0x00,sizeof(stHead));
	if (NULL == pszBuff)
	{
		return RL_ERROR_INVALID_PARAM;
	}

//	Data d=FileUtils::getInstance()->getDataFromFile(pszPath);
//	iData=d.getSize();
//	pszData=d.getBytes();
	FILE_DATA d;
	getDataFromFile(pszPath, &d);
	iData=d.size;
	pszData=d.buf;
	
	/*加载数据到内存*/
	iRet = 0;
	if (iRet < 0 || !pszData)
		return RL_ERROR_READ_FILE_FAILED; /* can not read. */


	iRet = trl_read_resheadAll(pszPath, &stHead);
	if (iRet != 0)
	{
		//free(pszData);
		return RL_ERROR_READ_FILE_FAILED;
	}

	/*检验数据是否有效*/
	if (trl_check_head(&stHead, iData) < 0)
	{
		//free(pszData);
		return RL_ERROR_CHECK_FAILE_FAILED;
	}

	/*检验元数据描述库hash值*/
	if ((NULL != pszMetalibHash) && !(iFlag & RL_FLAG_INGORE_DIFF_HASH))
	{
		if (strcmp(pszMetalibHash, stHead.stResHead.szMetalibHash))
		{
			//free(pszData);
			return RL_ERROR_DIFF_HASH;
		}
	}

	/*检查接受资源数据的缓冲区相关参数是否正确*/
	if (stHead.stResHead.iUnit < iUnit)
	{
		//free(pszData);
		return RL_ERROR_UNIT_SIZE_TOO_SMALE;
	}
	if (stHead.stResHead.iCount * iUnit + stHead.stResHeadExt.iBuff > iBuff)
	{
		//free(pszData);
		return RL_ERROR_BUFFER_SIZE_TO_SMALL;
	}

	/*拷贝数据*/
	iCount = stHead.stResHead.iCount;
	iSrcUnit = stHead.stResHead.iUnit;

	pszSrc = pszData + stHead.stResHeadExt.iDataOffset;

	pszDst = pszBuff;

	for (i = 0; i < iCount; i++)
	{
		memcpy(pszDst, pszSrc, iUnit);

		pszDst += iUnit;
		pszSrc += iSrcUnit;
	}

	if(stHead.stResHeadExt.iBuff >0)
	{
		memcpy(pszDst, pszSrc, stHead.stResHeadExt.iBuff);
	}
	//free(pszData);

	freeFileData(&d);
	return iCount;
}



