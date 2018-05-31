#ifndef app_data_statis_h_
#define app_data_statis_h_

#include "server_proto.h"
#include <string.h>


#define GAME_APPID 1005
#define GAME_APP_SECRET  "5e7f30b128c6bcdf04ab5c042154e1d0"
#define GAME_STATIS_APPID "0E1C2B3B7F65CC67A47B03D0FFE70CD9"


enum MAGIC_STATIS_ID
{
	MAGIC_ID_BEGIN,
	/// 创建银角色
	MAGIC_ID_CREATE_PLAYER,
	/// 登陆统计
	MAGIC_ID_LOGIN,
	MAGIC_ID_LOGOUT,			/// 登出
	/// 购买vip
	/// ext_num1购买物品的ID
	/// ext_num2购买物品消耗的人民币
	/// ext_num3购买物品发送的钻石数目
	/// ext_num4购买物品赠送的钻石数目
	MAGIC_ID_BUY_VIP,


	/// 商店购买
	/// ext_num1: 购买的物品id
	/// ext_num2: 购买的物品数量
	MAGIC_ID_SHOP_BUY,	

	/// 货币消耗
	MAGIC_ID_USE_GOLD, 
	MAGIC_ID_USE_ITEM,

	/// 货币增加
	MAGIC_ID_ADD_GOLD, 
	MAGIC_ID_ADD_ITEM,

	MAGIC_ID_RECHARGE,				/// 充值
	MAGIC_ID_ADD_LEVEL,				/// 升级

	MAGIC_ID_END	
};

