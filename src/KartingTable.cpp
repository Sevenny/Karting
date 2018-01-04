#include "KartingTable.h"
#include "KartingComm.h"
#include "KartingServer.h"
#include "SendResponse.h"

CKartingTable::CKartingTable()
:CGameTable::CTableData()
,CEventObject()
,m_nTableId(0)
,m_nPlayerCnt(0)
,m_nRobotCnt(0)
,m_nIndex(ZONE_NONE)
,m_tWaitingTimer()
,m_tBettingTimer()
,m_tLotteryTimer()
,m_tBalanceTimer()
,m_tWaitZhuangTimer()
,m_tAddRobotTimer()
,m_tChangeZhuangTimer()
,m_tWtachDogTimer()
,m_tRobotBettingTimer()
{
    m_pDealer = nullptr;
	Init();
}

CKartingTable::~CKartingTable()
{
    m_tWaitingTimer.StopTimer();
    m_tBettingTimer.StopTimer();
    m_tLotteryTimer.StopTimer();
    m_tBalanceTimer.StopTimer();
    m_tWaitZhuangTimer.StopTimer();
    m_tAddRobotTimer.StopTimer();
    m_tChangeZhuangTimer.StopTimer();
    m_tWtachDogTimer.StopTimer();
    m_tRobotBettingTimer.StopTimer();
    
    delete m_pDealer;
}

void CKartingTable::Init()
{
	m_mapPlayer.clear();
	m_lGameRecord.clear();
	m_lZhuangList.clear();
    
	m_nDealerId         =       0;
	m_bStart            =       false;
    m_bWatchDog         =       false;
	m_nZhuangRounds     =       0;
    m_nZhuangTotalWin   =       0;
    m_nZhuangWin        =       0;
    m_nPlayerTotalChip  =       0;
    m_nRobotBettingLimit =      0;
    m_nTotalChip        =       0;
    m_nIndex            =       ZONE_NONE;
    m_nCoefficient      =       1000;
    m_nNeedRobotCnt     =       0;
    m_nVisiable         =       0;
    
    m_tWaitingTimer.init(TIMERID_WAITING, this);
    m_tBettingTimer.init(TIMERID_BETTING, this);
    m_tLotteryTimer.init(TIMERID_LOTTERY, this);
    m_tBalanceTimer.init(TIMERID_BALANCE, this);
    m_tWaitZhuangTimer.init(TIMERID_WAIT_ZHUANG, this);
    m_tAddRobotTimer.init(TIMERID_ADD_ROBOT, this);
    m_tChangeZhuangTimer.init(TIMERID_CHANGE_ZHUANG, this);
    m_tWtachDogTimer.init(TIMERID_WATCH_DOG, this);
    m_tRobotBettingTimer.init(TIMERID_ROBOT_BETTING, this);
    
	m_nStatus = TABLE_STATUS_WAITING;

	for (int i = 0; i < 9; i++)
	{
		m_stBettingZone[i].init();
        m_nPlayerBetting[i] = 0;
        m_nRobotBetting[i] = 0;
	}
    
    m_stBettingZone[1].nOdds = ODDS_FERRARI;
    m_stBettingZone[2].nOdds = ODDS_BMW;
    m_stBettingZone[3].nOdds = ODDS_BENZ;
    m_stBettingZone[4].nOdds = ODDS_DAZHONG;
    m_stBettingZone[5].nOdds = ODDS_FERRARI_MINI;
    m_stBettingZone[6].nOdds = ODDS_BMW_MINI;
    m_stBettingZone[7].nOdds = ODDS_BENZ_MINI;
    m_stBettingZone[8].nOdds = ODDS_DAZHONG_MINI;
    
    m_stBettingZone[1].nProbability = BASE_FERRARI;
    m_stBettingZone[2].nProbability = BASE_BMW;
    m_stBettingZone[3].nProbability = BASE_BENZ;
    m_stBettingZone[4].nProbability = BASE_DAZHONG;
    m_stBettingZone[5].nProbability = BASE_FERRARI_MINI;
    m_stBettingZone[6].nProbability = BASE_BMW_MINI;
    m_stBettingZone[7].nProbability = BASE_BENZ_MINI;
    m_stBettingZone[8].nProbability = BASE_DAZHONG_MINI;
    m_nCoefficient = BASE_COEFFICIENT;
}

void CKartingTable::Reset()
{
	m_nStatus = TABLE_STATUS_WAITING;
    m_bStart = false;
    m_nZhuangWin = 0;
    m_nPlayerTotalChip = 0;
    m_nRobotBettingLimit = 0;
    m_nTotalChip = 0;

	for (int i = 0; i < 9; i++)
	{
		m_stBettingZone[i].init();
        m_nPlayerBetting[i] = 0;
        m_nRobotBetting[i] = 0;
	}

	map<int, CKartingUser*>::iterator it = m_mapPlayer.begin();
	for (; it != m_mapPlayer.end(); it++)
	{
		CKartingUser* p = it->second;
		if (!p) continue;
        
        if (p->m_pUser->IsAndroid() && (int)p->m_pUser->GetUserId() != m_nDealerId)
        {
            if (GetRandom(1, 101) < 33)
            {
                p->Reset();
                p->StartQuitTimer(GetRandom(1, 5) * 1000);
                
                continue;
            }
        }

		if (p->m_pUser->IsOffline() && (int)p->m_pUser->GetUserId() != m_nDealerId)
		{
			logDebug("[%s]玩家离线, 踢出房间", p->UserInfo());
            p->Reset();
            p->StartQuitTimer(GetRandom(1, 6) * 500);

			continue ;
		}
		if (p->m_pUser->IsAndroid() && p->m_pUser->GetGold() < 5000)
		{
			logDebug("[%d]机器人金币[%d]不足, 踢出房间", p->m_pUser->GetUserId(), p->m_pUser->GetGold());
            p->Reset();
            p->StartQuitTimer(GetRandom(1, 6) * 500);

			continue ;
		}
        p->Reset();
	}
}

