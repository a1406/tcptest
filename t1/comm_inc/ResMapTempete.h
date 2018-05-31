#ifndef __ResMapTempete_h__
#define __ResMapTempete_h__
//#include "GCommon.h"
#include <map>
#include <assert.h>
#include <set>
#include "ResLoader.h"

using namespace std;

//template< typename __ResType__,int maxCount>
template< typename __ResType__>
class ResMapTempete
{
protected:
	//typename std::map<int , __ResType__*> ResMap;
	//typedef std::map<int ,int> ResMap;
    typename std::map<int , __ResType__*>  m_mapData;

public:	
	typedef typename std::map<int , __ResType__*>::iterator iterator;

	ResMapTempete() 
		: resArrData(NULL){}

	virtual ~ResMapTempete(){
		clearData();
	};

	void clearData()
	{
		m_mapData.clear();
		if (resArrData) {
			delete []resArrData;
			resArrData= NULL;
		}
		resArrDataUseLen = 0;
	};

	int loadData(const char* url){
		//int count=0;
		clearData();
		int iRet = LoadTemplateCfg<__ResType__>(url, &resArrData, &resArrDataUseLen);
		if(iRet<0){
			printf("load cfg[%s] failed:%d.\n", url, iRet);
			return iRet;
		}
		for(int i=0;i<resArrDataUseLen;i++){
	//		assert(m_mapData.find(resArrData[i].iID) == m_mapData.end());
			m_mapData.insert(make_pair(resArrData[i].iID,&resArrData[i]));
		}
		return 0;
	};

	int loadData(__ResType__* data, int len)
	{
		clearData();
		resArrData = data;
		resArrDataUseLen = len;
		for(int i=0;i<resArrDataUseLen;i++){
	//		assert(m_mapData.find(resArrData[i].iID) == m_mapData.end());
			m_mapData.insert(make_pair(resArrData[i].iID,&resArrData[i]));
		}
		return (0);
	}

	__ResType__* getDataById(int id){

		typename std::map<int , __ResType__*>::iterator ptr=m_mapData.find(id);

		if(ptr!=m_mapData.end()){
			return ptr->second;
		}
		return NULL;
	};

	iterator begin() {
		return m_mapData.begin();
	}

	iterator end() {
		return m_mapData.end();
	}

	int MaxKey() {
		typename std::map<int , __ResType__*>::reverse_iterator it = m_mapData.rbegin();
		if (it!=m_mapData.rend())
			return it->first;

		return 0;
	}

//	__ResType__  resArrData[maxCount];
	__ResType__* resArrData;
	int			 resArrDataUseLen;
private:
};

//template<typename __ResType__,int maxCount>
template<typename __ResType__>
class ResMultiMapTempete{
private:

    typedef typename std::multimap<int , __ResType__*> ResMap;
	//typedef std::map<int ,int> ResMap;
    std::multimap<int , __ResType__*>  m_mapData;

public:	
	typedef typename ResMap::iterator iterator;

	ResMultiMapTempete()
	: resArrData (NULL) {

	}

	virtual ~ResMultiMapTempete(){
		//delete m_resArr;
		m_mapData.clear();

		if (resArrData) {
			delete []resArrData;
			resArrData = NULL;
		}
	};

	int loadData(const char* url){
		//int count=0;
		int iRet = LoadTemplateCfg<__ResType__>(url, &resArrData,&resArrDataUseLen);
		if(iRet<0){
			printf("load cfg[%s] failed:%d.\n", url, iRet);
			return iRet;
		}
		for(int i=0;i<resArrDataUseLen;i++){
			m_mapData.insert(make_pair(resArrData[i].iID,&resArrData[i]));
		}
		return 0;
	};

	__ResType__* getDataByIdAndLevel(int id, int level){

		typename std::multimap<int , __ResType__*>::iterator ptr=m_mapData.find(id);

		while (ptr != m_mapData.end()) {
			if (ptr->second->iLevel == level)
				return ptr->second;
			++ptr;
		}
		return NULL;
	};

