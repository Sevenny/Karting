#ifndef _KARTING_SERVER_H
#define _KARTING_SERVER_H
#include "KartingComm.h"
#include "LoadServer.h"
#include "RedisClient.h"

class CKartingTable;
static int g_nCount = 0;
typedef std::map<int, CKartingTable*> MAP_TABLE;
class CKartingServer
{
public:
	static CKartingServer* Instance();
	CKartingServer();
	~CKartingServer();
    
public:
	int init();
	int ReadConf(const char* szFile = "./config/karting.ini");
	int ReadRedis();

	CKartingTable* GetTableByTableId(int nTableId);
	CKartingTable* CreateNewTable(CGameTable* pTable);
    
    void GetJetton(vector<int> &v);

	void SetGameType(int t) {m_nGameType = t;}
	int  GetGameType() {return m_nGameType;}

	void SetGameMod(int m) {m_nGameMod = m;}
	int  GetGameMod() {return m_nGameMod;}
    
    int  GetRobotMinGold(){return Options::Instance()->m_nAIMinGold;}
    int  GetRobotMaxGold(){return Options::Instance()->m_nAIMaxGold;}

	void SetMaxPlayerCount(int c) {m_nMaxPlayerCount = c;}
	int  GetMaxPlayerCount() {return m_nMaxPlayerCount;}

	void SetMinPlayerCount(int c) {m_nMinPlayerCount = c;}
	int  GetMinPlayerCount() {return m_nMinPlayerCount;}

	int GetWaitingTimeout() {return m_nWaitingTimeout;}
	int GetBettingTimeout() {return m_nBettingTimeout;}
	int GetLotteryTimeout() {return m_nLotteryTimeout;}
	int GetBalanceTimeout() {return m_nBalanceTimeout;}
	int GetShuishou() {return m_nShuishou;}
    int GetProbability(int &nIndex);
    int GetLottery(vector<int> &v, int nJetton);

	int GetRedisValue(int nKey);
	int GetBlackList(int nUserId);
    int GetWhiteList(int nUserId);

	int GetRoomStock() {return m_nRoomStock;}
	void SetRoomStock(int nStock) {m_nRoomStock = nStock;}
	int GetCurrentStock() {return m_nCurrentStock;}
	void SetCurrentStock(int nStock) {m_nCurrentStock = nStock;}

	//上下庄
	int GetZhuangMinGold(){return m_nZhuangMinGold;}
	int GetZhuangLowGold(){return m_nZhuangLowGold;}
	int GetZhuangRounds(){return m_nZhuangRounds;}
    int GetBettingLimit(){return m_nBettingLimit;}
    int GetRobotMin(){return m_nRobotMin;}
    int GetRobotMax(){return m_nRobotMax;}
    int GetThreshold(){return m_nThreshold;} //启动调控的下注阈值

public:
	RedisClient *m_pRedis;

protected:
	int m_nTableCost;//台费
	int m_nTableTax;//税率
	int m_nMaxPlayerCount;
	int m_nMinPlayerCount;
	int m_nGameType;
	int m_nGameMod;

	int m_nWaitingTimeout;
	int m_nBettingTimeout;
	int m_nLotteryTimeout;
    int m_nBalanceTimeout;
	int m_nShuishou;			//机器人的税收 1/1000

	//百人系列
	int m_nZhuangMinGold;		//最低上庄金币数
	int m_nZhuangLowGold;		//下庄下限金币数
	int m_nZhuangRounds;		//坐庄轮数
    int m_nBettingLimit;        //全局下注上限
    int m_nRobotMin;
    int m_nRobotMax;
    int m_nThreshold;           //调控阈值

	MAP_TABLE m_mTable;
    vector<int> m_vecJetton;    //下注选项

	int m_nRoomStock;	        //库存
	int m_nCurrentStock;	    //当前库存

	char m_szRedisHost[64];
	int m_nRedisPort;
	char m_szRedisPass[64];
};


#endif
