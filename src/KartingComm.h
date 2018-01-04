#ifndef __KARTING_COMM_H
#define __KARTING_COMM_H
//算法类
#include <vector>
#include <map>
#include <list>
#include <algorithm>
//标准库类
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <Options.h>
#include <Singleton.h>
#include <sys/time.h>
//自定义类
#include "KartingMsg.pb.h"
#include "GameBaseMsg.pb.h"
#include "KartingUser.h"
#include "KartingCmd.h"
//消息处理类
#include "ProcessZhuang.h"
#include "ProcessZhuangList.h"
#include "ProcessGameRecord.h"
#include "ProcessBetting.h"
#include "ProcessUserInfo.h"
//底层框架类
#include "DispatchPackage.h"
#include "InitSo.h"
#include "clib_log.h"
#include "log.h"
#include "wtypedef.h"
#include "LogServer.h"

using namespace std;

extern clib_log*  g_perrorlog;
extern clib_log*  g_pdebuglog;
#define ERRORFMT "[ERROR]%20s:%4d[%20s]: "
#define DEBUGFMT "[DEBUG]%20s:%4d[%20s]: "
	
#define logError(fmt, args...) g_perrorlog->logMsg(ERRORFMT fmt, __FILE__, __LINE__, __FUNCTION__, ##args)
#define logDebug(fmt, args...) g_pdebuglog->logMsg(DEBUGFMT fmt, __FILE__, __LINE__, __FUNCTION__, ##args)

#define ABS(X) ((X) > 0 ? (X) : -(X))		                //取绝对值
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))	                //取最大值
#define MIN(X, Y) ((X) > (Y) ? (Y) : (X))	                //取最小值
#define OVERZERO(X) ((X) <= 0 ? 1 : (X))	                //不低于1

const int ODDS_FERRARI         =           40;             //法拉利赔率
const int ODDS_BMW             =           30;             //宝马赔率
const int ODDS_BENZ            =           20;
const int ODDS_DAZHONG         =           10;
const int ODDS_FERRARI_MINI    =           5;
const int ODDS_BMW_MINI        =           5;
const int ODDS_BENZ_MINI       =           5;
const int ODDS_DAZHONG_MINI    =           5;

const int BASE_FERRARI         =           15;             //法拉利基本概率
const int BASE_BMW             =           20;
const int BASE_BENZ            =           30;
const int BASE_DAZHONG         =           60;
const int BASE_FERRARI_MINI    =           120;
const int BASE_BMW_MINI        =           120;
const int BASE_BENZ_MINI       =           120;
const int BASE_DAZHONG_MINI    =           120;
const int BASE_COEFFICIENT     =           605;            //基本系数（分母）

const int MAX_ODDS             =           ODDS_FERRARI;              //最大赔率

#define TIMEOUT_WATCH_DOG                   15 * 1000       //看门狗轮询时间

typedef enum
{
	ZHUANG_NULL					=			0x0,            //不是庄家
	ZHUANG_TOBE					=			0x01,		    //队列中
	ZHUANG_ING					=			0x02,		    //当庄中
}ZHUANG;

//上下庄
typedef enum
{
	CALL_ZHUANG_UP				=			0x01,		    //上庄
	CALL_ZHUANG_DOWN			=			0x02,		    //下庄
}CALLZHUANGTYPE;

typedef enum
{
	ZONE_NONE			        =			0x00,			//
	ZONE_FERRARI			    = 			0x01,			//法拉利
    ZONE_BMW                    =           0x02,           //宝马
    ZONE_BENZ                   =           0x03,           //奔驰
    ZONE_DAZHONG                =           0x04,           //大众
	ZONE_FERRARI_MINI			=			0x05,			//小法拉利
	ZONE_BMW_MINI		        =			0x06,			//小宝马
    ZONE_BENZ_MINI              =           0x07,           //小奔驰
    ZONE_DAZHONG_MINI           =           0x08,           //小大众
}ZONE;

typedef enum USER_STATUS{
	USER_STATUS_BEGIN 			= 			-1, 
	USER_STATUS_NULL 	 		=			0x00,				//没有状态
	USER_STATUS_FREE			=			0x01,				//站立状态
	USER_STATUS_LOOKON			=			0x02,				//旁观状态
	USER_STATUS_SITUP 	 		= 			0x03,				//起立状态
	USER_STATUS_SITDOWN			=			0x04,				//坐下状态
	USER_STATUS_READY			=			0x05,				//同意状态
	USER_STATUS_PLAYING			=			0x06,				//游戏状态
	USER_STATUS_OFFLINE			=			0x07,				//断线状态
	USER_STATUS_RECONNECTD  	=			0x08,				//玩家重连后状态
} USER_STATUS;

typedef enum _TABLE_STATUS
{
	TABLE_STATUS_WAITING		=			0x41,				//等待 7s
	TABLE_STATUS_BETTING		=			0x42,				//下注 15s
	TABLE_STATUS_LOTTERY		=			0x43,				//开奖 6s
	TABLE_STATUS_BALANCE	    =			0x44,				//结算 4s
}TABLE_STATUS;