enum MAGIC_STATIS_TYPE
{
	MAGIC_TYPE_GM                = 1, //GM命令
	MAGIC_TYPE_BAG_UNLOCK        = 2, //背包解锁
	MAGIC_TYPE_BAG_SELL          = 3, //背包出售
	MAGIC_TYPE_BAG_USE           = 4, //背包使用
	MAGIC_TYPE_MONSTER_DEAD      = 5, //怪物死亡掉落
	MAGIC_TYPE_RAID              = 6, //副本结算
	MAGIC_TYPE_RENAME            = 7, //角色改名
	MAGIC_TYPE_GATHER = 8, //采集
	MAGIC_TYPE_TASK_EVENT        = 9, //任务事件
	MAGIC_TYPE_TASK_REWARD       = 10, //任务奖励
	MAGIC_TYPE_TASK_DROP         = 11, //任务掉落
	MAGIC_TYPE_RELIVE         = 12, //复活
	MAGIC_TYPE_ENTER_RAID   = 13,   //进入副本
	MAGIC_TYPE_EQUIP_STAR_UP     = 14, //装备升星
	MAGIC_TYPE_EQUIP_ENCHANT     = 15, //装备附魔
	MAGIC_TYPE_EQUIP_DRILL       = 16, //装备开孔
	MAGIC_TYPE_EQUIP_INLAY       = 17, //装备镶嵌
	MAGIC_TYPE_EQUIP_STRIP       = 18, //装备剥离
	MAGIC_TYPE_GEM_COMPOSE       = 19, //宝石合成
	MAGIC_TYPE_SKILL = 20, //学技能
	MAGIC_TYPE_FASHION = 21, //时装
	MAGIC_TYPE_SHOP_BUY          = 22, //商城购买
	MAGIC_TYPE_HORSE = 23, //坐骑
	MAGIC_TYPE_WANYAOGU_BBQ = 24, //万妖谷烤火
	MAGIC_TYPE_WANYAOKA_REWARD = 25, //万妖卡周奖励
	MAGIC_TYPE_YUQIDAO_FILL     = 26, //御气道灌入真气
	MAGIC_TYPE_YUQIDAO_BREAK    = 27, //御气道冲脉
	MAGIC_TYPE_SET_PK_TYPE    = 28, //修改PK类型
	MAGIC_TYPE_BAGUAPAI_DECOMPOSE    = 29, //八卦牌分解
	MAGIC_TYPE_BAGUAPAI_REFINE_STAR    = 30, //八卦牌炼星
	MAGIC_TYPE_BAGUAPAI_REFINE_MAIN_ATTR    = 31, //八卦牌重铸
	MAGIC_TYPE_BAGUAPAI_REFINE_MINOR_ATTR    = 32, //八卦牌洗炼
	MAGIC_TYPE_PVP_RAID_DAILY_REWARD    = 33, //pvp副本每日奖励
	MAGIC_TYPE_PVP_RAID_LEVEL_REWARD    = 34, //pvp副本段位奖励
	MAGIC_TYPE_YAOSHI = 35, //妖师客栈
	MAGIC_TYPE_MUDER_PUNISH = 36, //PK惩罚
	MAGIC_TYPE_ACTIVE_REWARD = 37, //活跃度奖励
	MAGIC_TYPE_ACTIVITY_REWARD = 38, //活动奖励
	MAGIC_TYPE_GUILD_CREATE = 39, //帮会创建
	MAGIC_TYPE_GUILD_RENAME = 40, //帮会改名
	MAGIC_TYPE_GUILD_DAILY_REWARD = 41, //帮会每日奖励
	MAGIC_TYPE_GUILD_SKILL_PRACTICE = 42, //帮会技能修炼
	MAGIC_TYPE_ZHENYING = 43, //阵营
	MAGIC_TYPE_QUESTION = 44, //答题
	MAGIC_TYPE_FRIEND_GIFT = 45, //好友赠送礼物
	MAGIC_TYPE_LIVE_SKILL = 46, //生活技能
	MAGIC_TYPE_GUILD_BATTLE_WAIT = 47, //帮战等待奖励
	MAGIC_TYPE_GUILD_BATTLE_FIGHT = 48, //帮战战斗奖励
	MAGIC_TYPE_GUILD_BATTLE_ACTIVITY = 49, //帮战活动奖励
	MAGIC_TYPE_XUNBAO = 50, //寻宝
	MAGIC_TYPE_PARTNER_LEARN_SKILL = 51, //伙伴学习技能
	MAGIC_TYPE_PARTNER_USE_EXP_ITEM = 52, //伙伴使用经验丹
	MAGIC_TYPE_PARTNER_EXCHANGE = 53, //伙伴兑换
	MAGIC_TYPE_PARTNER_RECRUIT = 54, //伙伴招募
	MAGIC_TYPE_PARTNER_DISMISS = 55, //伙伴遣散
	MAGIC_TYPE_CASH_TRUCK = 56, //押镖
	MAGIC_TYPE_RECEIVE_JIJIANGOPEN = 57, //领取即将开启奖励
	MAGIC_TYPE_PARTNER_ATTR = 58, //伙伴属性
	MAGIC_TYPE_TASK_CHAPTER = 59, //任务章节奖励
	MAGIC_TYPE_FRIEND_RECEIVE_GIFT = 60, //好友接收礼物
	MAGIC_TYPE_PARTNER_BOND_ACTIVE = 61, //伙伴激活羁绊
	MAGIC_TYPE_PARTNER_BOND_REWARD = 62, //伙伴羁绊奖励
	MAGIC_TYPE_PARTNER_COMPOSE_STONE = 63, //伙伴神曜合成
	MAGIC_TYPE_PARTNER_FABAO_STONE = 64, //伙伴法宝合成
	MAGIC_TYPE_SERVER_LEVEL_BREAK = 65, //服务器等级突破
	MAGIC_TYPE_ACHIEVEMENT_REWARD = 66, //成就奖励
	MAGIC_TYPE_DOUFACHANG_BUY_CHALLENGE_COUNT = 67, //购买斗法场挑战次数
	MAGIC_TYPE_DOUFACHANG_REWARD = 68, //斗法场奖励
	MAGIC_TYPE_WORLDBOS_KILL_REWARD = 69, //世界boss击杀奖励
	MAGIC_TYPE_WORLDBOS_RANK_REWARD = 70, //世界boss排行奖励
	MAGIC_TYPE_SERVER_LEVEL_TURN = 71, //服务器等级经验转化
	MAGIC_TYPE_HEROCHALLENGE_SWEEP_REWARD = 72, //英雄挑战领取扫荡奖励消耗物品
	MAGIC_TYPE_HEROCHALLENGE_SWEEP_REWARD_RECIVE = 73, //英雄奖励物品进背包
	MAGIC_TYPE_MIJING_XIULIAN_SHUAXING = 74, //秘境修炼任务刷星消耗物品
	MAGIC_TYPE_FISHING = 75, //钓鱼
	MAGIC_TYPE_STRONG_GOAL = 76, //变强目标奖励
	MAGIC_TYPE_STRONG_CHAPTER = 77, //变强章节奖励
	MAGIC_TYPE_TRADE_ON_SHELF = 78, //交易上架
	MAGIC_TYPE_TRADE_OFF_SHELF = 79, //交易下架
	MAGIC_TYPE_TRADE_BUY = 80, //交易购买
	MAGIC_TYPE_TRADE_SOLD_EARNING = 81, //交易售出收益
	MAGIC_TYPE_GUILD_RUQIN_REWARD = 82, //帮会入侵奖励
	MAGIC_TYPE_TRADE_RE_SHELF = 83, //交易重新上架
	MAGIC_TYPE_TRADE_ENLARGE_SHELF = 84, //交易扩大格子
	MAGIC_TYPE_GUILD_RUQIN_BBQ = 85, //帮会入侵烤火
	MAGIC_TYPE_AUCTION_BID = 86, //拍卖竞价
	MAGIC_TYPE_AUCTION_BID_FAIL = 87, //拍卖竞价失败
	MAGIC_TYPE_AUCTION_BUY_NOW = 88, //拍卖一口价
	MAGIC_TYPE_AUCTION_SOLD = 89, //拍卖出售奖励
	MAGIC_TYPE_TRAVEL_ROUND_REWARD = 90, //游历一轮奖励
	MAGIC_TYPE_RECIEVE_LEVVL_REWARD = 91, //等级奖励
	MAGIC_TYPE_RECIEVE_ONLINE_REWARD = 92, //在线奖励
	MAGIC_TYPE_BAGUAPAI_MOVE_ADDITIONAL_ATTR    = 93, //八卦牌新洗炼
	MAGIC_TYPE_BUQIAN_XIAOHAO_YUANBAO = 94, //签到补签消耗元宝
	MAGIC_TYPE_GIVE_QIANDAO_REWARD    = 95, //给签到奖励
	MAGIC_TYPE_GIVE_QIANDAO_LEIJI_REWARD = 96, //给签到累计奖励
	MAGIC_TYPE_NORMO_BACK_USE_COIN    = 97, //普通找回消耗银票
	MAGIC_TYPE_PERFECT_BACK_USE_GOLD    = 98, //完美找回消耗金票
	MAGIC_TYPE_REWARD_BACK_FIVE_ITEM    = 99,  //奖励找回增加物品
	MSGIC_TYPE_YAOQIAN_SHU_USE_GOLD     = 100, //玩摇钱树消耗金票
	MSGIC_TYPE_YAOQIAN_SHU_ADD_COIN     = 101, //玩家摇摇钱树获取银票
	MSGIC_TYPE_TOWER = 102, //玩家摇摇钱树获取银票
	MAGIC_TYPE_GUILD_DONATE     = 103, //帮会捐献
	MSGIC_TYPE_RECEIVE_LOGIN_REWARD     = 104, //登录奖励
	MSGIC_TYPE_CIFU_REWARD = 105,   //赐福奖励