int CKartingTable::ProcessEvent(int nTimerId)
{
	//logDebug("定时器超时[%d]", nTimerId);
	switch(nTimerId)
	{
        case TIMERID_WAIT_ZHUANG:
            OnWaitZhuangTimeout();      break;
        case TIMERID_WAITING:
            StopTimer(TIMERID_WAITING);
			OnGameStart();	            break;
		case TIMERID_BETTING:
			OnLottery(); 		break;
		case TIMERID_LOTTERY:
			OnGameOver();		break;
		case TIMERID_BALANCE:
			OnBalanceTimeout();		        break;
        case TIMERID_ADD_ROBOT:
            OnAddRobotTimeout();        break;
        case TIMERID_CHANGE_ZHUANG:
            OnChangeZhuangTimeout();        break;
        case TIMERID_WATCH_DOG:
            OnWatchDogTimeout();        break;
        case TIMERID_ROBOT_BETTING:
            OnRobotBettingTimeout();        break;
		default:
			break;
	}
	return 0;
}
//玩家进入房间
int CKartingTable::PlayerIntoRoom(CGameUser* pUser)
{
    if (!m_bWatchDog)
    {
        StartTimer(TIMERID_WATCH_DOG, TIMEOUT_WATCH_DOG, true);
        m_bWatchDog = true;
    }
    
	int nUserId = pUser->GetUserId();
	CKartingUser* p = nullptr;
	map<int, CKartingUser*>::iterator it = m_mapPlayer.find(nUserId);
	if (it == m_mapPlayer.end())
	{
		p = dynamic_cast<CKartingUser*>(pUser->GetUserData());
        if (!p)
        {
            logError("[%d]转换玩家数据失败，不能进入房间", pUser->GetUserId());
            return -1;
        }
		m_mapPlayer.insert(map<int, CKartingUser*>::value_type(nUserId, p));
		if (pUser->IsAndroid())
        {
			m_nRobotCnt++;
            int nGold = pUser->GetGold();
            int nDiff = GetRandom(CKartingServer::Instance()->GetRobotMinGold(), CKartingServer::Instance()->GetRobotMaxGold());
            nDiff = nDiff / 1000 * 1000;
            nDiff -= nGold;
            
            //更新玩家金币
            pUser->AddTempGold(nDiff);
            pUser->UpdateUserData();
        }
		else
		{
			m_nPlayerCnt++;
            m_nNeedRobotCnt = GetRandom(CKartingServer::Instance()->GetRobotMin(), CKartingServer::Instance()->GetRobotMax()+1);
            //logDebug("[%s]玩家进入 金币[%9d] 当前人数[%2d] 真实玩家[%2d] 机器人[%2d]",
            //         p->UserInfo(), pUser->GetGold(), GetRoomPlayerCnt(), GetPlayerCnt(), GetRobotCnt());
		}
		p->Reset();
	}
    else
        p = it->second;
    
    StartTimer(TIMERID_ADD_ROBOT, 500);

	if (GetRoomPlayerCnt() >= 2 && !m_bStart)
	{
		m_bStart = true;
		//logDebug("满足游戏开始条件，启动等待上庄定时器");
		StartTimer(TIMERID_WAIT_ZHUANG, 5000);
	}

	return 0;
}

int CKartingTable::PlayerLeaveRoom(int nUserId)
{
	CKartingUser* p = GetUserByUserId(nUserId);
	if (!p) return 0;
	
	if (p->GetTotal() > 0 || nUserId == m_nDealerId)
	{
		logDebug("[%s]玩家游戏中, 不能退出游戏", p->UserInfo());
		p->SetReconnected(true);
		return -1;
	}

	map<int, CKartingUser*>::iterator it = m_mapPlayer.find(nUserId);
	if (it != m_mapPlayer.end())
		m_mapPlayer.erase(it);
	
	if (p->m_pUser->IsAndroid())
    {
		m_nRobotCnt--;
        StartTimer(TIMERID_ADD_ROBOT, 500);
    }
	else
    {
		m_nPlayerCnt--;
        logDebug("[%s]玩家离开 金币[%9d] 当前人数[%2d] 真实玩家[%2d] 机器人[%2d]",
                 p->UserInfo(), p->m_pUser->GetGold(), GetRoomPlayerCnt(), m_nPlayerCnt, m_nRobotCnt);
    }
    
    //从上庄列表剔除该玩家
    if (IsMember(nUserId, m_lZhuangList))
        m_lZhuangList.remove(nUserId);
	
	if (GetRoomPlayerCnt() == 0)
		StopTimer(TIMERID_WAITING);
	return 0;
}

