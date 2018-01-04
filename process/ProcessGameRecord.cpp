#include "ProcessGameRecord.h"
#include "log.h"
#include "KartingComm.h"
#include "KartingTable.h"
#include "KartingServer.h"


///返回1，说明处理成功，但是数据没有完整，需要从别的地方请求数据，这时需要保存现场
///返回0，代表处理成功完成,若客户端设置了 errorcode 的值不为0，则框架会将这两个值和strmsg回复给客户端
///返回-1，处理失败，这时框架会断开游戏服跟代理的链路
int CProcessGameRecord::DoRequest(INPUT_MSG& msg)
{
	
	CKartingTable* pTable = CKartingServer::Instance()->GetTableByTableId(msg.pUser->GetTableId());
	if (!pTable)
	{
		logError("***获取桌子[%d]信息失败", msg.pUser->GetTableId());
		return -1;
	}

	return pTable->OnGetGameRecordList(msg.pUser);
}

///返回1，说明处理成功，但是数据没有完整，需要从别的地方请求数据，这时需要保存现场
///返回0，代表处理成功完成
///返回-1，处理失败
int CProcessGameRecord::DoResponse(INPUT_MSG&)
{
	return 0;
}
