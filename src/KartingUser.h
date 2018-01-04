#ifndef _KARTING_USER_H_
#define _KARTING_USER_H_

#include "GameUser.h"
#include "Timer.h"

class CKartingUser:public CGameUser::CUserData, public CEventObject
{
public:
	CKartingUser();
	~CKartingUser();

	int ProcessEvent(int nTimerId);

public:
	void Reset();

	void AddJetton(int nIndex, int nJetton){ m_nJetton[nIndex] += nJetton;}
	int  GetJetton(int nIndex){return m_nJetton[nIndex];}
	void AddTotal(int n) {m_nTotal += n;}
	int  GetTotal(){return m_nTotal;}
	void AddWin(int n) {m_nWin += n;}
	int  GetWin(){return m_nWin;}
    void AddTotalWin(int n) {m_nTotalWin += n;}
    int  GetTotalWin() {return m_nTotalWin;}
    void SetBettingLimit(int n) {m_nLimit = n;}
    int  GetBettingLimit();// {return m_nLimit;}

	void SetReconnected(bool b) {m_bIsReconnected = b;}
	bool IsReconnected(){return m_bIsReconnected;}
    
    const char* UserInfo();

	void OnQuitTimeout();
    void StartQuitTimer(int nDuration);
    void StopQuitTimer();

private:
	int m_nStatus;
	int m_nJetton[9];
	int m_nTotal;	        //八个下注区总数
	int m_nWin;             //本局输赢
    int m_nTotalWin;        //输赢总计
    int m_nLimit;           //还能下注上限

	bool m_bIsReconnected;  //是否是重连
    
    CTimer m_tQuitTimer;
	
};

#endif
