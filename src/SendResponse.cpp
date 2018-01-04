#include "KartingComm.h"
#include "MsgDef.h"
#include "KartingTable.h"
#include "SendResponse.h"
#include "KartingServer.h"

static CSendResponse* pInstance = nullptr;
CSendResponse*  CSendResponse::Instance()
{
	if(pInstance != nullptr) return pInstance;
	pInstance = new CSendResponse();	
	return pInstance;
}

//广播游戏即将开始
int CSendResponse::BroadcastReadyStart(CKartingTable * pTable, int nDuration)
{
	com::game::karting::GameReadyStartBroadcast pb;
	pb.set_tableid(pTable->m_pTable->GetId());
	pb.set_timeout(nDuration);

    //logDebug("[%d]广播游戏即将开始[%d]", pTable->m_nTableId, pb.timeout());

	SendMsgToTable(pTable, pb, KARTING_CMD_READYSTART);
	return 0;
}


//广播游戏开始
int CSendResponse::BroadcastGameStart(CKartingTable* pTable, int nDuration)
{
	int nUserLimit = 0;

	map<int, CKartingUser*>::iterator it = pTable->m_mapPlayer.begin();
	for (; it != pTable->m_mapPlayer.end(); it++)
	{
		CKartingUser* pUser = it->second;
		if (!pUser)
			continue;
		
		com::game::karting::GameStartBroadcast pb;
		pb.set_tableid(pTable->m_pTable->GetId());
		pb.set_timeout(nDuration);
	    pb.set_dealer(pTable->m_nDealerId);
        pb.set_lianzhuang(pTable->m_nZhuangRounds);
        
        for (int i = 1; i < 9; i++)
        {
            pb.add_limit(pTable->GetZoneLimit(i));
        }

		//logDebug("[%s]limit[%d]", pUser->UserInfo(), nUserLimit);

		SendMsgToUser(pUser->m_pUser, pb, KARTING_CMD_GAMESTART);
	}
	return 0;
}

int CSendResponse::BroadcastDealer(CKartingTable * pTable, int nDealer)
{
	com::game::karting::ComfirmZhuangBroadcast pb;
	pb.set_tableid(pTable->m_nTableId);
    com::game::karting::UserInfo* info = pb.mutable_dealer();
    info->set_userid(nDealer);
    if (pTable->m_pDealer)
    {
        info->set_info(pTable->m_pDealer->m_pUser->GetUserInfo());

        logDebug("[%d]广播庄家[%s]", pTable->m_nTableId, pTable->m_pDealer->UserInfo());
    }
    else
        logError("没有庄家，请上庄");

	SendMsgToTable(pTable, pb, KARTING_CMD_DEALER);
	return 0;
}

//广播玩家押注
int CSendResponse::BroadcastBetting(CKartingTable* pTable, CKartingUser* pUser, int nJetton, int nIndex, int nResult, string msg)
{
	com::game::karting::UserBettingBroadcast pb;

	pb.set_tableid(pTable->m_nTableId);
	pb.set_userid(pUser->m_pUser->GetUserId());
	pb.set_jetton(nJetton);
	pb.set_index(nIndex);
    pb.set_gold(pUser->m_pUser->GetGold() - pUser->GetTotal());
    
    int nLimit = 0;
    int nVisiable = pb.gold() - pUser->GetTotal();
    int nMax = pTable->GetZoneLimit(1);
    for (int i = 1; i < 9; i++)
    {
        int nLimit = pTable->GetZoneLimit(i);
        pb.add_limit(nLimit);
        nMax = nLimit > nMax ? nLimit : nMax;
    }
    nVisiable = nVisiable < nMax ? nVisiable : nMax;
	pb.set_result(nResult);
	pb.set_msg(msg);
    pTable->m_nVisiable = nVisiable;
    pb.set_visiable(nVisiable);

	for (int i = 1 ; i < 9; i++)
	{
        com::game::karting::BettingZone* data = pb.add_data();
		data->set_index(i);
		data->set_gold(pUser->GetJetton(i));
        data->set_totalgold(pTable->m_stBettingZone[i].nGold);
	}
    /*
    if (!pUser->m_pUser->IsAndroid())
        logDebug("[%s]金币[%9d] 总下注[%9d] 剩余[%9d] [%d][%9d]",
                 pUser->UserInfo(), pb.gold(), pUser->GetTotal(),
                 nVisiable, nIndex, pTable->GetZoneLimit(nIndex));
    */
    if (nResult == -1)
        logDebug("[%s] [%d] [%7d] [%7d] [%s]:%s",
                 pUser->UserInfo(), nIndex, pb.jetton(), pTable->GetZoneLimit(nIndex),
                 nResult == -1 ? "失败" : "成功", msg.c_str());

	SendMsgToTable(pTable, pb, KARTING_CMD_BETTING);
	return 0;
}