	MAGIC_TYPE_ZHANLIDAREN_REWARD     = 106, //限时活动战力达人奖励
	MAGIC_TYPE_SHIDAMENZONG_REWARD    = 107, //限时活动十大门宗奖励
	MAGIC_TYPE_RANKING_REWARD         = 108, //排行榜奖励
	MAGIC_TYPE_FRIEND_TRACK_ENEMY     = 109, //追踪仇人
	MAGIC_TYPE_MONEY_EXCHANGE_USE       = 110, //货币兑换消耗
	MAGIC_TYPE_MONEY_EXCHANGE_ADD       = 111, //货币兑换增加 
	MAGIC_TYPE_GUILD_BONFIRE_REWARD     = 112, //门宗篝火奖励
	MAGIC_TYPE_GUILD_LEVEL_GIFT     = 113, //门宗等级礼包
	MAGIC_TYPE_GUILD_CHUAN_GONG     = 114, //传功传功
	MAGIC_TYPE_PARTNER_RENAME = 115, //伙伴改名
	MAGIC_TYPE_JIU_GONG_BA_GUA_RECIVE_REWARD = 116, //九宫八卦领奖
	MAGIC_TYPE_RED_PACKET_SEND_USE_MONEY = 117, //发红包消耗
	MAGIC_TYPE_RED_PACKET_TUIHUAN_MONEY = 118, //红包剩余金钱退还
	MAGIC_TYPE_RED_PACKET_GRAB_ADD_MONEY_MONEY = 119, //抢红包给钱
	MAGIC_TYPE_PROPOSE_USE_RING = 120, //求婚消耗
	MAGIC_TYPE_RESERVE_WEDDING_USE_MONEY = 121, //预定婚礼消耗
	MAGIC_TYPE_RESERVE_WEDDING_GIVE_ITEM = 122, //预定婚礼给物品
	MAGIC_TYPE_CANCEL_PROPOSE_MARRY_USE_MONEY = 123, //取消订婚消耗元宝
	MAGIC_TYPE_CANCEL_PROPOSE_MARRY_MAIL = 124, //取消订婚发邮件
};


