#!/bin/sh

echo "#ifndef _PROCESS_USER_LOGIN_H_$1
#define _PROCESS_USER_LOGIN_H_$1

#include \"IProcess.h\"

class CProcess$1:public IProcess
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

#endif" > Process$1.h 


echo "#include \"Process$1.h\"
#include \"log.h\"

///返回1，说明处理成功，但是数据没有完整，需要从别的地方请求数据，这时需要保存现场
///返回0，代表处理成功完成,若客户端设置了 errorcode 的值不为0，则框架会将这两个值和strmsg回复给客户端
///返回-1，处理失败，这时框架会断开游戏服跟代理的链路
int CProcess$1::DoRequest(INPUT_MSG& msg)
{
	return 0;
}

///返回1，说明处理成功，但是数据没有完整，需要从别的地方请求数据，这时需要保存现场
///返回0，代表处理成功完成
///返回-1，处理失败
int CProcess$1::DoResponse(INPUT_MSG&)
{
	return 0;
}"  > Process$1.cpp


