#ifndef _KARTING_TABLE_H_
#define _KARTING_TABLE_H_

#include "GameTable.h"
#include "Timer.h"
#include "KartingComm.h"

class CGameLogic;
class CKartingUser;

class CKartingTable : public CGameTable::CTableData, public CEventObject
{
public:
	typedef int TimerId;			//定时器id
	typedef int Duration;			//定时器时长		
	typedef int TimeStamp;			//定时器开始时间戳
	typedef map<TimerId, map<Duration,TimeStamp> > MapTimer;

	typedef struct _zone
	{
		int nOdds;//输赢倍率
		int nIndex;
		int nGold;//下注总数
        int nLimit;//下注限制
        int nProbability;
        int nRank;

		void init()
		{
			nGold = 0;
            nLimit = 0;
            nRank = 0;
		}
	}BettingZone;
    
public:
	CKartingTable();
	~CKartingTable();

	int                 ProcessEvent(int nTimerId);
	void                Init();
	void                Reset();

public:
	int                 PlayerIntoRoom(CGameUser* pUser);                   //玩家进入房间
	int                 PlayerLeaveRoom(int nUserId);			            //玩家离开房间
	int                 PlayerBetting(int nUserId, int nIndex, int nGold);	//玩家下注
	void                NotifyRobotIntoRoom();          //通知机器人进房
    void                KitoutPlayer(CGameUser* p);

	int                 OnGameStart();	                //游戏开始
	int                 OnGameOver();	                //游戏结束
	int                 OnBalance(int nDuration);		//结算
    int                 GetWin(int nIndex);             //计算输赢

	int                 SendGameStatusToUser(CKartingUser* pUser);	            //发送游戏场景
	int                 OnGetGameRecordList(CGameUser* pUser);
	int                 OnCallZhuang(CKartingUser* pUser, int nType);           //玩家上下庄
	int                 OnGetZhuangList(CKartingUser* pUser);                   //玩家获取上庄列表
    int                 OnGetUserInfo(CKartingUser* pUser);                  //获取玩家数据
    int                 GetRobotBettingLimit();                             //获取机器人下注上限
    int                 GetRank();
    int                 GetIndexByRank(int r);
    
    const char*         GetZhuangList();
    int                 GetLotteryProbability();

	inline int          GetPlayerCnt(){return m_nPlayerCnt;}                //获取当前房间真实玩家数量
	inline int          GetRobotCnt(){return m_nRobotCnt;}			        //获取当前房间机器人数量
	inline int          GetRoomPlayerCnt() {return (int)m_mapPlayer.size();}//获取当前房间玩家总数
    inline int          GetZhuangListSize(){return (int)m_lZhuangList.size(); }
    inline int          GetZoneLimit(int i)
                        {
                            return (m_stBettingZone[i].nLimit - m_stBettingZone[i].nGold);
                        }
    
	CKartingUser*        GetUserByUserId(int nUserId);
    const char*         GetUserInfoByUserId(int nUserId);
    
    bool                IsRobot(int nUserId);
	bool                IsAllRobot();

	int                 StartTimer(int nTimerId, int nDuration, bool isLoop = false);
	int                 StopTimer(int nTimerId);
	int                 GetRemainTime(int nTimerId);		//获取定时器剩余时间

	void                OnLottery();
    void                OnBalanceTimeout();
	void                OnWaitZhuangTimeout();
    void                OnAddRobotTimeout();
    void                OnChangeZhuangTimeout();
    void                OnWatchDogTimeout();
    void                OnRobotBettingTimeout();

	void                InsertGameRecord(int nIndex);
	int                 GetNextZhuang();
    void                ChangeZhuang();
	int                 InsertZhuang(int nUserId);
    void                GetZhuangLimit();

public:
    CKartingUser*        m_pDealer;

	bool 				m_bStart;           //是否已启动等待上庄
    bool                m_bWatchDog;        //是否已启动看门狗

	int 				m_nTableId;
	int 				m_nStatus;
	int 				m_nPlayerCnt;
	int 				m_nRobotCnt;
    int                 m_nNeedRobotCnt;    //房间所需机器人数量
	int 				m_nDealerId;
    int                 m_nIndex;           //本局游戏开奖结果
    int                 m_nCoefficient;     //系数，作为开奖概率的分母
	int 				m_nZhuangRounds;    //坐庄局数
    int                 m_nZhuangTotalWin;  //庄家坐庄总输赢
    int                 m_nZhuangWin;       //庄家本局总输赢
    int                 m_nPlayerTotalChip; //所有玩家总下注数，不包含机器人
    int                 m_nTotalChip;       //所有玩家总下注数，包含机器人   
    int                 m_nRobotBettingLimit;//机器人下注上限
    int                 m_nVisiable;        //用于显示哪个按钮能亮

	map<int, CKartingUser*> m_mapPlayer;        //游戏玩家集合
	BettingZone 		m_stBettingZone[9]; //下注区域
    int                 m_nRobotBetting[9]; //八个区域机器人总下注数1-8
    int                 m_nPlayerBetting[9];//八个区域普通玩家总下注数1-8
	MapTimer 			m_mTimer;           //定时器集合
	list<int>           m_lGameRecord;      //游戏列表
	list<int>			m_lZhuangList;      //上庄玩家列表

//定时器
protected:
	CTimer              m_tWaitingTimer;
	CTimer              m_tBettingTimer;
	CTimer              m_tLotteryTimer;
    CTimer              m_tBalanceTimer;
    CTimer              m_tWaitZhuangTimer;
    CTimer              m_tAddRobotTimer;
    CTimer              m_tChangeZhuangTimer;
    CTimer              m_tWtachDogTimer;       //轮询定时器，处理一些轮询事务
    CTimer              m_tRobotBettingTimer;   //通知机器人下注
};

#endif
