#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <semaphore.h>
#include "../lock/locker.h"

class connection_pool
{
public:
	MYSQL *GetConnection();				 //获取数据库连接
	bool ReleaseConnection(MYSQL *conn); //释放连接
	int GetFreeConn();					 //获取连接
	void DestroyPool();					 //销毁所有连接

	//单例模式
	static std::shared_ptr<connection_pool> GetInstance();

	void init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, unsigned int MaxConn); 
	
	connection_pool();
	~connection_pool();

private:
	unsigned int MaxConn;  //最大连接数
	unsigned int CurConn;  //当前已使用的连接数
	unsigned int FreeConn; //当前空闲的连接数

private:
	std::mutex mutex;
	std::list<MYSQL*> connList; //连接池
	sem_t reserve;

private:
	std::string url;			 //主机地址
	std::string Port;		 //数据库端口号
	std::string User;		 //登陆数据库用户名
	std::string PassWord;	 //登陆数据库密码
	std::string DatabaseName; //使用数据库名
};

class connectionRAII{

public:
	connectionRAII(MYSQL **con, std::shared_ptr<connection_pool> connPool);
	~connectionRAII();
	
private:
	MYSQL *conRAII;
	std::shared_ptr<connection_pool> poolRAII;
};

#endif