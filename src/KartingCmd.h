#ifndef __KARTING_CMD_H_
#define __KARTING_CMD_H_
/*
*疯狂动物园命令定义文件
*/

typedef enum
{
    KARTING_CMD_READYSTART       =      0x31,   //游戏开始倒计时
    KARTING_CMD_GAMESTART        =      0x32,   //游戏真正开始
    KARTING_CMD_ZHAUNG           =      0x33,   //上下庄
    KARTING_CMD_DEALER           =      0x34,   //确定庄家
    KARTING_CMD_BETTING          =      0x35,   //下注
	KARTING_CMD_LOTTERY		     =		0x36,	//开奖
	KARTING_CMD_BALANCE			 =		0x37,	//结算
    
	KARTING_CMD_ZHUANG_LIST		 =		0x38,	//获取上庄列表
	KARTING_CMD_RECORD			 =		0x39,	//获取游戏记录
	KARTING_CMD_SCENE 			 =		0x40,	//游戏场景
    KARTING_CMD_USER_INFO        =      0x41,   //获取玩家数据

	KARTING_CMD_ROBOT_ZHUANG	 =		0x90,	//通知机器人上庄
    KARTING_CMD_ROBOT_BETTING    =      0x91,   //通知机器人下注
}KARTING_CMD;

typedef enum BASEGAME_CMD
{
	BASEGAME_CMD_ERROR 			= 		0X10,
	BASEGAME_CMD_USER_LOGIN 	= 		0X11,
	BASEGAME_CMD_B_USER_LOGIN 	= 		0X12,
	BASEGAME_CMD_USER_READY 	= 		0X13,
	BASEGAME_CMD_USER_SITDOWN 	= 		0X14,
	BASEGAME_CMD_USER_SITUP 	= 		0X15,
	BASEGAME_CMD_USER_EXIT	 	= 		0X16,
	BASEGAME_CMD_USER_STATUS 	= 		0X17,
	BASEGAME_CMD_KITOUT_USER 	= 		0X18,

	BASEGAME_CMD_USER_CHAT 		= 		0x4F,
	BASEGAME_CMD_END 			= 		0X50,
	GAME_CMD_START,

}BASEGAME_CMD;

#endif
