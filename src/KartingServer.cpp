#include "KartingComm.h"
#include "IniConfig.h"
#include "GameServer.h"
#include "KartingTable.h"
#include "KartingServer.h"
#include "GameTable.h"

static CKartingServer* pInstance = nullptr;

CKartingServer* CKartingServer::Instance()
{
	if(pInstance != nullptr) return pInstance;
		pInstance = new CKartingServer();
	
	return pInstance;
}

CKartingServer::CKartingServer()
{

}


CKartingServer::~CKartingServer()
{

}

int CKartingServer::init()
{
	SetLogErrorFle("./log/debug_%d.log", Options::Instance()->m_nGameMod);
	SetLogdebugFle("./log/debug_%d.log", Options::Instance()->m_nGameMod);
	
	ReadConf();

	g_pdebuglog->set_level(3);
    m_vecJetton.clear();
    m_vecJetton.push_back(1000);
    m_vecJetton.push_back(10000);
    m_vecJetton.push_back(100000);
    m_vecJetton.push_back(500000);
    m_vecJetton.push_back(1000000);

	m_pRedis = new RedisClient();
	int nRet = m_pRedis->init(m_szRedisHost, m_nRedisPort, 1000, m_szRedisPass);
	if (nRet < 0)
	{
		logError("初始化Redis失败");	   
	}
	else
	{
		logDebug("初始化Redis完成");
	}

	ReadRedis();
	
	m_mTable.clear();
	SetGameType(Options::Instance()->m_nGameType);
	SetGameMod(Options::Instance()->m_nGameMod);
	SetMaxPlayerCount(Options::Instance()->m_nMaxPlayerCount);
	SetMinPlayerCount(Options::Instance()->m_nMinPlayerCount);
	
	logError("===============================================");
	logError("->               初始化服务器                ||");
	logError("-----------------------------------------------");
	logError("-> 玩家下限·························[%7d]||",     GetMinPlayerCount());
	logError("-> 玩家上限·························[%7d]||",     GetMaxPlayerCount());
	logError("-> 游戏类型·························[%7d]||",     GetGameType());
	logError("-> 游戏模式·························[%7d]||",     GetGameMod());
	logError("-> 上庄条件·························[%5d万]||",    m_nZhuangMinGold/10000);
    logError("-> 下庄条件·························[%5d万]||",    m_nZhuangLowGold/10000);
    logError("-> 坐庄轮数·························[%7d]||",     m_nZhuangRounds);
	logError("-> 机器人上限·······················[%7d]||",      m_nRobotMax);
    logError("-> 机器人下限·······················[%7d]||",      m_nRobotMin);
	logError("-> 调节池···························[%5d万]||",   m_nRoomStock/10000);
	logError("-> 库存·····························[%5d万]||",  m_nCurrentStock/10000);
	logError("===============================================\n\n");
		
	return 0;
}

void CKartingServer::GetJetton(vector<int> &v)
{
    v = m_vecJetton;
}

int CKartingServer::ReadConf(const char *pFile)
{   	
	CReadIniFile iniReader;
	char section[64] = {0};
	
	iniReader.load_ini_file(pFile);
	snprintf(section,sizeof(section)-1,"CONFIG_%d", Options::Instance()->m_nGameMod);
	
    //定时器时间
	m_nWaitingTimeout     = iniReader.read_profile_int(section,     "m_nWaitingTimeout",    7000);
	m_nBettingTimeout     = iniReader.read_profile_int(section,     "m_nBettingTimeout",    15000);
	m_nLotteryTimeout     = iniReader.read_profile_int(section,     "m_nLotteryTimeout",    10000);
	m_nBalanceTimeout     = iniReader.read_profile_int(section,     "m_nBalanceTimeout",    6000);
    
    //redis接口
	iniReader.read_profile_string(section, "m_strRedisHost", m_szRedisHost, 64, "127.0.0.1");
	iniReader.read_profile_string(section, "m_strRedisPass", m_szRedisPass, 64, "");
	m_nRedisPort            = iniReader.read_profile_int(section, "m_nRedisPort",           6379);
    //百人系类
	m_nZhuangMinGold        = iniReader.read_profile_int(section, "m_nZhuangMinGold",       80000000);
	m_nZhuangLowGold        = iniReader.read_profile_int(section, "m_nZhuangLowGold",       80000000);
    m_nBettingLimit         = iniReader.read_profile_int(section, "m_nBettingLimit",        7500000);
	m_nZhuangRounds         = iniReader.read_profile_int(section, "m_nZhuangRounds",        5);
    m_nRobotMin             = iniReader.read_profile_int(section, "m_nRobotMin",            10);
    m_nRobotMax             = iniReader.read_profile_int(section, "m_nRobotMax",            40);
    m_nThreshold            = iniReader.read_profile_int(section, "m_nThreshold",           10000);
    
	return 0;
}

