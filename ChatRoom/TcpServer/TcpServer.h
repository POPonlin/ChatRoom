#pragma once
#include <vector>
#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "../InitSocket.hpp"
#include "../MessageType.hpp"

class ThreadRecv;

/// <summary>
/// 客户端类
/// </summary>
class ClientObject
{
public:
	ClientObject();
	//虚析构，保证调用正确的析构序列
	virtual ~ClientObject();
	SOCKET GetSocket();
	void SetSocket(SOCKET _fd);
	void MSGPross(ThreadRecv* thread, MSGHead* msgHead);
	char* GetBuff();
	int GetLastPos();
	void SetLastPos(int pos);
	void SendData(MSGHead* msgHead);
private:
	//通信套接字
	SOCKET cfd;
	//接收缓存
	char cbuff[MAX_PACKAGE_SIZE * 2] = { 0 };
	//记录缓存中最后位置
	int lastPos = 0;
};

class TcpServer;

/// <summary>
/// 接收线程类
/// </summary>
class ThreadRecv
{
public:
	TcpServer* baseServer = nullptr;
	static int isRun;
	void Start();
	ThreadRecv();
	~ThreadRecv();
	static void ThreadPro(ThreadRecv* threadRecv);

	int GetClientMapSize();
	void Add2ClientCache(ClientObject* client);
	void setBaseServer(TcpServer* _baseServer);
private:
	//缓存添加进的客户端
	std::vector<ClientObject*> clientCache;
	//文件描述符和客户端的映射表
	std::map<SOCKET, ClientObject*> clientMap;
	//文件描述符集合，负责读缓冲区
	FD_SET fdRecv_ThreadRecv;
	//判断客户端列表是否发生变化
	int clientChange;
	std::mutex mutex;
};

/// <summary>
/// 服务器类
/// </summary>
class TcpServer
{
public:
	int acceptNumber;
	TcpServer();
	virtual ~TcpServer();

	void Bind(unsigned short port);
	void Listen();
	void ListenUpdate();
	void StartThread(int num);
	virtual void MSGPross(ClientObject* client, MSGHead* msgHead);
private:
	//初始化服务
	InitSocket socketInit;
	//监听套接字
	SOCKET fdListen;
	FD_SET fdRecv_TcpServer;
	//储存建立的线程
	ThreadRecv* threadRecv;
	//存储连接的所有客户端
	std::vector<ClientObject*> clientVec;
	//建立的线程数
	int threadNumber;
};