typedef enum
{
	TIMERID_WAITING		        =		    1001,               //等待 7s
    TIMERID_BETTING             =           1002,               //下注 15s
	TIMERID_LOTTERY 		    =		    1003,               //开奖 6s
    TIMERID_BALANCE             =           1004,               //结算 4s
    TIMERID_WAIT_ZHUANG         =           1005,               //等待上庄 5s
	TIMERID_ADD_ROBOT		    =		    1006,               //添加机器人 0.5s
    TIMERID_CHANGE_ZHUANG       =           1007,               //庄家轮换
    TIMERID_WATCH_DOG           =           1099,               //看门狗 30s
    TIMERID_ROBOT_BETTING       =           1008,               //通知机器人下注
}TIMERID;	


static const char* EnumToString(int n)
{
	switch (n)
	{
	//定时器
	case TIMERID_WAITING:	        return "等待";             break;
	case TIMERID_LOTTERY:		    return "开奖";             break;
	case TIMERID_WAIT_ZHUANG:	    return "等待上庄";          break;
    case TIMERID_BETTING:           return "下注";             break;
	case TIMERID_BALANCE:	        return "结算";             break;
    case TIMERID_CHANGE_ZHUANG:     return "轮庄";             break;
    case TIMERID_WATCH_DOG:         return "看门狗";            break;
    case TIMERID_ROBOT_BETTING:     return "机器人下注";         break;

	//桌子状态
	case TABLE_STATUS_WAITING:	    return "等待阶段";			break;
	case TABLE_STATUS_BETTING:	    return "下注阶段";			break;
	case TABLE_STATUS_LOTTERY:	    return "开奖阶段";			break;
	case TABLE_STATUS_BALANCE:	    return "结算阶段";			break;

	//玩家状态
	case USER_STATUS_NULL: 		    return "没有状态"; 			break;
	case USER_STATUS_FREE: 		    return "站立状态";			break;
	case USER_STATUS_LOOKON: 	    return "旁观状态"; 			break;
	case USER_STATUS_SITUP: 	    return "起立状态"; 			break;
	case USER_STATUS_SITDOWN: 	    return "坐下状态";			break;
	case USER_STATUS_READY: 	    return "同意状态"; 			break;
	case USER_STATUS_PLAYING: 	    return "游戏状态";			break;
	case USER_STATUS_OFFLINE: 	    return "断线状态"; 			break;
	case USER_STATUS_RECONNECTD:    return "玩家重连";            break;
	default: 					    return "未知命令";			break;
	}

	return "未知命令";
}

static const char* ToString[] =
{
    "无结果",
    "法拉利",
    "宝马",
    "奔驰",
    "大众",
    "小法拉利",
    "小宝马",
    "小奔驰",
    "小大众"
};

//获取一个范围内的随机数
static int GetRandom(int begin, int end)
{
	int nResult = 0;
	if (end < 0 || begin < 0 || end < begin)
		nResult = 0;
	else if (begin == end)
		nResult = begin;
	else
	{
		struct timeval tv;
		gettimeofday(&tv, NULL);
		unsigned  int seed = tv.tv_sec + tv.tv_usec;//time(NULL);

		nResult = (rand_r(&seed) % (end - begin)) + begin;
	}
	//logDebug("begin[%d] end[%d] random[%d]", begin, end, nResult);
	return nResult;
}
//分割字符串
static int SplitString(string src, string sFlag, vector<string>& vecResult)
{
	int sFlagLen = sFlag.size();
	int lastPosition = 0, index = -1;
	while (-1 != (index = src.find(sFlag, lastPosition)))
	{
		vecResult.push_back(src.substr(lastPosition, index - lastPosition));
		lastPosition = index + sFlagLen;
	}

	string lastString = src.substr(lastPosition);
	if (!lastString.empty())
	{
		vecResult.push_back(lastString);
	}

	return 0;
}
//分割字符串
static int SplitString(string src, string sFlag, vector<int>& vecResult)
{
	int sFlagLen = sFlag.size();
	int lastPosition = 0, index = -1;
	while (-1 != (index = src.find(sFlag, lastPosition)))
	{
		vecResult.push_back(atoi(src.substr(lastPosition, index - lastPosition).c_str()));
		lastPosition = index + sFlagLen;
	}

	string lastString = src.substr(lastPosition);
	if (!lastString.empty())
	{
		vecResult.push_back(atoi(lastString.c_str()));
	}

	return 0;
}


static bool IsMember(const string str, const vector<string> vec)
{
	return  (find(vec.begin(), vec.end(), str) != vec.end());
}

static bool IsMember(const int pid, const set<int> s)
{
	return  (s.find(pid) != s.end());
}


static bool IsMember(const int pid, const vector<int> vec)
{
	return (find(vec.begin(), vec.end(), pid) != vec.end());
}

static int FliterZero(int nNumber)
{
	int nResult = nNumber;
	if (nResult < 10000)
		nResult = (nResult / 100) * 100;
	else if (nResult < 100000)
		nResult = (nResult / 1000) * 1000;
	else 
		nResult = (nResult / 10000) * 10000;

	return nResult;
}

static bool IsMember(const int id, const map<int, int> m)
{
	return (m.find(id) != m.end());
}

static bool IsMember(const int id, const list<int> l)
{
    list<int>::const_iterator it = l.begin();
    while(it != l.end())
    {
        if (*it == id)
            return true;
        it++;
    }
    
    return false;
}

static int GetTimeStamp()
{
	time_t t;

	return time(&t);
}

#endif