void CKartingTable::KitoutPlayer(CGameUser* p)
{
    m_pTable->KitoutUser(p);
}

//开始游戏
int CKartingTable::OnGameStart()
{
    m_bStart = true;

	if (m_pTable->IsPlaying())
		return 0;
	if (GetRoomPlayerCnt() < 2)
    {
        logError("人数不足[%d]", GetRoomPlayerCnt());
        m_bStart = false;
        StartTimer(TIMERID_WAITING, CKartingServer::Instance()->GetWaitingTimeout());
		return m_pTable->GameBreak();
    }
    if (m_nDealerId == 0 || !m_pDealer)
    {
        m_bStart = false;
        logError("没有庄家[%d]，请上庄", m_nDealerId);
        CSendResponse::Instance()->NotifyRobotZhuang(this, nullptr, CALL_ZHUANG_UP);
        return m_pTable->GameBreak();
    }

	logDebug("===============================游戏开始===============================");

	m_nStatus = TABLE_STATUS_BETTING;//将桌子状态设置为下注阶段
	m_pTable->GameStart();
	m_logData.LogStart(m_nTableId, 0);
    GetZhuangLimit();
    //GetLotteryProbability();
    m_nRobotBettingLimit = GetRobotBettingLimit();
    m_nZhuangRounds++;
    
    logDebug("庄家列表:%s", GetZhuangList());

    //启动一个下注倒计时
    int nDuration = CKartingServer::Instance()->GetBettingTimeout();
	CSendResponse::Instance()->BroadcastGameStart(this, nDuration);
	StartTimer(TIMERID_BETTING, nDuration+1000);
    StartTimer(TIMERID_ROBOT_BETTING, nDuration / 3, true);
    CSendResponse::Instance()->NotifyRobotBetting(this, 100);
	//logDebug("[%s][%ds]", EnumToString(m_nStatus), nDuration/1000);
	return 0;
}

int CKartingTable::OnGameOver()
{
	StopTimer(TIMERID_LOTTERY);
    int nDuration = CKartingServer::Instance()->GetBalanceTimeout();

	OnBalance(nDuration);

	Reset();
    m_nStatus = TABLE_STATUS_BALANCE;
    //logDebug("[%s][%ds]", EnumToString(m_nStatus), nDuration/1000);
    m_logData.LogOver();
	m_pTable->GameOver();
    
    //换成延长
    StartTimer(TIMERID_BALANCE, nDuration+1000);
	return 0;
}

int CKartingTable::OnBalance(int nDuration)
{
	CSendResponse::Instance()->BroadcastBalance(this, nDuration, m_lGameRecord);
	return 0;
}

int CKartingTable::GetWin(int nIndex)
{
    int nRobotWin = 0;
    int nDealerWin = 0;
    nRobotWin = m_nRobotBetting[nIndex] * m_stBettingZone[nIndex].nOdds;//机器人买中所赢
    nRobotWin -= (m_nTotalChip - m_nPlayerTotalChip/* - m_nRobotBetting[nIndex]*/);//减去机器人未买中的筹码
    
    nDealerWin = (m_nTotalChip/* - m_stBettingZone[nIndex].nGold*/);//赢未中奖区域的筹码
    nDealerWin -= (m_stBettingZone[nIndex].nGold * m_stBettingZone[nIndex].nOdds);//减去中奖区域的输赢
    
    
    if (m_pDealer->m_pUser->IsAndroid())
    {
        nRobotWin += nDealerWin;
    }
    
    m_nZhuangWin = nDealerWin;
    m_pDealer->AddTotalWin(m_nZhuangWin);
    //logDebug("nIndex[%d] nRobotWin[%d]", nIndex, nRobotWin);
    
    return nRobotWin;
}