//广播开奖
int CSendResponse::BroadcastLottery(CKartingTable* pTable, int nDuration, int nIndex)
{
    pTable->InsertGameRecord(nIndex);
    int nRandom = GetRandom(1, 33);
    
    map<int, CKartingUser*>::iterator it = pTable->m_mapPlayer.begin();
    for (; it != pTable->m_mapPlayer.end(); it++)
    {
        CKartingUser* pUser = it->second;
        if (!pUser)
            continue;

        com::game::karting::LotteryBroadcast pb;
        pb.set_tableid(pTable->m_nTableId);
        pb.set_timeout(nDuration);
        pb.set_index(nIndex);
        pb.set_start(nRandom);
        
        for (int i = 1 ; i < 9; i++)
        {
            com::game::karting::BettingZone* data = pb.add_data();
            data->set_index(i);
            data->set_gold(pUser->GetJetton(i));
            data->set_totalgold(pTable->m_stBettingZone[i].nGold);
        }

        SendMsgToUser(pUser->m_pUser, pb, KARTING_CMD_LOTTERY);
    }
	return 0;
}

//广播玩家上下庄
int CSendResponse::BroadcastZhuang(CKartingTable* pTable, CGameUser* pUser)
{
	if (!pUser)
	{
		logError("获取玩家信息失败!");
		return -1;
	}
	if (!pTable)
	{
		logError("获取桌子[%d]信息失败!", pTable->m_nTableId);
		return -1;
	}

	com::game::karting::UserZhuangResponse resp;
	resp.set_tableid(pTable->m_nTableId);
	resp.set_userid(pUser->GetUserId());
	resp.set_result(1);

	SendMsgToTable(pTable, resp, KARTING_CMD_ZHAUNG);
	return 0;
}

