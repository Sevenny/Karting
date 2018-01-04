#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <list>
#include <queue>
#include <stdarg.h>
#include "RedisClient.h"
#include "KartingComm.h"

RedisClient::RedisClient()
{
    context = NULL;
	reply = NULL;
}

RedisClient::~RedisClient()
{
}

int RedisClient::init(std::string host, int port, int timeout, std::string pass)
{
    _host = host;
    _port = port;
	_timeout = timeout;
	_pass = pass;
	
    return connect_redis();
}

int RedisClient::connect_redis()
{
	struct timeval tv;
	tv.tv_sec = _timeout / 1000;
	tv.tv_usec = _timeout * 1000;
	if (context)
	{
		redisFree(context);
		context = NULL;		
	}

    context = redisConnectWithTimeout(_host.c_str(), _port, tv);
    if (context->err) {
        logError("redis[%s] host[%s] port[%d]", context->errstr, _host.c_str(), _port);
        return -1;
    }
	//logDebug("redis host[%s] port[%d] is connected succ", _host.c_str(), _port);
	
	int ret =  command_spec("auth %s", _pass.c_str());
	if (ret < 0) {
		logError("auth %s >> redis[%s] host[%s] port[%d]", _pass.c_str(), context->errstr, _host.c_str(), _port);
	}
	return 0;
}

void RedisClient::deinit()
{
	redisFree(context);
	context = NULL;
}

int RedisClient::command(const char *format, ...)
{
	int i = 200000;
    if (reply)
    {
        freeReplyObject(reply);
		reply = NULL;
    }
	
	
	while (i--)
	{
		va_list ap;
		va_start(ap, format);
	    reply = (redisReply*)redisvCommand(context, format, ap);
	    va_end(ap);
	    if (context->err)
	    {
	        logError("redis[%s] ip[%s] port[%d], reconnecting...",context->errstr, _host.c_str(), _port);
			int ret = connect_redis();
			if (ret < 0)
				return -1;
			continue;
	    }
		break;
	}

    if (reply->type == REDIS_REPLY_INTEGER)	
    	return reply->integer;
	
	return 0;
}

int RedisClient::command_spec(const char *format, ...)
{
    if (reply)
    {
        freeReplyObject(reply);
		reply = NULL;
    }
	
	va_list ap;
	va_start(ap, format);
    reply = (redisReply*)redisvCommand(context, format, ap);
    va_end(ap);
    if (context->err)
    {
		return -1;
	}
	
    return 0;
}

int RedisClient::is_array_return_ok()
{
	if (reply->type == 2)
	{
		if (reply->elements > 0)
		{
			return 0;
		}
		return -1;
	}
	
	return -1;
}

char* RedisClient::get_value_as_string(const char *key)
{
    size_t i = 0;
    while (i < (reply->elements))
    {
        if (!strcmp(key, reply->element[i]->str))
        {
            return reply->element[i + 1]->str;
        }
        i += 2;
    }

    logError("can't find key[%s]", key);
    return (char*)"nil";
}

char* RedisClient::get_value_as_string()
{
	return reply->element[0]->str;
}


int RedisClient::get_value_as_int(const char *key)
{
    size_t i = 0;
    while (i < (reply->elements))
    {
        if (!strcmp(key, reply->element[i]->str))
        {
            return ::atoi(reply->element[i + 1]->str);
        }
        i += 2;
    }

    logError("can't find key[%s]", key);
    return 0;
}

long long RedisClient::get_value_as_int64(const char *key)
{
    size_t i = 0;
    while (i < (reply->elements))
    {
        if (!strcmp(key, reply->element[i]->str))
        {
            return ::atoll(reply->element[i + 1]->str);
        }
        i += 2;
    }

    logError("can't find key[%s]", key);
    return 0;
}

float RedisClient::get_value_as_float(const char *key)
{
    size_t i = 0;
    while (i < (reply->elements))
    {
        if (!strcmp(key, reply->element[i]->str))
        {
            return ::atof(reply->element[i + 1]->str);
        }
        i += 2;
    }

    logError("can't find key[%s]", key);
    return 0;
}