int CKartingTable::PlayerBetting(int nUserId, int nIndex, int nGold)
{
	CKartingUser* pUser = GetUserByUserId(nUserId);
	int nResult = 1;
    int nOdds = MAX_ODDS;
    int nLimit = pUser->GetBettingLimit();
    
    vector<int> v;//合法的下注额
    v.clear();
    CKartingServer::Instance()->GetJetton(v);
    
	string msg = "";
    char szMsg[128] = {0};
	if (!pUser)
		return 0;
	if (m_nStatus != TABLE_STATUS_BETTING)
	{
		snprintf(szMsg, sizeof(szMsg), "[%6d]不是下注阶段[%s]", nUserId, EnumToString(m_nStatus));
		nResult = -1;
		msg = szMsg;
        goto ERROR;
	}
    
    if (nUserId == m_nDealerId)
    {
        snprintf(szMsg, sizeof(szMsg), "[%s]庄家不能参与下注", pUser->UserInfo());
        nResult = -1;
        msg = szMsg;
        goto ERROR;
    }

	if (nIndex < 1 || nIndex > 8)
	{
		snprintf(szMsg, sizeof(szMsg), "[%6d]下注区域错误[%6d]", nUserId, nIndex);
		nResult = -1;
		msg = szMsg;
        goto ERROR;
	}
    
    if (!IsMember(nGold, v))
    {
        snprintf(szMsg, sizeof(szMsg), "[%6d]下注金额错误[%6d]", nUserId, nGold);
        nResult = -1;
        msg = szMsg;
        goto ERROR;
    }
    
    nLimit = nLimit > GetZoneLimit(nIndex) ? GetZoneLimit(nIndex) : nLimit;
    
    if (nGold > nLimit)
    {
        snprintf(szMsg, sizeof(szMsg), "区域[%d]下注[%d]大于上限[%d]", nIndex, nGold, nLimit);
        nResult = -1;
        msg = szMsg;
        goto ERROR;
    }
    
ERROR:
	if (nResult == -1)
	{        
		CSendResponse::Instance()->BroadcastBetting(this, pUser, nGold, nIndex, nResult, msg);
		return 0;
	}

	m_stBettingZone[nIndex].nGold += nGold;//区域下注总数
    if (pUser->m_pUser->IsAndroid())
        m_nRobotBetting[nIndex] += nGold;
    else
    {
        m_nPlayerBetting[nIndex] += nGold;
        m_nPlayerTotalChip += nGold;
    }

	pUser->AddJetton(nIndex, nGold);//玩家下注总数
	pUser->AddTotal(nGold);
    
    m_nTotalChip += nGold;
    /*
    int n = GetZoneLimit(nIndex);
    if (n < 1000)
        logDebug("区域[%d]可下注小于1000[%d]", nIndex, n);
    else if (n < 10000)
        logDebug("区域[%d]可下注小于10000[%d]", nIndex, n);
    else if (n < 100000)
        logDebug("区域[%d]可下注小于100000[%d]", nIndex, n);
    else if (n < 500000)
        logDebug("区域[%d]可下注小于500000[%d]", nIndex, n);
    else if (n < 1000000)
        logDebug("区域[%d]可下注小于1000000[%d]", nIndex, n);
*/
	CSendResponse::Instance()->BroadcastBetting(this, pUser, nGold, nIndex, nResult, msg);
	return 0;
}

//通知机器人进入房间
void CKartingTable::NotifyRobotIntoRoom()
{
	if (GetRobotCnt() >= m_nNeedRobotCnt)
    {
        //logError("机器人[%d]达到上限[%d]", GetRobotCnt(), m_nNeedRobotCnt);
		return ;
    }

	if ((int)m_mapPlayer.size() < CKartingServer::Instance()->GetMaxPlayerCount())
	{
		//logDebug("添加机器人, 目前机器人数量[%d]", GetRobotCnt()+1);
		m_pTable->AddAndroidUser();
	}
}

int CKartingTable::SendGameStatusToUser(CKartingUser * pUser)
{
	return CSendResponse::Instance()->SendGameScene(this, pUser, m_lGameRecord);
}

int CKartingTable::OnGetGameRecordList(CGameUser * pUser)
{
	CSendResponse::Instance()->SendGameRecordList(this, pUser, m_lGameRecord);
	return 0;
}

int CKartingTable::OnCallZhuang(CKartingUser * pUser, int nType)
{
	int nResult = 1;
	int nUserId = pUser->m_pUser->GetUserId();
	//logDebug("[%s]玩家[%s]庄操作", pUser->UserInfo(), nType == CALL_ZHUANG_UP ? "上" : "下");
    bool bInQueue = IsMember(pUser->m_pUser->GetUserId(), m_lZhuangList);
    
    if ((int)pUser->m_pUser->GetUserId() == m_nDealerId && nType == CALL_ZHUANG_DOWN &&
        m_nZhuangRounds != CKartingServer::Instance()->GetZhuangRounds())
    {
        //logDebug("[%s]庄家下庄", pUser->UserInfo());

        m_nZhuangRounds = CKartingServer::Instance()->GetZhuangRounds();
        CSendResponse::Instance()->SendZhuangResponse(this, pUser->m_pUser, nResult, nType, "success", m_lZhuangList);
        return 0;
    }
	
	if ((bInQueue && nType == CALL_ZHUANG_UP)                   /*已经在队列中，不能上庄*/
		|| (!bInQueue && nType == CALL_ZHUANG_DOWN))            /*不在队列中不能下庄*/
	{
        logError("[%s]玩家[%s]不能[%s], 或者坐庄中", pUser->UserInfo(), bInQueue ? "队列中" : "不在队列", nType == CALL_ZHUANG_UP ? "上庄" : "下庄");
		nResult = -1;
        CSendResponse::Instance()->SendZhuangResponse(this, pUser->m_pUser, nResult, nType, "error", m_lZhuangList);
        
        return 0;
	}

	//如果是第一个玩家上庄，则直接定为庄家，并广播游戏准备开始
    if (nType == CALL_ZHUANG_UP)
    {
        if (CKartingServer::Instance()->GetZhuangMinGold() > pUser->m_pUser->GetGold())
        {
            logError("[%s]玩家金币[%d]不足[%d]", pUser->UserInfo(), pUser->m_pUser->GetGold(),
                     CKartingServer::Instance()->GetZhuangMinGold());
            nResult = -1;
            CSendResponse::Instance()->SendZhuangResponse(this, pUser->m_pUser, nResult, nType, "success", m_lZhuangList);
            
            return 0;
        }
        
        if (m_nDealerId == 0)
        {
            logDebug("[%s]是第一个上庄玩家，直接定为庄家", pUser->UserInfo());
            m_nDealerId = pUser->m_pUser->GetUserId();
            m_pDealer = GetUserByUserId(m_nDealerId);
            if (!m_pDealer)
                logError("[%d]无法获取庄家信息", m_nDealerId);
            CSendResponse::Instance()->BroadcastDealer(this, m_nDealerId);
            //启动一个等待延迟开始游戏的倒计时 7s
            int nDuration = CKartingServer::Instance()->GetWaitingTimeout();
            
            CSendResponse::Instance()->BroadcastReadyStart(this, nDuration);
            StartTimer(TIMERID_WAITING, nDuration+1000);
        }
        else
        {
            nResult = InsertZhuang(nUserId);
            //回复上庄结果
            CSendResponse::Instance()->SendZhuangResponse(this, pUser->m_pUser, nResult, nType, "success", m_lZhuangList);
        }
    }
    else if (nType == CALL_ZHUANG_DOWN)
    {
        if (IsMember(nUserId, m_lZhuangList))
            m_lZhuangList.remove(nUserId);
        
        CSendResponse::Instance()->SendZhuangResponse(this, pUser->m_pUser, nResult, nType, "success", m_lZhuangList);
    }

	return 0;
}

