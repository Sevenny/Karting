#ifndef __SEND_RESPONSE_H
#define __SEND_RESPOMSE_H

#include "SendPacket.h"
#include "KartingTable.h"

class CKartingTable;
class CGameUser;
class CSendResponse
{
public:
	static CSendResponse* Instance();

	//广播游戏即将开始
	int BroadcastReadyStart(CKartingTable* pTable, int nDuration);
	//广播游戏开始
	int BroadcastGameStart(CKartingTable* pTable, int nDuration);
	//确定庄家广播
	int BroadcastDealer(CKartingTable * pTable, int nDealer);
	//广播玩家下注
	int BroadcastBetting(CKartingTable* pTable, CKartingUser* pUser, int nJetton, int nIndex, int nResult, string msg);
	//广播开奖
	int BroadcastLottery(CKartingTable* pTable, int nDuration, int nIndex);
	//广播玩家上下庄
	int BroadcastZhuang(CKartingTable* pTable, CGameUser* pUser);
	//广播玩家结算
	int BroadcastBalance(CKartingTable* pTable, int nDuration, list<int> l);
    //当前游戏状态回包
    int SendGameScene(CKartingTable* pTable, CKartingUser* pUser, list<int> l);
	//发送游戏记录
	int SendGameRecordList(CKartingTable* pTable, CGameUser* pUser, list<int> l);
	//上下庄回包
	int SendZhuangResponse(CKartingTable* pTable, CGameUser* pUser, int nResult, int nType, string msg, list<int> l);
	//获取庄列表回包
	int SendZhuangListResponse(CKartingTable* pTable, CGameUser* pUser, list<int> l);
    //获取玩家数据
    int SendUserInfoResponse(CKartingTable* pTable, CKartingUser* pUser);

	//通知机器人上庄
	int NotifyRobotZhuang(CKartingTable* pTable, CGameUser* pUser, int nType);
    //通知机器人下注
    int NotifyRobotBetting(CKartingTable* pTable, int nPercent);
    
    template<class T>int SendMsgToTable(CKartingTable* pTable, T pb, KARTING_CMD cmd);
    template<class T>int SendMsgToUser(CGameUser* pUser, T pb, KARTING_CMD cmd);
};


#endif