enum SHOP_TYPE
{
	SHOP_TYPE_MYSTERY,					/// 神秘商店
	SHOP_TYPE_GOLD,						/// 金币商店
	SHOP_TYPE_DIAMOND,					/// 钻石商店
	SHOP_TYPE_ARENA,					/// 竞技场商店
	SHOP_TYPE_VIP,						/// VIP商店
	SHOP_TYPE_GUILD,					/// 公会商店
	SHOP_TYPE_SCORE,					/// 积分商店
	SHOP_TYPE_EQUIP,					/// 装备商店
};

enum RAID_FINISH_TYPE
{
	RAID_TYPE_ONCE,						/// 单次完成
	RAID_TYPE_SWEEP,					/// 扫荡
};


/*static inline void proto_statis_info__init(proto_statis_info* info, uint32_t commit_type, uint32_t appid, uint32_t open_id, uint64_t player_id
							 , uint32_t server_id, uint32_t  timestamp, uint32_t magic_id, uint32_t ext_num1=0, uint32_t ext_num2=0
							 , uint32_t ext_num3=0, uint32_t ext_num4=0, uint32_t ext_num5=0, uint32_t  ext_num6=0, uint32_t ext_num7=0
							 , uint32_t ext_num8=0, uint32_t ext_num9=0, uint32_t ext_num10=0, int len1=0, char* str1=NULL, int len2=0, char* str2=NULL) {

	info->commit_type = commit_type;
	info->appid = appid;
	info->open_id = open_id;
	info->player_id = player_id;
	info->server_id = server_id;
	info->timestamp = timestamp;
	info->magic_id = magic_id;
	info->ext_num1 = ext_num1;
	info->ext_num2 = ext_num2;
	info->ext_num3 = ext_num3;
	info->ext_num4 = ext_num4;
	info->ext_num5 = ext_num5;
	info->ext_num6 = ext_num6;
	info->ext_num7 = ext_num7;
	info->ext_num8 = ext_num8;
	info->ext_num9 = ext_num9;
	info->ext_num10 = ext_num10;
	info->len1 = len1;
	info->len2 = len2;
	
	if (len1)
		memcpy(&info->str1[0], str1, len1);
	if (len2)
		memcpy(&info->str2[0], str2, len2);
}*/



#endif//app_data_statis_h_