int CKartingTable::OnGetZhuangList(CKartingUser * pUser)
{
	//logDebug("[%s]玩家获取庄列表", pUser->UserInfo());
	CSendResponse::Instance()->SendZhuangListResponse(this, pUser->m_pUser, m_lZhuangList);
	return 0;
}

int CKartingTable::OnGetUserInfo(CKartingUser *pUser)
{
    CSendResponse::Instance()->SendUserInfoResponse(this, pUser);
    return 0;
}

/*
 当前庄家携带金币 单个机器人下注上限
 [8000w,1.5亿)      20w
 [.5亿,3亿)        40w
 [3亿,无穷)       80w
*/
int CKartingTable::GetRobotBettingLimit()
{
    int nGold = m_pDealer->m_pUser->GetGold();
    if (nGold >= 8e7 && nGold < 1.5e8)
        return 2e5;
    if (nGold >= 1.5e8 && nGold < 3e8)
        return 4e5;
    if (nGold >= 3e8)
        return 8e5;
    return 4e5;
}

int CKartingTable::GetRank()
{
    int nProfit[9] = {0}; //系统收益
    if (m_pDealer->m_pUser->IsAndroid())//庄家是机器人
    {
        for (int i = 1; i < 9; i++)
        {
            nProfit[i] = m_nPlayerTotalChip - m_nPlayerBetting[i] * m_stBettingZone[i].nOdds;
        }
    }
    else//庄家是玩家
    {
        for (int i = 1; i < 9; i++)
        {
            nProfit[i] = m_nRobotBetting[i] * m_stBettingZone[i].nOdds - (m_nTotalChip - m_nPlayerTotalChip);
        }
    }
    
    for (int i = 1; i < 9; i++)
    {
        for (int j = i+1; j < 9; j++)
        {
            if (nProfit[i] > nProfit[j])
            {
                int temp = nProfit[i];
                nProfit[i] = nProfit[j];
                nProfit[j] = temp;
            }
        }
        //logDebug("%d %d", i, nProfit[i]);
    }
    
    string s = "";
    char sz[20] = "";
    vector<int> v;
    v.clear();
    int iProfit = 0;
    for (int i = 8; i > 0; i--)
    {
        for (int j = 1; j < 9; j++)
        {
            if (m_pDealer->m_pUser->IsAndroid())//庄家是机器人
                iProfit = m_nPlayerTotalChip - m_nPlayerBetting[i] * m_stBettingZone[i].nOdds;
            else//庄家是玩家
                iProfit = m_nRobotBetting[i] * m_stBettingZone[i].nOdds - (m_nTotalChip - m_nPlayerTotalChip);
            
            if (iProfit == nProfit[j] && m_stBettingZone[i].nRank == 0 && !IsMember(j, v))
            {
                v.push_back(j);
                m_stBettingZone[i].nRank = j;
            }
        }
        snprintf(sz, sizeof(sz), "[%d - %d] ", i, m_stBettingZone[i].nRank);
        s.append(sz);
    }
    
    logDebug("%s", s.c_str());
    
    return 0;
}

int CKartingTable::GetIndexByRank(int r)
{
    for (int i = 1; i < 9; i++)
    {
        if (m_stBettingZone[i].nRank == r)
            return i;
    }
    
    return 8;
}

const char* CKartingTable::GetZhuangList()
{
    if (!m_lZhuangList.empty())
    {
        string s = "";
        char sz[10] = {0};
        snprintf(sz, sizeof(sz), "[%d]", (int)m_lZhuangList.size());
        s.append(sz).append(">");
        list<int>::iterator it = m_lZhuangList.begin();
        for (; it != m_lZhuangList.end(); it++)
        {
            snprintf(sz, sizeof(sz), "%s", GetUserInfoByUserId(*it));
            s.append(sz).append(" ");
        }
        return s.c_str();
    }
    
    return "null list";
}