	__ResType__* getDataByIdAndItemID(int id, int itemId){

		typename std::multimap<int , __ResType__*>::iterator ptr=m_mapData.find(id);

		while (ptr != m_mapData.end()) {
			if (ptr->second->iItemID == itemId)
				return ptr->second;
			++ptr;
		}
		return NULL;
	};

	__ResType__ * getDataByIdAndItemIDAndLevel(int id, int itemId, int level)
	{
		typename std::multimap<int , __ResType__*>::iterator ptr=m_mapData.find(id);

		while (ptr != m_mapData.end()) {
			if (ptr->second->iID == id && ptr->second->iItemID == itemId && ptr->second->iLevel == level)
				return ptr->second;
			++ptr;
		}
		return NULL;
	}

	__ResType__ * getDataByIdAndQualityAndStrength(int id, int quality, int level)
	{
		typename std::multimap<int , __ResType__*>::iterator ptr=m_mapData.find(id);

		while (ptr != m_mapData.end()) {
			if (ptr->second->iID == id && ptr->second->iQuality == quality && ptr->second->iStrengthenLevel == level)
				return ptr->second;
			++ptr;
		}
		return NULL;
	}

	__ResType__ * getDataByIdAndBadgeIdAndBadgeLevel(int id, int badgeId, int badgeLevel)
	{
		typename std::multimap<int , __ResType__*>::iterator ptr=m_mapData.find(id);

		while (ptr != m_mapData.end()) {
			if (ptr->second->iID == id && ptr->second->iBadge == badgeId && ptr->second->iBadgeLevel == badgeLevel)
				return ptr->second;
			++ptr;
		}
		return NULL;
	}

	bool exist(int id) {
		typename std::multimap<int , __ResType__*>::iterator ptr=m_mapData.find(id);
		return ptr != m_mapData.end();
	}

	iterator begin() {
		return m_mapData.begin();
	}

	iterator end() {
		return m_mapData.end();
	}

//	__ResType__  resArrData[maxCount];
	__ResType__* resArrData;
	int			 resArrDataUseLen;
private:
};


//template< typename __ResType__,int maxCount>
template< typename __ResType__>
class ResSkillMapTemplate
{
public:
	typedef __ResType__ ResType;
	/// map<id, ResType> ResValMap
	typedef std::map<uint32_t,  ResType*> ResValMap;
	/// map<品阶, ResValMap> QualityMap
	typedef std::map<uint32_t, ResValMap> QualityMap;
	/// map<类型, QualityMap> StMap
	typedef std::map<uint32_t, QualityMap > StMap;
public:
	typedef typename StMap::iterator iterator;
public:
	ResSkillMapTemplate()
	 : resArrData(NULL){}

	~ResSkillMapTemplate() {
		m_mapData.clear();
		if (resArrData)
		{
			delete []resArrData;
			resArrData = NULL;
		}
	}

	static uint32_t getSkillType(uint32_t id) {
		return id/1000; /// 除1000表示同一技能
	}

	static uint32_t getSkillLevel(uint32_t id) {
		return id%100;
	}

public:
	int loadData(const char* url){
		int iRet = LoadTemplateCfg<__ResType__>(url, &resArrData, &resArrDataUseLen);
		if(iRet<0){
			printf("load cfg[%s] failed:%d.\n", url, iRet);
			return iRet;
		}

		for(int i=0;i<resArrDataUseLen;i++){
			uint32_t skType = getSkillType(resArrData[i].iID);
			m_mapData[skType][resArrData[i].iSkillClass].insert(std::make_pair(resArrData[i].iID, &resArrData[i]));
		}

		return 0;
	};

	ResType* getDataById(uint32_t id) {
		uint32_t skType = getSkillType(id);
		typename StMap::iterator it = m_mapData.find(skType);
		if (it == m_mapData.end()) 
			return NULL;

		typename QualityMap::iterator qit = it->second.begin();
		for (; qit!=it->second.end(); ++qit) {
			typename ResValMap::iterator rit = qit->second.begin();
			for (; rit!=qit->second.end(); ++rit) {
				if (rit->first == id) {
					return rit->second;
				}
			}
		}

		return NULL;
	}

