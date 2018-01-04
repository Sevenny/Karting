#include "KartingComm.h"
#include "daemon.h"
#include "KartingRobot.h"
#include "KartingTable.h"
#include "KartingServer.h"

class CGameLogic;

extern "C" {
int game_handle_init(int argc,char**argv)
{
	daemon(1,0);
	
    REGIST_CMD_PROCESS(Options::Instance()->m_nGameType,KARTING_CMD_BETTING, 	    CProcessBetting);
	REGIST_CMD_PROCESS(Options::Instance()->m_nGameType,KARTING_CMD_RECORD, 	    CProcessGameRecord);
	REGIST_CMD_PROCESS(Options::Instance()->m_nGameType,KARTING_CMD_ZHAUNG, 	    CProcessZhuang);
	REGIST_CMD_PROCESS(Options::Instance()->m_nGameType,KARTING_CMD_ZHUANG_LIST,    CProcessZhuangList);
    REGIST_CMD_PROCESS(Options::Instance()->m_nGameType,KARTING_CMD_USER_INFO,      CProcessUserInfo);

	CKartingServer::Instance()->init();

    return 0;
}  

int game_handle_ready(CGameTable *pTable, CGameUser *pUser)
{
    return 0;
}

//玩家进入桌子
int game_handle_sitdown(CGameTable *pTable, CGameUser *pUser)
{
    //logDebug("[%d_%s]进入桌子[%d] 金币[%d]", pUser->GetUserId(), pUser->IsAndroid()?"R":"H", pTable->GetId(), pUser->GetGold());
	//玩家进入游戏调用的第一个函数
	CKartingTable* pTableData = CKartingServer::Instance()->GetTableByTableId(pTable->GetId());

	if (!pTableData)
		pTableData = CKartingServer::Instance()->CreateNewTable(pTable);
    
    if (!pTableData)
    {
        logError("创建新桌子失败[%d]", pTable->GetId());
        return -1;
    }

	//把玩家同步到桌子里
	return pTableData->PlayerIntoRoom(pUser);
}

int game_handle_situp(CGameTable *pTable, CGameUser *pUser)
{
    //logDebug("[%d_%s]离开房间[%d] 金币[%d]", pUser->GetUserId(), pUser->IsAndroid()?"R":"H", pTable->GetId(), pUser->GetGold());
	
	CKartingTable* pTableData = CKartingServer::Instance()->GetTableByTableId(pTable->GetId());
	if (pTableData)
	{
		return pTableData->PlayerLeaveRoom(pUser->GetUserId());
	}
	
    return -1;
}

int game_handle_offline(CGameTable *pTable, CGameUser *pUser)
{
    //logDebug("玩家[%d]离线 当前状态[%s]", pUser->GetUserId(), EnumToString(pUser->GetStatus()));
	if (pTable->IsPlaying() && pUser->IsPlaying())
		return -1;
   
    return 0;
}

int game_handle_logout(CGameTable *pTable, CGameUser *pUser)
{
    //logDebug("桌子[%d] 玩家[%d]登出 当前状态[%s]\n", pTable->GetId(), pUser->GetUserId(), EnumToString(pUser->GetStatus()));
    if (pTable->IsPlaying() && pUser->IsPlaying())
   		return -1;
   
   return 0;
}

//返回玩家是否能动态加入,返回<0不允许动态加入,>=0可以动态加入
int game_candynamicjoin()
{
	return 0;
}

int game_handle_scene_recover(CGameTable *pTable, CGameUser *pUser)
{
	
	CKartingTable* pTableData = CKartingServer::Instance()->GetTableByTableId(pTable->GetId());
	if (pTableData)
	{	
		CKartingUser* p = pTableData->GetUserByUserId(pUser->GetUserId());
		if (p)
		{
			return pTableData->SendGameStatusToUser(p);
		}
		logError("获取玩家信息失败");
	}
	else
		logError("获取桌子信息失败");
	
    return 0;
}


void* game_handle_create_tabledata()
{
    return new CKartingTable();
}

void* game_handle_create_userdata()
{
    return new CKartingUser();
}

CAndroidUser* game_create_ai_user()
{
	return new CKartingRobot();
}

}
