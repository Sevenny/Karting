#ifndef __REDIS_POOL_H__
#define __REDIS_POOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <list>
#include <vector>
#include <queue>
#include <map>
#include <string>
#include "hiredis/hiredis.h"

using namespace std;


class RedisClient {
public:
    redisReply          *reply;
    redisContext        *context;
public:
    RedisClient();
    virtual ~RedisClient();
	 
    int init(std::string host, int port, int timeout, std::string pass);
	int connect_redis();
	void deinit();
    int command(const char *format, ...);
	int command_spec(const char *format, ...);
	int is_array_return_ok();
    char* get_value_as_string(const char *key);
	char* get_value_as_string();
    int get_value_as_int(const char *key);
    long long get_value_as_int64(const char *key);
    float get_value_as_float(const char*key);

	//Hash相关操作
	int HGetAll(const string& strKey, map<string, string>& mapValue);
	int HGet(const string& strKey, const string& strField, string& strValue);
	int HMGet(const string& strKey,vector<string> fields ,map<string, string>& mapValue);

	//判断key是否存在
	int key_exists(const char *format, ...);
private:
    std::string			_host;
    int                 _port;
	int					_timeout;
	std::string			_pass;
};

#endif