	/// 是否最大品阶; 返回值: <0: 出错; =0: 否; >0: 是
	int isMaxQuality(uint32_t id) {
		uint32_t skType = getSkillType(id);
		uint32_t quality = 0;
		typename StMap::iterator it = m_mapData.find(skType);
		if (it == m_mapData.end()) 
			return -1;

		if (quality==0) {
			ResType* r = getDataById(id);
			if (NULL == r) 
				return -1;

			quality = r->iSkillClass;
		}

		typename QualityMap::iterator qit = it->second.find(quality);
		if (qit==it->second.end())
			return -1;

		return (++qit)==it->second.end() ? 1 : 0;
	}

	/// 是否当前品质下最大等级; 返回值: <0: 出错; =0: 否; >0: 是
	int isMaxLevel(uint32_t id) {
		ResType* r = getDataById(id);
		if (NULL == r) 
			return -1;

		uint32_t skType = getSkillType(id);
		typename StMap::iterator it = m_mapData.find(skType);
		if (it == m_mapData.end()) 
			return -1;


		typename QualityMap::iterator qit = it->second.find(r->iSkillClass);
		if (qit == it->second.end()) {
			return -1;
		}

		typename ResValMap::iterator rit = qit->second.find(id);
		if (rit == qit->second.end())
			return -1;

		return (++rit) == qit->second.end() ? 1 : 0;
	}

	iterator begin() {
		return m_mapData.begin();
	}

	iterator end() {
		return m_mapData.end();
	}


public:
//	ResType  resArrData[maxCount];
	ResType* resArrData;
	int		 resArrDataUseLen;
	StMap m_mapData;
};


//template< typename __ResType__, int maxCount>
template< typename __ResType__>
struct ResMapJobTempete : public ResMapTempete<__ResType__>
{
	using ResMapTempete<__ResType__>::m_mapData;
	typedef typename ResMapTempete<__ResType__>::iterator iterator;

	__ResType__* getDataByJob(int32_t job){
		typename std::map<int , __ResType__*>::iterator ptr=m_mapData.begin();
		for (; ptr!=m_mapData.end(); ++ptr ) {
			if (ptr->second->iJobLimit == job)
				return ptr->second;
		}

		return NULL;
	}
};


#if 0
template<typename __ResType__,int maxCount>
class CMachaiMapTempete {
	/// map<id%100, map<star, std::map<id,__ResType__*> > >
	typedef typename std::map<uint32_t, std::map<uint32_t, std::map<uint32_t, __ResType__*> > > StMap;
	StMap  m_mapData;
public:
	CMachaiMapTempete() {}
	virtual ~CMachaiMapTempete(){
		//delete m_resArr;
		m_mapData.clear();
	};

public:
	int loadData(const char* url){
		int iRet = LoadTemplateCfg<__ResType__,maxCount>(url,resArrData,&resArrDataUseLen);
		if(iRet<0){
			printf("load cfg[%s] failed:%d.\n", url, iRet);
			return iRet;
		}

		for(int i=0;i<resArrDataUseLen;i++){
			m_mapData[resArrData[i].iID%100][(resArrData[i].iID%1000)/100][resArrData[i].iID] = &resArrData[i];
		}

		return 0;
	};

	__ResType__* getDataById(int id){
		uint32_t mod = id%100;
		typename StMap::iterator it=m_mapData.find(mod);
		if (it == m_mapData.end())
			return NULL;

		uint32_t star = (id%1000) /100;
		typename std::map<uint32_t, std::map<uint32_t, __ResType__*> >::iterator sit = it->second.find(star);
		if (sit == it->second.end())
			return NULL;

		typename std::map<uint32_t, __ResType__*>::iterator fit = sit->second.find(id);
		if (fit != sit->second.end())
			return fit->second;

		return NULL;		
	};


	uint32_t MaxStar(uint32_t id) {
		uint32_t mod = id%100;
		typename StMap::iterator it = m_mapData.find(mod);
		if ( it == m_mapData.end())
			return 0;

		typename std::map<uint32_t, std::map<uint32_t, __ResType__*> >::reverse_iterator rit = it->second.rbegin();
		if (rit!=it->second.rend())
			return rit->first;

		return 0;
	}