//广播玩家结算
int CSendResponse::BroadcastBalance(CKartingTable* pTable, int nDuration, list<int> l)
{
    int nIndex = pTable->m_nIndex;
    int nOdds = pTable->m_stBettingZone[nIndex].nOdds;
    int nRobotWin = pTable->GetWin(nIndex);//机器人总输赢，计入调节池
    
    logDebug("=========================游戏结束, 开始结算===========================");
    logDebug("|  开奖结果: %d %s   倍      率: %d 奖区下注: %d", nIndex, ToString[nIndex],
             pTable->m_stBettingZone[nIndex], pTable->m_stBettingZone[nIndex].nGold);
    logDebug("|  总 下 注: %-9d  机器人输赢: %d", pTable->m_nTotalChip, nRobotWin);
    logDebug("|  玩家     奖区下注   其他区下注     输赢      当前金币  ");
    logDebug("======================================================================");
    
	map<int, CKartingUser*>::iterator it = pTable->m_mapPlayer.begin();
    char szKey[64] = {0};
	for (; it != pTable->m_mapPlayer.end(); it++)
	{
		CKartingUser* pUser = it->second;
		if (!pUser)
			continue;

		com::game::karting::Balance pb;
		pb.set_tableid(pTable->m_nTableId);
		pb.set_userid(pUser->m_pUser->GetUserId());
        pb.set_userjetton(pUser->GetTotal());
        int nWin = 0;
		
		//计算玩家输赢 
        if ((int)pUser->m_pUser->GetUserId() == pTable->m_nDealerId)
        {
            pb.set_userwin(pTable->m_nZhuangWin);
            pUser->AddTotalWin(pTable->m_nZhuangWin);
            pUser->AddWin(pTable->m_nZhuangWin);
            pb.set_userjetton(0);
            
            pTable->m_nZhuangTotalWin += pTable->m_nZhuangWin;
        }
        else
        {
            nWin = nOdds * pUser->GetJetton(nIndex);
            nWin -= pUser->GetTotal();
            pUser->AddWin(nWin);
            pUser->AddTotalWin(nWin);
            pb.set_userwin(nWin);
            pb.set_userjetton(pUser->GetTotal());
        }
		pb.set_dealerwin(pTable->m_nZhuangWin);
        pb.set_totaljetton(pTable->m_nTotalChip);
        pb.set_timeout(nDuration);
        
        if (pUser->GetWin() != 0)
        {
            //写金币流水
            pTable->m_logData.AddUserLog(pUser->m_pUser->GetUserId(),//玩家id
                                     pUser->m_pUser->GetSeatId(),//玩家桌子号
                                     pUser->m_pUser->GetGold(),//玩家更新前金币
                                     pUser->GetWin(),
                                     pUser->m_pUser->IsAndroid(), 0);
            
            //更新玩家金币
            pUser->m_pUser->AddTempGold(pUser->GetWin());
            pUser->m_pUser->UpdateUserData();
        }
        
        pb.set_usergold(pUser->m_pUser->GetGold());
        pb.set_dealergold(pTable->m_pDealer->m_pUser->GetGold());
        pb.set_dealertotalwin(pTable->m_nZhuangTotalWin);
        pb.set_index(nIndex);
        
        if (!l.empty())
        {
            list<int>::iterator it = l.begin();
            for (; it != l.end(); it++)
            {
                pb.add_indexlist(*it);
            }
        }

        
		logDebug("|%s| %9d | %9d | %9d | %10d %s",
            pUser->UserInfo(),
			pUser->GetJetton(nIndex),
			pUser->GetTotal() - pUser->GetJetton(nIndex),
			pUser->GetWin(),
			pb.usergold(),
            (int)pUser->m_pUser->GetUserId() == pTable->m_nDealerId ? "| 庄家" : "");
        
        SendMsgToUser(pUser->m_pUser, pb, KARTING_CMD_BALANCE);
	}

    int nCurrStock = 0;
    int nStock = 0;
    int nTax = 0;
    //扣除税
    if (pTable->m_nPlayerTotalChip > 0)
        nTax = (CKartingServer::Instance()->GetShuishou() * pTable->m_nPlayerTotalChip) / 1000;
    
    snprintf(szKey, 64, "HINCRBY config_%d_%d currentstock %d",
             Options::Instance()->m_nGameType, Options::Instance()->m_nGameMod, nTax);
    
    nCurrStock = CKartingServer::Instance()->m_pRedis->command(szKey);
    CKartingServer::Instance()->SetCurrentStock(nCurrStock);
    
    //设置调节池
    nRobotWin -= nTax;
    snprintf(szKey, 64, "HINCRBY config_%d_%d stock %d",
             Options::Instance()->m_nGameType, Options::Instance()->m_nGameMod, nRobotWin);
    
    nStock = CKartingServer::Instance()->m_pRedis->command(szKey);
    CKartingServer::Instance()->SetRoomStock(nStock);
    
    logDebug("----------------------------------------------------------------------");
    logDebug("|  庄家  |    本局   |  总输赢   |   连庄    |   库存    |  调节池   |");
    logDebug("----------------------------------------------------------------------");
    logDebug("|%s| %9d | %9d | zhuang %2d | %9d | %9d |",
             pTable->m_pDealer->UserInfo(), pTable->m_nZhuangWin, pTable->m_nZhuangTotalWin,
             pTable->m_nZhuangRounds, CKartingServer::Instance()->GetCurrentStock(), nStock);
    logDebug("===============================结算完毕===============================\n\n");
    
	return 0;
}

