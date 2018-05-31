#ifndef _TIME_HELPER_H__
#define _TIME_HELPER_H__

#include <time.h>
#include <string>
#include <stdint.h>

class time_helper
{
public:
		//每次进入事件循环之前会设置一个cache时间，之后都统一使用该事件做计算
	static void set_cached_time(uint64_t t);
	static uint64_t get_cached_time();

		//部分对时间精度要求非常高，或者没有设置cache时间的场所，可以实时计算当前时间
	static uint64_t get_micro_time();
	static std::string				print_time_string(time_t t,bool is_print = false);
	static int				get_current_time(timeval *t);
//	static unsigned                get_local_time(void);
	static unsigned				calc_time(time_t start_time,int hour,int min,int sec);
	static bool					get_duration_time(time_t time_start, time_t time_end, int& hour, int& min, int& sec);

//	static tm						localTime(time_t now);
//	static tm						localTime(void);
	static unsigned				get_day_hour(unsigned hour, time_t now = 0);	// 获取now所对应的整数天+hour对应的时间戳
	static unsigned				get_day_timestamp(unsigned hour, unsigned minute, unsigned second, time_t now=0); //当天指定时间的时间戳
	static unsigned				get_day_timestamp_old(unsigned hour, unsigned minute, unsigned second, time_t now=0); //当天指定时间的时间戳	
	static unsigned				get_cur_hour(time_t now = 0); /// result: 0~23
	static unsigned				lastOffsetTime(int offset = 0, time_t now = 0); //now之前的第一个偏移offset的时间点，offset是相对于0点的偏移时间
	static unsigned				lastOffsetTime_old(int offset = 0, time_t now = 0); //next offset time!	
	static unsigned				nextOffsetTime_old(int offset = 0, time_t now = 0); //next offset time!		
	static unsigned				nextOffsetTime(int offset = 0, time_t now = 0); //now之后的第一个偏移offset的时间点，offset是相对于0点的偏移时间
	static unsigned				get_cur_day_by_month(time_t now=0);  /// 获取当前时间是本月的第几天
	static bool                 is_same_month_by_year(time_t t1, time_t t2); /// 判断两时间戳是否为同一年中的同一月
	static unsigned				nextHalfHour(time_t now = 0);
	static unsigned				nextHour(time_t now = 0);
	static unsigned				nextWeek(int offset = 0, time_t now = 0);
	static unsigned				next2Minute(time_t now = 0);
	static unsigned             getWeek(time_t now=0);
	static unsigned				get_timestamp_by_day(int hour, int minute, time_t now = 0);  //当天指定时间的时间戳
	static unsigned				get_timestamp_by_day_old(int hour, int minute, time_t now = 0);  //当天指定时间的时间戳
	static unsigned				get_next_timestamp_by_week_old(int wday, int hour, int minute, time_t now = 0);  //下一个周几（wday）当天指定时间的时间戳
	static unsigned				get_next_timestamp_by_month_old(int mday, int hour, int minute, time_t now = 0);  //下个月（mday）号当天指定时间的时间戳

	static void					showTime(time_t t = 0);
	static void					showTime(tm t);

//		int						timeZone(void);
	static int                     timezone_offset(void);
	static bool					is_same_day(unsigned int t1, unsigned int t2, unsigned int offset);
		//microsecond 
	static void					sleep(int micro_secs);

	static uint32_t             get_time(int year, int month, int day);
//	static time_t               next_time_update();
	static unsigned get_cur_month_by_year(time_t now);
	static int      get_date_by_time(int32_t* year, int32_t* month, int32_t* day, time_t now=0);  /// 根据时间戳返回年/月/日
	static bool		is_same_hour(time_t t1, time_t t2);
	static uint32_t             get_month_day_by_month(uint32_t month);  //根据月份获取本月的天数
};

#endif
