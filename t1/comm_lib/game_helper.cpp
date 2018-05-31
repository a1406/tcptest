#include "game_helper.h"
#include "comm_define.h"
#include "excel_data.h"
#include "time_helper.h"
#include <assert.h>

int get_item_type(uint32_t item_id)
{
	if (item_id == 201010020)
	{
		return ITEM_TYPE_SILVER;		
	}
	if (item_id == 201010001)
	{
		return ITEM_TYPE_COIN;
	}
	if (item_id == 201010002)
	{
		return ITEM_TYPE_BIND_GOLD;
	}
	if (item_id == 201010003)
	{
		return ITEM_TYPE_GOLD;
	}
	if (item_id == 201010004)
	{
		return ITEM_TYPE_EXP;
	}
	if (item_id == 201010005)
	{
		return ITEM_TYPE_GUOYU_EXP;
	}
	if (item_id == 201010006)
	{
		return ITEM_TYPE_CHENGJIE_EXP;
	}
	if (item_id == 201010007)
	{
		return ITEM_TYPE_SHANGJIN_EXP;
	}
	// if (item_id == 201010008)
	// {
	// 	return ITEM_TYPE_GUOYU_COIN;
	// }
	// if (item_id == 201010009)
	// {
	// 	return ITEM_TYPE_CHENGJIE_COIN;
	// }
	if (item_id == 201010010)
	{
		return ITEM_TYPE_LINGSHI;
	}
	if (item_id == 201010011)
	{
		return ITEM_TYPE_GUILD_DONATION;
	}
	if (item_id == 201010012)
	{
		return ITEM_TYPE_GUILD_TREASURE;
	}
	if (item_id == 201010013)
	{
		return ITEM_TYPE_PARTNER_EXP;
	}
	if (item_id == 201010021)
	{
		return ITEM_TYPE_GONGXUN;
	}
	if (item_id == 201010022)
	{
		return ITEM_TYPE_XUEJING;
	}
	if (item_id == 201010023)
	{
		return ITEM_TYPE_SHENGWANG;
	}
	if (item_id >= 201070027 && item_id <= 201070036)
	{
		return ITEM_TYPE_EQUIP;
	}
	return ITEM_TYPE_ITEM;
}

bool control_is_open(ControlTable *ctrl_config, uint64_t now) //控制表里的活动，是否在开启时间
{
	bool in_time = false;
	do
	{
		if (!ctrl_config)
		{
			break;
		}

		//检查时间
		if (ctrl_config->n_OpenDay > 0)
		{
			bool pass = false;
			uint32_t week = time_helper::getWeek(now);
			for (size_t i = 0; i < ctrl_config->n_OpenDay; ++i)
			{
				if (week == ctrl_config->OpenDay[i])
				{
					pass = true;
					break;
				}
			}
			if (pass == false)
			{
				break;
			}
		}
		assert(ctrl_config->n_OpenTime == ctrl_config->n_CloseTime);
		if (ctrl_config->n_OpenTime > 0)
		{
			bool pass = false;
			for (size_t i = 0; i < ctrl_config->n_OpenTime; ++i)
			{
				uint32_t start = time_helper::get_timestamp_by_day(ctrl_config->OpenTime[i] / 100,
						ctrl_config->OpenTime[i] % 100, now);
				uint32_t end = time_helper::get_timestamp_by_day(ctrl_config->CloseTime[i] / 100,
						ctrl_config->CloseTime[i] % 100, now);
				if (now >= start && now <= end)
				{
					pass = true;
					break;
				}
			}
			if (pass == false)
			{
				break;
			}
		}

		in_time = true;
	} while(0);

	return in_time;
}

uint64_t rand_between(uint64_t a, uint64_t b)
{
	uint64_t min, max;
	if (a > b)
	{
		max = a;
		min = b;
	}
	else
	{
		max = b;
		min = a;
	}
	return random() % (max - min + 1) + min;
}