//当前游戏状态回包
int CSendResponse::SendGameScene(CKartingTable * pTable, CKartingUser * pUser, list<int> l)
{
	int nStatus = pTable->m_nStatus;
    int nDuration = 0;
	com::game::karting::GameSceneResponse pb;
	pb.set_tableid(pTable->m_nTableId);
    pb.set_tablestatus(nStatus);
	pb.set_userid(pUser->m_pUser->GetUserId());
	pb.set_userstatus(pUser->IsReconnected() ? 1 : 2);
	pb.set_players(pTable->m_mapPlayer.size());
    pb.set_lianzhuang(pTable->m_nZhuangRounds);
    pb.set_dealertotalwin(pTable->m_nZhuangTotalWin);
    
    com::game::karting::UserInfo *info = pb.mutable_dealer();
    info->set_userid(pTable->m_nDealerId);
    if (pTable->m_pDealer)
        info->set_info(pTable->m_pDealer->m_pUser->GetUserInfo());

    switch(nStatus)
    {
        case TABLE_STATUS_WAITING://等待 7s
            nDuration = pTable->GetRemainTime(TIMERID_WAITING); break;
        case TABLE_STATUS_BETTING://下注 15s
            nDuration = pTable->GetRemainTime(TIMERID_BETTING); break;
        case TABLE_STATUS_LOTTERY://开奖 6s
            pb.set_index(pTable->m_nIndex);
            nDuration = pTable->GetRemainTime(TIMERID_LOTTERY); break;
        case TABLE_STATUS_BALANCE://结算 4s
            pb.set_index(pTable->m_nIndex);
            nDuration = pTable->GetRemainTime(TIMERID_BALANCE); break;
        default:
            nDuration = 0;
    }
    
    nDuration -= 1000;
    nDuration = nDuration > 0 ? nDuration : 0;
    
    pb.set_timeout(nDuration);
    pb.set_visiable(pTable->m_nVisiable);
    
    if (nStatus == TABLE_STATUS_BETTING || nStatus == TABLE_STATUS_LOTTERY)
    {
        for (int i = 1; i < 9; i++)
        {
            com::game::karting::BettingZone* data = pb.add_bettingzone();
            data->set_index(i);
            data->set_totalgold(pTable->m_stBettingZone[i].nGold);
            data->set_gold(pUser->GetJetton(i));
            
            pb.add_limit(pTable->GetZoneLimit(i));
        }
    }
    
    if (!l.empty())
    {
        list<int>::iterator it = l.begin();
        for (; it != l.end(); it++)
        {
            pb.add_indexlist(*it);
        }
    }

	if (!pUser->m_pUser->IsAndroid())
    {
        logDebug("[%s]玩家[%s] 金币[%9d] 玩家数[%d] [%s][%ds]", pUser->UserInfo(),
             pb.userstatus() == 1 ? "重连进入":"动态加入", pUser->m_pUser->GetGold(),
             pTable->m_nPlayerCnt,
             EnumToString(nStatus), nDuration/1000);
    }
  
	SendMsgToUser(pUser->m_pUser, pb, KARTING_CMD_SCENE);

	return 0;
}