	__ResType__  resArrData[maxCount];
	int			 resArrDataUseLen;
};
#else

//template<typename __ResType__,int maxCount>
template<typename __ResType__>
class CMachaiMapTempete {
	/// multimap<id,__ResType__*>
	typedef  typename std::multimap<uint32_t, __ResType__*> StMap;
	StMap  m_mapData;
public:
	typedef typename StMap::iterator iterator;
public:
	CMachaiMapTempete() : resArrData(NULL) {}
	virtual ~CMachaiMapTempete(){
		//delete m_resArr;
		m_mapData.clear();
		if (resArrData)
		{
			delete []resArrData;
			resArrData = NULL;
		}
	};

public:
	int loadData(const char* url){
		int iRet = LoadTemplateCfg<__ResType__>(url, &resArrData, &resArrDataUseLen);
		if(iRet<0){
			printf("load cfg[%s] failed:%d.\n", url, iRet);
			return iRet;
		}

		for(int i=0;i<resArrDataUseLen;i++){
			m_mapData.insert(std::make_pair(resArrData[i].iID, &resArrData[i]));
		}

		return 0;
	};

	std::pair<iterator, iterator>  find(uint32_t id) {		
		return m_mapData.equal_range(id);
	}

	__ResType__* find(uint32_t id, uint32_t star, uint32_t quality, uint32_t quality_lv) {
		__ResType__* record = NULL;
		std::pair<iterator, iterator> out = m_mapData.equal_range(id);
		for (iterator it=out.first; it!=out.second; ++it) {
			if (it->first == id && it->second->iStar==star
				&& it->second->iQuality==quality && it->second->iTrainLevel==quality_lv) {
					record = it->second;
					break;
			}		
		}

		return record;
	}

	uint32_t maxLevel(uint32_t id) {
		return 0;
	}

	uint32_t maxStar(uint32_t id,  uint32_t curStar) {
		uint32_t prev = id/1000;
		uint32_t last = id%100;
		for (;;) {
			uint32_t nextStar = curStar + 1;
			uint32_t nextId = prev*1000 + nextStar*100 + last;

			std::pair<iterator, iterator> out = find(nextId);
			
			bool bExist = false;
			for (iterator it=out.first; it!=out.second && !bExist; ++it) {
				bExist = true;
				curStar = nextStar;
			}

			if (!bExist)
				break;
		}

		return curStar;
	}

	uint32_t maxQuality(uint32_t id) {
		std::set<uint32_t> qualitySet;
		std::pair<iterator, iterator> out = find(id);

		for (iterator it=out.first; it!=out.second; ++it) {
			qualitySet.insert(it->second->iQuality);
		}

		std::set<uint32_t>::reverse_iterator rit = qualitySet.rbegin();
		if (rit == qualitySet.rend())
			return 0;

		return *rit;
	}

	uint32_t maxQualityLevel(uint32_t id, uint32_t quality) {
		std::set<uint32_t> qualitySet;
		std::pair<iterator, iterator> out = find(id);

		for (iterator it=out.first; it!=out.second; ++it) {
			if (it->second->iQuality == quality) {
				qualitySet.insert(it->second->iTrainLevel);
			}			
		}

		std::set<uint32_t>::reverse_iterator rit = qualitySet.rbegin();
		if (rit == qualitySet.rend())
			return 0;

		return *rit;
	}

	iterator begin() {
		return m_mapData.begin();
	}

	iterator end() {
		return m_mapData.end();
	}

	__ResType__* resArrData;
	int			 resArrDataUseLen;
};


