#ifndef _GAMEDEMO_SO_H_
#define _GAMEDEMO_SO_H_

#include "GameBase.h"
#include "GameTable.h"
#include "GameUser.h"
#include "AndroidUser.h"

extern "C"{
int	game_handle_init(int argc,char**argv);  
int	game_handle_ready(CGameTable*,CGameUser*);
int	game_handle_sitdown(CGameTable*,CGameUser*);
int	game_handle_situp(CGameTable*,CGameUser*);
int	game_handle_offline(CGameTable*,CGameUser*);
int game_handle_gamestart(CGameTable*,CGameUser*);
int	game_handle_logout(CGameTable*,CGameUser*);
int game_handle_scene_recover(CGameTable *pTable, CGameUser *pUser);
int game_candynamicjoin();
void* game_handle_create_tabledata();
void* game_handle_create_userdata();
CAndroidUser* game_create_ai_user();

}

#endif
