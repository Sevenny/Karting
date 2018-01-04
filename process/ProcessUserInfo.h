#ifndef _PROCESS_USER_LOGIN_H_UserInfo
#define _PROCESS_USER_LOGIN_H_UserInfo

#include "IProcess.h"

class CProcessUserInfo:public IProcess
{
public:
	///返回1，说明处理成功，但是数据没有完整，需要从别的地方请求数据，这时需要保存现场
	///返回0，代表处理成功完成,若客户端设置了 errorcode 的值不为0，则框架会将这两个值和strmsg回复给客户端
	///返回-1，处理失败，这时框架会断开游戏服跟代理的链路
	virtual int DoRequest(INPUT_MSG&);

	///返回1，说明处理成功，但是数据没有完整，需要从别的地方请求数据，这时需要保存现场
	///返回0，代表处理成功完成
	///返回-1，处理失败
	virtual int DoResponse(INPUT_MSG&);	
};

#endif