template<typename __ResType__>
class CMachaiQualityItemMapTempete {
	/// multimap<id,__ResType__*>
	typedef  typename std::multimap<uint32_t, __ResType__*> StMap;
	StMap  m_mapData;
public:
	typedef typename StMap::iterator iterator;

public:
	CMachaiQualityItemMapTempete() : resArrData(NULL){}
	virtual ~CMachaiQualityItemMapTempete(){
		//delete m_resArr;
		m_mapData.clear();
		if (resArrData)
		{
			delete []resArrData;
			resArrData = NULL;
		}
	};

public:
	int loadData(const char* url){
		int iRet = LoadTemplateCfg<__ResType__>(url, &resArrData, &resArrDataUseLen);
		if(iRet<0){
			printf("load cfg[%s] failed:%d.\n", url, iRet);
			return iRet;
		}

		for(int i=0;i<resArrDataUseLen;i++){
			m_mapData.insert(std::make_pair(resArrData[i].iID, &resArrData[i]));
		}

		return 0;
	};

	std::pair<iterator, iterator>  find(uint32_t id) {		
		return m_mapData.equal_range(id);
	}

	__ResType__* find(uint32_t id, uint32_t quality, uint32_t quality_lv) {
		__ResType__* record = NULL;
		std::pair<iterator, iterator> out = m_mapData.equal_range(id);
		for (iterator it=out.first; it!=out.second; ++it) {
			if (it->first == id && it->second->iQuality==quality && it->second->iTrainLevel==quality_lv) {
					record = it->second;
					break;
			}		
		}

		return record;
	}

	__ResType__* resArrData;
	int			 resArrDataUseLen;
};


template<typename __ResType__>
class CMachaiAttrMapTempete {
	/// multimap<id,__ResType__*>
	typedef  typename std::multimap<uint32_t, __ResType__*> StMap;
	StMap  m_mapData;
public:
	typedef typename StMap::iterator iterator;

public:
	CMachaiAttrMapTempete() : resArrData(NULL){}
	virtual ~CMachaiAttrMapTempete(){
		//delete m_resArr;
		m_mapData.clear();
		if (resArrData)
		{
			delete []resArrData;
			resArrData = NULL;
		}
	};

public:
	int loadData(const char* url){
		int iRet = LoadTemplateCfg<__ResType__>(url, &resArrData, &resArrDataUseLen);
		if(iRet<0){
			printf("load cfg[%s] failed:%d.\n", url, iRet);
			return iRet;
		}

		for(int i=0;i<resArrDataUseLen;i++){
			m_mapData.insert(std::make_pair(resArrData[i].iID, &resArrData[i]));
		}

		return 0;
	};

	std::pair<iterator, iterator>  find(uint32_t id) {		
		return m_mapData.equal_range(id);
	}

	__ResType__* find(uint32_t id, uint32_t quality, uint32_t quality_lv) {
		__ResType__* record = NULL;
		std::pair<iterator, iterator> out = m_mapData.equal_range(id);
		for (iterator it=out.first; it!=out.second; ++it) {
			if (it->first == id && it->second->iQuality==quality && it->second->iTrainLevel==quality_lv) {
				record = it->second;
				break;
			}		
		}

		return record;
	}

	__ResType__* resArrData;
	int			 resArrDataUseLen;
};


template<typename __ResType__>
class CWingMapTempete {
	/// multimap<id,__ResType__*>
	typedef  typename std::multimap<uint32_t, __ResType__*> StMap;
	StMap  m_mapData;
public:
	typedef typename StMap::iterator iterator;

public:
	CWingMapTempete() : resArrData(NULL){}
	virtual ~CWingMapTempete(){
		//delete m_resArr;
		m_mapData.clear();
		if (resArrData) {
			delete []resArrData;
			resArrData = NULL;
		}
	};

public:
	int loadData(const char* url){
		int iRet = LoadTemplateCfg<__ResType__>(url, &resArrData, &resArrDataUseLen);
		if(iRet<0){
			printf("load cfg[%s] failed:%d.\n", url, iRet);
			return iRet;
		}

		for(int i=0;i<resArrDataUseLen;i++){
			m_mapData.insert(std::make_pair(resArrData[i].iID, &resArrData[i]));
		}

		return 0;
	};

	std::pair<iterator, iterator>  find(uint32_t id) {		
		return m_mapData.equal_range(id);
	}