int RedisClient::key_exists(const char * format,...)
{
	int nRet = -1;
	if (reply)
    {
        freeReplyObject(reply);
		reply = NULL;
    }
	
	va_list ap;
	va_start(ap, format);
    reply = (redisReply*)redisvCommand(context, format, ap);
    va_end(ap);
	if(context->err)
	{
		connect_redis();
		return nRet;
	}

	//key 不存在
	if(reply->type == REDIS_REPLY_INTEGER && reply->integer == 0)
	{
		return nRet;
	}
    return 0;
}


int RedisClient::HGetAll(const string& strKey, map<string, string>& mapValue)
{
	redisReply *pstReply;
	errno = 0;

	pstReply = (redisReply *)redisCommand(context, "HGETALL %s", strKey.c_str());
	if(NULL == pstReply)
	{
		logError("redisCommand return NULL");
		return -__LINE__;
	}
	if(REDIS_REPLY_ARRAY == pstReply->type)
	{
		size_t i = 0;
		string strKey, strValue;
		for(i = 0; (i + 1) < pstReply->elements; i+= 2)
		{
			strKey = pstReply->element[i]->str;
			strValue = pstReply->element[i+1]->str;
			mapValue[strKey] = strValue;
			logError("key:%s value:%s", strKey.c_str(), strValue.c_str());
		}
	}
	else
	{
		logError("redisCommand return type not right");
		freeReplyObject(pstReply);
		return -__LINE__;
	}
	freeReplyObject(pstReply);
	return 0;
}

int RedisClient::HGet(const string& strKey, const string& strField, string& strValue)
{
	redisReply *pstReply;
	
	pstReply = (redisReply *) redisCommand(context, "HGET %s %s",
	        strKey.c_str(), strField.c_str());
	if (NULL == pstReply || NULL == pstReply->str || REDIS_REPLY_STRING != pstReply->type)
	{
		if (pstReply)
		{
			//logError("redisCommand error:%s", pstReply->str);
			freeReplyObject(pstReply);
		}
		else
		{
			logError("redisCommand return NULL");
		}
		return -__LINE__;
	}

	strValue = pstReply->str;
	freeReplyObject(pstReply);

	return 0;
}

int RedisClient::HMGet(const string& strKey,vector<string> fields ,map<string, string>& mapValue)
{

	redisReply *pstReply;
	vector<const char*> vecArgv(fields.size() + 2);
	vector<size_t> vecArgvLen(fields.size() + 2);
	int j = 0;

	static char szHMGet[] = "HMGET";
	vecArgv[j] = szHMGet;
	vecArgvLen[j] = sizeof(szHMGet) - 1;
	++j;

	vecArgv[j] = strKey.c_str();
	vecArgvLen[j] = strKey.size();
	++j;

	for(vector<string>::const_iterator it = fields.begin(); it != fields.end(); ++it, ++j)
	{
		vecArgv[j] = it->c_str();
		vecArgvLen[j] = it->size();
	}

	pstReply = (redisReply *)redisCommandArgv(context, vecArgv.size(), &(vecArgv[0]), &(vecArgvLen[0]));
	if(NULL == pstReply)
	{
		logError("redisCommand return NULL");
		return -__LINE__;
	}
	if(REDIS_REPLY_ARRAY == pstReply->type)
	{
		size_t i = 0;
		string  strValue;
		for(i = 0; i < pstReply->elements; i++)
		{
			if(NULL == pstReply->element[i]->str)
			{
				continue;
			}
			strValue = pstReply->element[i]->str;
			mapValue[fields[i]] = strValue;
			logError("key:%s value:%s", strKey.c_str(), strValue.c_str());
		}
	}
	else
	{
		logError("redisCommand return type not right");
		freeReplyObject(pstReply);
		return -__LINE__;
	}
	freeReplyObject(pstReply);
	return 0;
}