int CKartingTable::GetLotteryProbability()
{
    int nBase = 0;
    int nIndex = 0;
    nBase = CKartingServer::Instance()->GetProbability(nIndex);

    m_stBettingZone[nIndex].nProbability += nBase;
    m_nCoefficient += nBase;
    
    logDebug("[%d] %d > %d > %d > %d > %d > %d > %d > %d", m_nCoefficient,
             m_stBettingZone[1].nProbability,
             m_stBettingZone[2].nProbability,
             m_stBettingZone[3].nProbability,
             m_stBettingZone[4].nProbability,
             m_stBettingZone[5].nProbability,
             m_stBettingZone[6].nProbability,
             m_stBettingZone[7].nProbability,
             m_stBettingZone[8].nProbability);
    
    return 0;
}

int CKartingTable::InsertZhuang(int nUserId)
{
    if ((int)m_lZhuangList.size() < 10)
    {
        //logDebug("[%d]加入庄队列", nUserId);
        m_lZhuangList.push_back(nUserId);
    }
    else
    {
        logError("[%d]庄列表已满[%d]，上庄失败", nUserId, (int)m_lZhuangList.size());
        return -1;
    }
    
    return 1;
}

void CKartingTable::GetZhuangLimit()
{
    if (m_nDealerId == 0)
        return ;
    
    CKartingUser* pUser = GetUserByUserId(m_nDealerId);
    if (!pUser)
        return ;
    
    int nGold = pUser->m_pUser->GetGold();//庄家身上金币数
    
    int nOdds = 0;//最大倍数
    int nLimit = 0;
    int nMaxBetting = CKartingServer::Instance()->GetBettingLimit() * MAX_ODDS;
    if (nGold > nMaxBetting)
        nGold = nMaxBetting;
    
    logDebug("庄家[%s] 金币[%d]", m_pDealer->UserInfo(), nGold, nLimit);
    for (int i = 1; i < 9; i++)
    {
        nOdds = m_stBettingZone[i].nOdds;
        nLimit = (int)(nGold / nOdds);
        
        nLimit = (nLimit / 1000) * 1000;
        
        m_stBettingZone[i].nLimit = nLimit;
        //logDebug("[%d]Limit[%d]", i, nLimit);
    }
    m_nVisiable = nGold / MAX_ODDS;
}
//游戏输赢记录
void CKartingTable::InsertGameRecord(int nIndex)
{
	if (m_lGameRecord.size() == 6)
	{
        m_lGameRecord.pop_front();
	}

	m_lGameRecord.push_back(nIndex);
}

int CKartingTable::GetNextZhuang()
{
	if (m_lZhuangList.empty())
		return 0;

	int nUserId = m_lZhuangList.front();

	m_lZhuangList.erase(m_lZhuangList.begin());

	CKartingUser* p = GetUserByUserId(nUserId);
	if (p->m_pUser->GetGold() < CKartingServer::Instance()->GetZhuangMinGold())
	{
		logDebug("[%s]玩家金币[%d]不足，不能上庄", p->UserInfo(), p->m_pUser->GetGold());
        CSendResponse::Instance()->SendZhuangResponse(this, p->m_pUser, 1, CALL_ZHUANG_DOWN, "金币不足", m_lZhuangList);
		return GetNextZhuang();
	}

	return nUserId;
}

void CKartingTable::ChangeZhuang()
{
    m_nDealerId = GetNextZhuang();
    m_nZhuangRounds = 0;
    m_nZhuangTotalWin = 0;
    //广播轮庄
    m_pDealer = nullptr;
    if (m_nDealerId > 0)
    {
        m_pDealer = GetUserByUserId(m_nDealerId);
    }
    CSendResponse::Instance()->BroadcastDealer(this, m_nDealerId);
    //StartTimer(TIMERID_CHANGE_ZHUANG, 3000);
}

CKartingUser* CKartingTable::GetUserByUserId(int nUserId)
{
	map<int, CKartingUser*>::iterator it = m_mapPlayer.find(nUserId);
	if (it != m_mapPlayer.end())
		return it->second;

	return nullptr;
}

const char* CKartingTable::GetUserInfoByUserId(int nUserId)
{
    CKartingUser* p = GetUserByUserId(nUserId);
    if (!p)
        return "error";
    return p->UserInfo();
}

bool CKartingTable::IsRobot(int nUserId)
{
    CKartingUser* p = GetUserByUserId(nUserId);
    if (p)
        return p->m_pUser->IsAndroid();
    
    return false;
}

int CKartingTable::StartTimer(int nTimerId, int nDuration, bool isLoop)
{
	//logDebug("启动定时器[%s], 超时[%d]", EnumToString(nTimerId), nDuration);
	m_mTimer[nTimerId][nDuration] = GetTimeStamp();
	switch(nTimerId)
	{
        case TIMERID_WAIT_ZHUANG:
            m_tWaitZhuangTimer.StartTimer(nDuration, isLoop);   break;
        case TIMERID_WAITING://等待 7s
            m_tWaitingTimer.StartTimer(nDuration, isLoop);      break;
		case TIMERID_BETTING://下注 15s
			m_tBettingTimer.StartTimer(nDuration, isLoop); 		break;
		case TIMERID_LOTTERY://开奖 6s
			m_tLotteryTimer.StartTimer(nDuration, isLoop);		break;
		case TIMERID_BALANCE://结算 4s
			m_tBalanceTimer.StartTimer(nDuration, isLoop);		break;
        case TIMERID_ADD_ROBOT:
            m_tAddRobotTimer.StartTimer(nDuration, isLoop);     break;
        case TIMERID_CHANGE_ZHUANG: //轮庄 3s
            m_tChangeZhuangTimer.StartTimer(nDuration, isLoop);     break;
        case TIMERID_WATCH_DOG:
            //logDebug("启动定时器[%s], 超时[%d]", EnumToString(nTimerId), nDuration);
            m_tWtachDogTimer.StartTimer(nDuration, isLoop);     break;
        case TIMERID_ROBOT_BETTING: //机器人下注 5s
            m_tRobotBettingTimer.StartTimer(nDuration, isLoop);     break;
		default:
			break;
	}

	return 0;
}