	__ResType__* find(uint32_t id, uint32_t star_lv, uint32_t quality_lv) {
		__ResType__* record = NULL;
		std::pair<iterator, iterator> out = m_mapData.equal_range(id);
		for (iterator it=out.first; it!=out.second; ++it) {
			if (it->first == id && it->second->iStarLv==star_lv && it->second->iQualityLv==quality_lv) {
				record = it->second;
				break;
			}		
		}

		return record;
	}

	uint32_t maxStarLv(uint32_t id, uint32_t quality_lv) {
		uint32_t maxStar = 0;
		std::pair<iterator, iterator> out = m_mapData.equal_range(id);
		for (iterator it=out.first; it!=out.second; ++it) {
			if (it->first == id && it->second->iQualityLv==quality_lv) {
				maxStar = std::max(maxStar, (uint32_t)it->second->iStarLv);
			}		
		}

		return maxStar;
	}

	uint32_t maxStarLv(uint32_t id) {
		uint32_t maxStar = 0;
		std::pair<iterator, iterator> out = m_mapData.equal_range(id);
		for (iterator it=out.first; it!=out.second; ++it) {
			if (it->first == id) {
				maxStar = std::max(maxStar, (uint32_t)it->second->iStarLv);
			}		
		}

		return maxStar;
	}

	uint32_t maxQualityLv(uint32_t id, uint32_t star_lv) {
		uint32_t maxQuality = 0;
		std::pair<iterator, iterator> out = m_mapData.equal_range(id);
		for (iterator it=out.first; it!=out.second; ++it) {
			if (it->first == id && it->second->iStarLv==star_lv) {
				maxQuality = std::max(maxQuality, (uint32_t)it->second->iQualityLv);
			}		
		}

		return maxQuality;
	}

	uint32_t maxQualityLv(uint32_t id) {
		uint32_t maxQuality = 0;
		std::pair<iterator, iterator> out = m_mapData.equal_range(id);
		for (iterator it=out.first; it!=out.second; ++it) {
			if (it->first == id) {
				maxQuality = std::max(maxQuality, (uint32_t)it->second->iQualityLv);
			}		
		}

		return maxQuality;
	}


	__ResType__* resArrData;
	int			 resArrDataUseLen;
};


template<typename __ResType__>
class ResBossMapTempete{
private:

	typedef typename std::multimap<int , __ResType__*> ResMap;
	//typedef std::map<int ,int> ResMap;
	std::multimap<int , __ResType__*>  m_mapData;

public:	
	typedef typename ResMap::iterator iterator;

	ResBossMapTempete()
		: resArrData (NULL) {

	}

	virtual ~ResBossMapTempete(){
		//delete m_resArr;
		m_mapData.clear();

		if (resArrData) {
			delete []resArrData;
			resArrData = NULL;
		}
	};

	void clearData()
	{
		m_mapData.clear();
		if (resArrData) {
			delete []resArrData;
			resArrData= NULL;
		}
		resArrDataUseLen = 0;
	};

	int loadData(const char* url){
		//int count=0;
		int iRet = LoadTemplateCfg<__ResType__>(url, &resArrData,&resArrDataUseLen);
		if(iRet<0){
			printf("load cfg[%s] failed:%d.\n", url, iRet);
			return iRet;
		}
		for(int i=0;i<resArrDataUseLen;i++){
			m_mapData.insert(make_pair(resArrData[i].iID,&resArrData[i]));
		}
		return 0;
	};

	__ResType__* getDataById(int id) {
		for (int i=0; i<resArrDataUseLen; ++i) {
			if (resArrData[i].iID == id)
				return &resArrData[i];
		}

		return NULL;
	}
	
	bool exist(int id) {
		typename std::multimap<int , __ResType__*>::iterator ptr=m_mapData.find(id);
		return ptr != m_mapData.end();
	}

	iterator begin() {
		return m_mapData.begin();
	}

	iterator end() {
		return m_mapData.end();
	}

	//	__ResType__  resArrData[maxCount];
	__ResType__* resArrData;
	int			 resArrDataUseLen;
private:
};


#endif

#endif // __ResMapTempete_h__