int CKartingServer::ReadRedis()
{
	char szKey[64] = {0};
	snprintf(szKey, 64, "HGETALL config_%d_%d", Options::Instance()->m_nGameType, Options::Instance()->m_nGameMod);
	if (m_pRedis->command(szKey) == 0)
	{
		snprintf(szKey, 64, "config_%d_%d", Options::Instance()->m_nGameType, Options::Instance()->m_nGameMod);
        //奖池
		string strMsg = m_pRedis->get_value_as_string("stock");
		if (strMsg == "nil")
		{
			m_nRoomStock = 100000000;//设置默认值
			m_pRedis->command("HSET %s stock %d", szKey, m_nRoomStock);
		}
		else
		{
			m_nRoomStock = atoi(strMsg.c_str());
		}
        //库存
		strMsg = m_pRedis->get_value_as_string("currentstock");
		if (strMsg == "nil")
		{
			m_nCurrentStock = -100000000;//设置默认值
			m_pRedis->command("HSET %s currentstock %d", szKey, m_nCurrentStock);
		}
		else
		{
			m_nCurrentStock = atoi(strMsg.c_str());
		}
        //税收
		strMsg = m_pRedis->get_value_as_string("shuishou");
		if (strMsg == "nil")
		{
			m_nShuishou = 1;//设置默认值
			m_pRedis->command("HSET %s shuishou %d", szKey, m_nShuishou);
		}
		else
		{
			m_nShuishou = atoi(strMsg.c_str());
		}
	}

	return 0;
}

int CKartingServer::GetWhiteList(int nUserId)
{
    char szKey[64] = {0};
    snprintf(szKey, 64, "%d", nUserId);
    string strValue = "";
    
    if (m_pRedis->HGet("hash_white_list", szKey, strValue) != 0)
        return 0;
    
    return atoi(strValue.c_str());
}

int CKartingServer::GetBlackList(int nUserId)
{
	char szKey[64] = {0};
	snprintf(szKey, 64, "%d", nUserId);
	string strValue = "";
	
	if (m_pRedis->HGet("hash_black_list", szKey, strValue) != 0)
		return -1;

	return atoi(strValue.c_str());
}

//调节池区间  (-无穷，0) [0,5000w) [5000w,1亿) [1亿,2亿) [2亿,无穷)
//机器人胜负率 -10   -5    0          5         10        15
int CKartingServer::GetProbability(int &nIndex)
{
    int nBase = 0;
    int nStock = GetRoomStock();
    int nRandom = GetRandom(1, 9);
    
    if (nStock < 0)
    {
        if (nRandom <= 4)
            nBase = -10;
        else
            nBase = 10;
        
    }
    else if (nStock >= 0 && nStock < 50000000)
    {
        if (nRandom <= 4)
            nBase = -5;
        else
            nBase = 5;
    }
    else if (nStock >= 50000000 && nStock < 100000000)
    {
        nBase = 0;
    }
    else if (nStock >= 100000000 && nStock < 200000000)
    {
        if (nRandom <= 4)
            nBase = 5;
        else
            nBase = -5;
    }
    else if (nStock >= 200000000)
    {
        if (nRandom <= 4)
            nBase = 10;
        else
            nBase = -10;
    }
    
    //logDebug("nStock[%d] nRandom[%d] nCheatType[%d]", nStock, nRandom, nCheatType);
    nIndex = nRandom;
    return nBase;
}

/*
 区间 (-&,-8000w) [-8000w,0)  [0,8000w)
 开奖    7、8        6、7、8   5、6、7、8
 区间 [8000w,30000w) [30000w,38000w) [38000w,46000w) [46000w,+&)
 开奖    随机           1、2、3、4、5      1、2、3、4     1、2、3
 */
int CKartingServer::GetLottery(vector<int> &v, int nJetton)
{
    v.clear();
    int nStock = GetRoomStock() + nJetton;
    int nStart = 1;
    int nEnd = 8;
    if (nStock < -8e7)
    {
        nStart = 7;
        nEnd = 8;
    }
    else if (nStock < 0)
    {
        nStart = 6;
        nEnd = 8;
    }
    else if (nStock < 8e7)
    {
        nStart = 5;
        nEnd = 8;
    }
    else if (nStock < 3e8)
    {
        nStart = 1;
        nEnd = 8;
    }
    else if (nStock < 3.8e8)
    {
        nStart = 1;
        nEnd = 5;
    }
    else if (nStock < 4.6e8)
    {
        nStart = 1;
        nEnd = 4;
    }
    else
    {
        nStart = 1;
        nEnd = 3;
    }
    
    string s = "";
    char sz[3] = "";
    for (int i = nStart; i <= nEnd; i++)
    {
        v.push_back(i);
        snprintf(sz, sizeof(sz), "%d ", i);
        s.append(sz);
    }
    logDebug("总下注[%d] 当前调节池[%d] 开奖集合[%s]", nJetton, nStock, s.c_str());
    return 0;
}

CKartingTable* CKartingServer::GetTableByTableId(int nTableId)
{
	MAP_TABLE::iterator it = m_mTable.find(nTableId);
	if (it != m_mTable.end())
		return it->second;
	
	return nullptr;
}

CKartingTable* CKartingServer::CreateNewTable(CGameTable * pTable)
{
	int nTableId = pTable->GetId();
	
	CKartingTable* pTableData = dynamic_cast<CKartingTable*>(pTable->GetTableData());
	if (!pTableData)
	{
		logDebug("创建新桌子失败!");
		return nullptr;
	}
	m_mTable.insert(MAP_TABLE::value_type(nTableId, pTableData));
	pTableData->m_nTableId = nTableId;

	logDebug("创建新桌子[%d] 当前桌子数[%d]", nTableId, m_mTable.size());

	pTableData->Init();
	return pTableData;
}

