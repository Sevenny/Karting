#include "KartingUser.h"
#include "KartingComm.h"
#include "KartingServer.h"
#include "KartingTable.h"

#define TIMERID_PLAYER_QUIT     1

CKartingUser::CKartingUser()
:CGameUser::CUserData()
,CEventObject()
,m_tQuitTimer()
{
	m_nStatus = 0;
	m_nTotal = 0;
	m_nWin = 0;
	m_bIsReconnected = false;
    m_nLimit = 0;
    m_nTotalWin = 0;
    
    m_tQuitTimer.init(TIMERID_PLAYER_QUIT, this);

	for (int i = 0; i < 9; i++)
		m_nJetton[i] = 0;
}

int CKartingUser::ProcessEvent(int nTimerId)
{
    switch (nTimerId) {
        case TIMERID_PLAYER_QUIT:
            OnQuitTimeout();
            break;
            
        default:
            break;
    }
    return 0;
}

void CKartingUser::Reset()
{
	m_nStatus = 0;
	m_nTotal = 0;
	m_nWin = 0;
	m_bIsReconnected = false;
    m_nLimit = 0;

	for (int i = 0; i < 9; i++)
		m_nJetton[i] = 0;
}

CKartingUser::~CKartingUser()
{
    StopQuitTimer();
}

const char* CKartingUser::UserInfo()
{
    string s = "";
    char info[20] = "";
    
    snprintf(info, sizeof(info), "%d_%s", m_pUser->GetUserId(), m_pUser->IsAndroid() ? "R" : "H");
    //snprintf(info, sizeof(info), "%d_%d", m_pUser->GetUserId(), m_pUser->GetSeatId());
    s.append(info);
    
    return s.c_str();
}

int CKartingUser::GetBettingLimit()
{
	int nGold = m_pUser->GetGold();

	int nLimit = (nGold / 1000) * 1000;
    nLimit -= GetTotal();
    
    //logDebug("[%s]nlimit[%d]", UserInfo(), nLimit);

	return nLimit;
}

void CKartingUser::OnQuitTimeout()
{
    StopQuitTimer();
    CKartingServer::Instance()->GetTableByTableId(m_pUser->GetTableId())->KitoutPlayer(m_pUser);
}

void CKartingUser::StartQuitTimer(int nDuration)
{
    m_tQuitTimer.StartTimer(nDuration);
}

void CKartingUser::StopQuitTimer()
{
    m_tQuitTimer.StopTimer();
}