int CSendResponse::SendGameRecordList(CKartingTable * pTable, CGameUser * pUser, list< int > l)
{
	com::game::karting::GameRecordResponse pb;

	//logDebug("[%d]获取游戏记录", pUser->GetUserId());
	if (!l.empty())
	{
        list<int>::iterator it = l.begin();
        for (; it != l.end(); it++)
		{
            pb.add_record(*it);
		}
	}

	SendMsgToUser(pUser, pb, KARTING_CMD_RECORD);
	return 0;
}
//上庄回复
int CSendResponse::SendZhuangResponse(CKartingTable * pTable, CGameUser * pUser, int nResult, int nType, string msg, list<int> l)
{
	com::game::karting::UserZhuangResponse pb;

	pb.set_result(nResult);
	pb.set_tableid(pTable->m_nTableId);
	pb.set_userid(pUser->GetUserId());
    pb.set_type(nType);
    pb.set_msg(msg);

	if (!l.empty())
	{	
		list<int>::iterator it = l.begin();
		for (; it != l.end(); it++)
        {
            com::game::karting::UserInfo* info = pb.add_zhuanglist();
            info->set_userid(*it);
            CGameUser* p = pTable->m_pTable->GetUserByUserId(*it);
            if (p)
                info->set_info(p->GetUserInfo());
        }
	}
    //logDebug("庄家列表:%s", pTable->GetZhuangList());
    if (nResult == -1)
        logDebug("操作失败[%d] nType[%d] msg:%s", nResult, nType, msg.c_str());
	
    SendMsgToUser(pUser, pb, KARTING_CMD_ZHAUNG);
	return 0;
}
//获取上庄列表
int CSendResponse::SendZhuangListResponse(CKartingTable * pTable, CGameUser * pUser, list < int > l)
{
	com::game::karting::ZhuangListResponse pb;

	pb.set_tableid(pTable->m_nTableId);
	pb.set_userid(pUser->GetUserId());

	if (!l.empty())
	{
		list<int>::iterator it = l.begin();
		for (; it != l.end(); it++)
        {
            com::game::karting::UserInfo* info = pb.add_zhuanglist();
            info->set_userid(*it);
            CGameUser* p = pTable->m_pTable->GetUserByUserId(*it);
            if (p)
                info->set_info(p->GetUserInfo());
        }
    }
    
    //logDebug("庄列表:%d", pb.zhuanglist_size());

	SendMsgToUser(pUser, pb, KARTING_CMD_ZHUANG_LIST);
	return 0;
}
//获取玩家数据
int CSendResponse::SendUserInfoResponse(CKartingTable *pTable, CKartingUser *pUser)
{
    com::game::karting::UserInfoResponse pb;
    pb.set_tableid(pTable->m_nTableId);
    
    com::game::karting::UserInfo* info = pb.mutable_info();
    info->set_userid(pUser->m_pUser->GetUserId());
    info->set_info(pUser->m_pUser->GetUserInfo());
    info->set_totalwin(pUser->GetTotalWin());
    
    SendMsgToUser(pUser->m_pUser, pb, KARTING_CMD_USER_INFO);
    return 0;
}
//通知机器人上庄
int CSendResponse::NotifyRobotZhuang(CKartingTable* pTable, CGameUser * pUser, int nType)
{
	com::game::karting::NotifyRobotZhuang pb;
	pb.set_tableid(pTable->m_nTableId);
    pb.set_userid(pUser ? pUser->GetUserId() : 0);
	pb.set_type(nType);
    
    SendMsgToTable(pTable, pb, KARTING_CMD_ROBOT_ZHUANG);
	return 0;
}

//通知机器人下注
int CSendResponse::NotifyRobotBetting(CKartingTable *pTable, int nPercent)
{
    com::game::karting::NotifyRobotBetting pb;
    pb.set_tableid(pTable->m_nTableId);
    pb.set_percent(nPercent);
    
    SendMsgToTable(pTable, pb, KARTING_CMD_ROBOT_BETTING);
    return 0;
}

template<class T>int CSendResponse::SendMsgToTable(CKartingTable* pTable, T pb, KARTING_CMD cmd)
{
    OUTPUT_MSG msg;
    msg.m_nMainCmd = CKartingServer::Instance()->GetGameType();
    msg.m_nSubCmd = cmd;
    msg.pMessage = &pb;
    
    CSendPacket::Instance()->BroadcastAllTableUser(pTable->m_pTable, msg);
    return 0;
}
template<class T>int CSendResponse::SendMsgToUser(CGameUser* pUser, T pb, KARTING_CMD cmd)
{
    OUTPUT_MSG msg;
    msg.m_nMainCmd = CKartingServer::Instance()->GetGameType();
    msg.m_nSubCmd = cmd;
    msg.pMessage = &pb;
    
    CSendPacket::Instance()->SendMsgToUser(pUser, msg);
    return 0;
}