int CKartingTable::StopTimer(int nTimerId)
{
	//logDebug("关闭定时器[%s]", EnumToString(nTimerId));
	
	switch(nTimerId)
	{
        case TIMERID_WAIT_ZHUANG:
            m_tWaitZhuangTimer.StopTimer();     break;
		case TIMERID_WAITING:
			m_tWaitingTimer.StopTimer();	    break;
		case TIMERID_BETTING:
			m_tBettingTimer.StopTimer(); 		break;
		case TIMERID_LOTTERY:
			m_tLotteryTimer.StopTimer();		break;
		case TIMERID_BALANCE:
			m_tBalanceTimer.StopTimer();		break;
        case TIMERID_ADD_ROBOT:
            m_tAddRobotTimer.StopTimer();       break;
        case TIMERID_CHANGE_ZHUANG:
            m_tChangeZhuangTimer.StopTimer();       break;
        case TIMERID_WATCH_DOG:
            //logDebug("关闭定时器[%s]", EnumToString(nTimerId));
            m_tWtachDogTimer.StopTimer();       break;
        case TIMERID_ROBOT_BETTING:
            m_tRobotBettingTimer.StopTimer();       break;
		default:
			break;
	}
	return 0;
}

void CKartingTable::OnAddRobotTimeout()
{
    StopTimer(TIMERID_ADD_ROBOT);
    
    NotifyRobotIntoRoom();
}

void CKartingTable::OnWatchDogTimeout()
{
    //如果没有庄家或者上庄列表为空，则通知机器人上庄
    if ((int)m_lZhuangList.size() < 3)
        CSendResponse::Instance()->NotifyRobotZhuang(this, nullptr, CALL_ZHUANG_UP);
    //如果庄列表有真实玩家
    else //庄列表至少要有三个玩家才有调整的意义
    {
        list<int>::iterator it = m_lZhuangList.begin();
        int nPos = 0;//真实玩家位置
        int nLastId = *it; //上一个机器人ID
        bool bPlayer = false;
        for (; it != m_lZhuangList.end(); it++)
        {
            int nUserId = *it;
            if (IsRobot(nUserId))//是机器人
            {
                nPos++;
                nLastId = nUserId;
            }
            else
            {
                bPlayer = true;
                break;//遇到第一个真实玩家，就结束循环
            }
        }
        if (bPlayer && nPos >= 1)
        {
            CGameUser* p = m_pTable->GetUserByUserId(nLastId);
            if (p)
                CSendResponse::Instance()->NotifyRobotZhuang(this, p, CALL_ZHUANG_DOWN);
        }
    }
}

void CKartingTable::OnRobotBettingTimeout()
{
    int nPercent = 100;
    int nRemain = GetRemainTime(TIMERID_BETTING);
    int nDuration = CKartingServer::Instance()->GetBettingTimeout();
    //logDebug("nRemain[%d]", nRemain);
    
    /*
     前三分之一时间 33%机器人下注 中三分之一时间 66%机器人下注 后三分一时间 所有机器人下注
     */
    if (nRemain >= nDuration / 3 * 2)
        nPercent = 100;
    else if (nRemain >= nDuration / 3)
    {
        nPercent = 100;
        StopTimer(TIMERID_ROBOT_BETTING);
    }
    
    CSendResponse::Instance()->NotifyRobotBetting(this, nPercent);
}

