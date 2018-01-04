#include "KartingRobot.h"
#include "KartingComm.h"
#include "KartingTable.h"
#include "KartingServer.h"

#define TIMERID_ROBOT_BETTING   0x01

CKartingRobot::CKartingRobot()
:CAndroidUser()
,m_tBettingTimer()
{
    m_tBettingTimer.init(TIMERID_ROBOT_BETTING, this);
}

CKartingRobot::~CKartingRobot()
{
    m_tBettingTimer.StopTimer();
}


int CKartingRobot::ProcessEvent(int nTimerId)
{
    switch(nTimerId)
    {
        case TIMERID_ROBOT_BETTING:
            OnBettingTimeout();     break;
        default:
            break;
    }
	return 0;
}


int CKartingRobot::OnEventGameMessage(const OUTPUT_MSG & msg)
{
	//logDebug("子命令[%d][%s]", msg.m_nSubCmd, SubCmdString(msg.m_nSubCmd));
	switch (msg.m_nSubCmd)
	{
		case KARTING_CMD_ROBOT_BETTING:	//收到下注通知
			OnNotifyBetting(msg);
			break;
		case KARTING_CMD_ROBOT_ZHUANG://收到上庄通知
			OnNotifyZhuang(msg);
            break;
        case KARTING_CMD_BETTING:
            OnBettingResponse(msg);
            break;
	}
	return 0;
}


int CKartingRobot::OnGameStart(const OUTPUT_MSG & msg)
{
    com::game::karting::GameStartBroadcast* pb = dynamic_cast<com::game::karting::GameStartBroadcast*>(msg.pMessage);
    
    int nTableId = pb->tableid();
    int nUserId = GetUserId();
    
    if (nUserId == pb->dealer())
        return 0;
    
    CKartingTable* pTable = CKartingServer::Instance()->GetTableByTableId(nTableId);
    if (!pTable)
    {
        logError("无法获取桌子信息[%d]", nTableId);
        return 0;
    }
    CKartingUser* pUser = pTable->GetUserByUserId(nUserId);
    if (!pUser)
    {
        logError("无法获取玩家信息[%d]", nUserId);
        return 0;
    }

    //RobotBetting(pTable);

    StartTimer(TIMERID_ROBOT_BETTING, GetRandom(3, 11) * 100);
	return 0;
}

int CKartingRobot::OnNotifyZhuang(const OUTPUT_MSG & msg)
{
	com::game::karting::NotifyRobotZhuang* pb = dynamic_cast<com::game::karting::NotifyRobotZhuang*>(msg.pMessage);
	
	int nTableId = pb->tableid();
	int nUserId = GetUserId();

	CKartingTable* pTable = CKartingServer::Instance()->GetTableByTableId(nTableId);
	if (!pTable)
	{
		logError("无法获取桌子信息[%d]", nTableId);
		return 0;
	}
	CKartingUser* pUser = pTable->GetUserByUserId(nUserId);
	if (!pUser)
	{
		logError("无法获取玩家信息[%d]", nUserId);
		return 0;
	}
    if (pTable->m_nDealerId == nUserId)
        return 0;
    
    int nType = pb->type();
    
    if (nUserId == pb->userid())
    {
        pTable->OnCallZhuang(pUser, nType);
    }
    else if (nType == CALL_ZHUANG_UP)
    {
        if (IsMember(nUserId, pTable->m_lZhuangList))
            return 0;
        
        if ((int)pTable->m_lZhuangList.size() >= 6)
            return 0;
        
        if (GetRandom(1, 101) <= 50 && pUser->m_pUser->GetGold() >= CKartingServer::Instance()->GetZhuangMinGold())
            pTable->OnCallZhuang(pUser, CALL_ZHUANG_UP);
    }
	return 0;
}

