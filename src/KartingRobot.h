#ifndef _KARTING_ROBOT_H_
#define _KARTING_ROBOT_H_
#include "AndroidUser.h"

class CKartingTable;
class CKartingRobot :  public CAndroidUser
{
public:
	CKartingRobot();
	virtual ~CKartingRobot();

	int OnEventGameMessage(const OUTPUT_MSG& msg);

	int ProcessEvent(int nTimerId);

	int OnGameStart(const OUTPUT_MSG& msg);
	int OnNotifyZhuang(const OUTPUT_MSG & msg);
    int OnNotifyBetting(const OUTPUT_MSG & msg);
    int OnBettingResponse(const OUTPUT_MSG & msg);
    
    int RobotBetting(CKartingTable* pTable);
    
    void StartTimer(int nTimerId, int nDuration, bool isLoop = false);
    void StopTimer(int nTimerId);
    void OnBettingTimeout();
    
private:
    CTimer m_tBettingTimer;
};

#endif