void CKartingTable::OnLottery()
{
	StopTimer(TIMERID_BETTING);
    StopTimer(TIMERID_ROBOT_BETTING);
    m_nStatus = TABLE_STATUS_LOTTERY;//开奖阶段
    
	//logDebug("开奖");
    int nIndex = GetRandom(1, 9);
    int nRandom = 0;
    int nThreshold = CKartingServer::Instance()->GetThreshold();
    
    if (m_nPlayerTotalChip > 0 || !m_pDealer->m_pUser->IsAndroid())
    {
        nRandom = GetRandom(1, m_nCoefficient+1);
        int nFerrari        =                       m_stBettingZone[1].nProbability;
        int nBMW            = nFerrari          +   m_stBettingZone[2].nProbability;
        int nBenz           = nBMW              +   m_stBettingZone[3].nProbability;
        int nDazhong        = nBenz             +   m_stBettingZone[4].nProbability;
        int nFerrariXiao    = nDazhong          +   m_stBettingZone[5].nProbability;
        int nBMWXiao        = nFerrariXiao      +   m_stBettingZone[6].nProbability;
        int nBenzXiao       = nBMWXiao          +   m_stBettingZone[7].nProbability;
        int nDazhongXiao    = nBenzXiao         +   m_stBettingZone[8].nProbability;
        
        if (nRandom <= nFerrari)
            nIndex = ZONE_FERRARI;
        else if (nRandom <= nBMW)
            nIndex = ZONE_BMW;
        else if (nRandom <= nBenz)
            nIndex = ZONE_BENZ;
        else if (nRandom <= nDazhong)
            nIndex = ZONE_DAZHONG;
        else if (nRandom <= nFerrariXiao)
            nIndex = ZONE_FERRARI_MINI;
        else if (nRandom <= nBMWXiao)
            nIndex = ZONE_BMW_MINI;
        else if (nRandom <= nBenzXiao)
            nIndex = ZONE_BENZ_MINI;
        else
            nIndex = ZONE_DAZHONG_MINI;
        
        logDebug("随机数[%02d] 概率:%d < %d < %d < %d < %d < %d < %d < %d", nRandom, nFerrari, nBMW, nBenz, nDazhong,
                 nFerrariXiao, nBMWXiao, nBenzXiao, nDazhongXiao);
        
        if (m_nPlayerTotalChip > nThreshold || !m_pDealer->m_pUser->IsAndroid())
        {
            vector<int> v;
            CKartingServer::Instance()->GetLottery(v, m_nTotalChip / 1000 * (1000 - CKartingServer::Instance()->GetShuishou()));
            GetRank();
            
            if (!IsMember(m_stBettingZone[nIndex].nRank, v))
            {
                int i = v.at(GetRandom(0, v.size()));
                logDebug("先是开了[%d], [%d]不在集合里, 最后决定开[%d]", nIndex, m_stBettingZone[nIndex].nRank, GetIndexByRank(i));
                nIndex = GetIndexByRank(i);
            }
        }
        else
        {
            logDebug("玩家总下注额[%d]低于阈值[%d]", m_nPlayerTotalChip, nThreshold);
        }
    }
    else
    {
        logDebug("没人下注，随机开[%d]", nIndex);
    }
    
    m_nIndex = nIndex;
    
    int nDuration = CKartingServer::Instance()->GetLotteryTimeout();
	CSendResponse::Instance()->BroadcastLottery(this, nDuration, nIndex);
    
    //logDebug("[%s][%ds]", EnumToString(m_nStatus), nDuration/1000);

	StartTimer(TIMERID_LOTTERY, nDuration+1000);
}

//结算完毕，启动等待游戏开始定时器
void CKartingTable::OnBalanceTimeout()
{
    StopTimer(TIMERID_BALANCE);
    m_nStatus =  TABLE_STATUS_WAITING;
    
    if (m_pDealer &&
        (m_pDealer->m_pUser->GetGold() < CKartingServer::Instance()->GetZhuangLowGold() ||
         m_nZhuangRounds >= CKartingServer::Instance()->GetZhuangRounds()))
    {
        logDebug("[%s]玩家金币不足或坐庄局数[%d]满", m_pDealer->UserInfo(), m_nZhuangRounds);
        ChangeZhuang();
    }
    if (m_nDealerId > 0)
    {
        int nDuration = CKartingServer::Instance()->GetWaitingTimeout();
        CSendResponse::Instance()->BroadcastReadyStart(this, nDuration);
        StartTimer(TIMERID_WAITING, nDuration+1000);
        //logDebug("[%s][%ds]", EnumToString(m_nStatus), nDuration/1000);
    }
    else
    {
        //通知机器人上庄
        StartTimer(TIMERID_WAIT_ZHUANG, 3000);
    }
}

void CKartingTable::OnChangeZhuangTimeout()
{
    StopTimer(TIMERID_CHANGE_ZHUANG);
    
    CSendResponse::Instance()->BroadcastDealer(this, m_nDealerId);
}

void CKartingTable::OnWaitZhuangTimeout()
{
	//通知机器人上庄
	StopTimer(TIMERID_WAIT_ZHUANG);
	if(m_nDealerId > 0)//如果有庄家了，则不上庄了
		return ;
	
	CKartingUser* p = nullptr;
	map<int, CKartingUser*>::iterator it = m_mapPlayer.begin();
    bool b = false;//是否找到合适上庄的机器人
	for (; it != m_mapPlayer.end(); it++)
	{
		p = it->second;
		if (p->m_pUser->IsAndroid() 
			&& p->m_pUser->GetGold() >= CKartingServer::Instance()->GetZhuangMinGold())
        {
            b = true;
			break;
        }
	}
    if (!b)//没有合适的机器人
        return ;
    
    logDebug("[%d]通知机器人上庄, 金币[%d]", p->m_pUser->GetUserId(), p->m_pUser->GetGold());
	CSendResponse::Instance()->NotifyRobotZhuang(this, p->m_pUser, CALL_ZHUANG_UP);
}

//获取定时器剩余时间
int CKartingTable::GetRemainTime(int nTimerId)
{
	//1通过id查询定时器开始时间戳和时长
	if (m_mTimer.find(nTimerId) == m_mTimer.end())
		return 0;

	map<int, int>::iterator it = m_mTimer[nTimerId].begin();
	int nDuration = it->first;
	int nTimeStamp = it->second;

	//2获取当前时间戳
	int nNowTime = GetTimeStamp();

	//3返回时间差  
	int nRemain = nDuration / 1000 - (nNowTime - nTimeStamp);
	
	nRemain =  nRemain < 0 ? 0 : nRemain;
    return (nRemain * 1000);
}