int CKartingRobot::OnNotifyBetting(const OUTPUT_MSG &msg)
{
    com::game::karting::NotifyRobotBetting* pb = dynamic_cast<com::game::karting::NotifyRobotBetting*>(msg.pMessage);
    
    int nTableId = pb->tableid();
    int nUserId = GetUserId();
    
    CKartingTable* pTable = CKartingServer::Instance()->GetTableByTableId(nTableId);
    if (!pTable)
    {
        logError("无法获取桌子信息[%d]", nTableId);
        return 0;
    }
    CKartingUser* pUser = pTable->GetUserByUserId(nUserId);
    if (!pUser)
    {
        logError("无法获取玩家信息[%d]", nUserId);
        return 0;
    }
    
    if (nUserId == pTable->m_nDealerId)
        return 0;
    
    int nRandom = GetRandom(1, 101);
    if (nRandom > pb->percent())
        return 0;
    
    StartTimer(TIMERID_ROBOT_BETTING, GetRandom(3, 11) * 100);
    return 0;
}

int CKartingRobot::OnBettingResponse(const OUTPUT_MSG & msg)
{
    com::game::karting::UserBettingBroadcast* pb = dynamic_cast<com::game::karting::UserBettingBroadcast*>(msg.pMessage);
    
    int nTableId = pb->tableid();
    int nUserId = GetUserId();
    
    if (pb->result() == -1)
    {
        return 0;
    }
    
    CKartingTable* pTable = CKartingServer::Instance()->GetTableByTableId(nTableId);
    if (!pTable)
    {
        logError("无法获取桌子信息[%d]", nTableId);
        return 0;
    }
    CKartingUser* pUser = pTable->GetUserByUserId(nUserId);
    if (!pUser)
    {
        logError("无法获取玩家信息[%d]", nUserId);
        return 0;
    }
    
    if (nUserId == pTable->m_nDealerId)
        return 0;
    /*
    if (pUser->GetTotal() >= pTable->m_nRobotBettingLimit)
    {
        logError("[%s]下注[%d]超过上限[%d]", pUser->UserInfo(), pUser->GetTotal(), pTable->m_nRobotBettingLimit);
        return 0;
    }
    */
    StartTimer(TIMERID_ROBOT_BETTING, GetRandom(3, 11) * 100);
    return 0;
}

int CKartingRobot::RobotBetting(CKartingTable *pTable)
{
    int nUserId = GetUserId();
    CKartingUser* pUser = pTable->GetUserByUserId(nUserId);
    if (!pUser)
    {
        logError("无法获取玩家信息[%d]", nUserId);
        return 0;
    }
    
    int nIndex = 0;
    int nGold = 0;
    
    vector<int> v;
    CKartingServer::Instance()->GetJetton(v);
    
    nGold = v.at(GetRandom(0, 3));
    
    int nTime = pTable->GetRemainTime(TIMERID_BETTING);
    
    if (nTime <= 1000)//最后一秒不下注
        return 0;
    
    nIndex = GetRandom(1, 9);
    
    //logDebug("[%d]玩家下注, 区域[%d], 金额[%d]", GetUserId(), nIndex, nGold);
    pTable->PlayerBetting(GetUserId(), nIndex, nGold);
    return 0;
}

void CKartingRobot::OnBettingTimeout()
{
    int nTableId = GetGameUser()->GetTableId();
    CKartingTable* pTable = CKartingServer::Instance()->GetTableByTableId(nTableId);
    if (!pTable)
    {
        logError("无法获取桌子信息[%d]", nTableId);
        return ;
    }
   
    RobotBetting(pTable);
}

void CKartingRobot::StartTimer(int nTimerId, int nDuration, bool isLoop)
{
    switch(nTimerId)
    {
        case TIMERID_ROBOT_BETTING:
            m_tBettingTimer.StartTimer(nDuration, isLoop);    break;
        default:
            break;
    }
}
void CKartingRobot::StopTimer(int nTimerId)
{
    switch(nTimerId)
    {
        case TIMERID_ROBOT_BETTING:
            m_tBettingTimer.StopTimer();    break;
        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////
