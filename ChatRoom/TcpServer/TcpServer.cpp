#include "TcpServer.h"

int ThreadRecv::isRun = 1;

ClientObject::ClientObject()
{
	cfd = INVALID_SOCKET;
	lastPos = 0;
}

ClientObject::~ClientObject()
{
	if (cfd != INVALID_SOCKET)
	{
		//关闭socket连接
		closesocket(cfd);
	}
}

/// <summary>
/// 处理消息
/// </summary>
/// <param name="thread"></param>
/// <param name="msgHead"></param>
void ClientObject::MSGPross(ThreadRecv* thread, MSGHead* msgHead)
{
	thread->baseServer->MSGPross(this, msgHead);
}

/// <summary>
/// 发送数据
/// </summary>
/// <param name="msgHead"></param>
void ClientObject::SendData(MSGHead* msgHead)
{
	int ret = send(cfd, (const char*)msgHead, msgHead->msgLen, 0);
	//if (ret > 0)
	//{
	//	printf("成功向客户端发送数据\n");
	//}
}

SOCKET ClientObject::GetSocket()
{
	return cfd;
}

void ClientObject::SetSocket(SOCKET _cfd)
{
	cfd = _cfd;
}

char* ClientObject::GetBuff()
{
	return cbuff;
}

int ClientObject::GetLastPos()
{
	return lastPos;
}

void ClientObject::SetLastPos(int pos)
{
	lastPos = pos;
}

TcpServer::TcpServer()
{
	acceptNumber = 0;
	threadNumber = 0;
	threadRecv = nullptr;
	fdListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//清空监听读缓冲区集合
	FD_ZERO(&fdRecv_TcpServer);
	if (fdListen == -1)
	{
		printf("监听套接字创建失败\n");
		return;
	}
}

TcpServer::~TcpServer()
{
	ThreadRecv::isRun = 0;
	closesocket(fdListen);
	//删除客户端列表
	delete[] & clientVec;
	//删除线程列表
	if (threadRecv != nullptr)
	{
		delete[] threadRecv;
	}
	clientVec.clear();
}

void TcpServer::Bind(unsigned short port)
{
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.S_un.S_addr = ADDR_ANY;
	int ret = bind(fdListen, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret == -1)
	{
		printf("监听套接字绑定失败\n");
		return;
	}
}

void TcpServer::Listen()
{
	int ret = listen(fdListen, 128);
	if (ret == -1)
	{
		printf("监听失败\n");
		return;
	}
	FD_SET(fdListen, &fdRecv_TcpServer);
}

//void TcpServer::ListenUpdate()
//{
//	struct sockaddr_in saddr;
//	int len = sizeof(saddr);
//	//新建一个通信套接字，此时不会阻塞
//	SOCKET ret = accept(fdListen, (struct sockaddr*)&saddr, &len);
//	if (ret == -1)
//	{
//		printf("连接失败\n");
//		return;
//	}
//
//	char ip[INET_ADDRSTRLEN];
//	inet_ntop(AF_INET, &(saddr.sin_addr), ip, INET_ADDRSTRLEN);
//	int port = ntohs(saddr.sin_port);
//	printf("客户端 PORT: %d IP: %s 已连接\n", port, ip);
//}

/// <summary>
/// 持续检测，是否有新连接
/// </summary>
void TcpServer::ListenUpdate()
{
	//创建备份，因为会频繁修改，保留原数据
	FD_SET fdTmp = fdRecv_TcpServer;
	//设置轮询检查时间为1s
	const timeval time = { 1, 0 };
	int ret = select(0, &fdTmp, nullptr, nullptr, &time);
	if (ret == 0)
	{
		//没有变化，休眠1秒后，继续检测
		Sleep(1);
		return;
	}
	for (int i = 0; i < (int)fdTmp.fd_count; ++i)
	{
		//找到监听套接字
		if (fdTmp.fd_array[i] == fdListen)
		{
			sockaddr_in saddr;
			int len = sizeof(saddr);
			//新建一个通信套接字，此时不会阻塞
			SOCKET ret = accept(fdListen, (struct sockaddr*)&saddr, &len);
			if (ret == -1)
			{
				printf("连接失败\n");
				return;
			}

			char ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &saddr.sin_addr, ip, INET_ADDRSTRLEN);
			int port = ntohs(saddr.sin_port);
			acceptNumber++;
			printf("客户端 PORT: %d IP: %s 已连接\n", port, ip);

			//新建一个客户端
			ClientObject* pcb = new ClientObject;
			pcb->SetSocket(ret);

			clientVec.push_back(pcb);

			//选择一个缓存客户端数量小的线程存储新连接的客户端
			ThreadRecv* tmp = threadRecv;
			for (int i = 1; i < threadNumber; ++i)
			{
				if (tmp->GetClientMapSize() > threadRecv[i].GetClientMapSize())
				{
					tmp = &threadRecv[i];
				}
			}
			tmp->Add2ClientCache(pcb);
			//将新建的通信套接字添加进文件描述符集合中，在下次select轮询中才能检测到
			FD_SET(ret, &fdRecv_TcpServer);
		}

	}
}

/// <summary>
/// 处理消息
/// </summary>
/// <param name="client"></param>
/// <param name="msgHead"></param>
void TcpServer::MSGPross(ClientObject* client, MSGHead* msgHead)
{

}

/// <summary>
/// 开启全部线程
/// </summary>
/// <param name="_threadNumber"></param>
void TcpServer::StartThread(int _threadNumber)
{
	threadNumber = _threadNumber;
	threadRecv = new ThreadRecv[_threadNumber];
	for (int i = 0; i < _threadNumber; ++i)
	{
		threadRecv[i].Start();
		threadRecv[i].setBaseServer(this);
	}
}

/// <summary>
/// 线程函数
/// </summary>
/// <param name="threadRecv"></param>
void ThreadRecv::ThreadPro(ThreadRecv* threadRecv)
{
	while (isRun)
	{
		//将缓存表中的元素移至映射表中
		if (!threadRecv->clientCache.empty())
		{
			std::lock_guard<std::mutex> lock(threadRecv->mutex);
			for (int i = (int)threadRecv->clientCache.size() - 1; i >= 0; --i)
			{
				threadRecv->clientMap[threadRecv->clientCache[i]->GetSocket()] = threadRecv->clientCache[i];
			}
			threadRecv->clientCache.clear();
			threadRecv->clientChange = true;
		}
		//将map中文件描述符添加进thread的fdset中
		if (threadRecv->clientChange)
		{
			int i = 0;
			for (auto& pair : threadRecv->clientMap)
			{
				threadRecv->fdRecv_ThreadRecv.fd_array[i] = pair.second->GetSocket();
				i++;
			}
			threadRecv->fdRecv_ThreadRecv.fd_count = i;
		}
		//处理接收信息
		FD_SET fdTmp = threadRecv->fdRecv_ThreadRecv;
		timeval time = { 0, 1 };
		int ret = select(0, &fdTmp, nullptr, nullptr, &time);
		if (ret > 0)
		{
			for (int i = 0; i < (int)threadRecv->fdRecv_ThreadRecv.fd_count; ++i)
			{
				auto iter = threadRecv->clientMap.find(fdTmp.fd_array[i]);
				char buff[MAX_PACKAGE_SIZE] = { 0 };
				if (iter != threadRecv->clientMap.end())
				{
					char* clientBuff = iter->second->GetBuff();
					int lastPos = iter->second->GetLastPos();
					int nLen = recv(fdTmp.fd_array[i], clientBuff + lastPos, MAX_PACKAGE_SIZE - lastPos, 0);
					if (nLen > 0)
					{
						MSGHead* phead = (MSGHead*)clientBuff;
						lastPos = lastPos + nLen;
						while (lastPos >= sizeof(MSGHead))
						{
							if (lastPos >= phead->msgLen)
							{
								iter->second->MSGPross(threadRecv, (MSGHead*)phead);
								memcpy(clientBuff, clientBuff + phead->msgLen, lastPos - phead->msgLen);
								lastPos -= phead->msgLen;
							}
							else
							{
								break;
							}
						}
						iter->second->SetLastPos(lastPos);
					}
					else
					{
						if (iter != threadRecv->clientMap.end())
						{
							threadRecv->clientMap.erase(iter);
						}
						threadRecv->clientChange = true;
						break;
					}
				}
			}
		}
	}
}

/// <summary>
/// 创建一个线程
/// </summary>
void ThreadRecv::Start()
{
	std::thread t(ThreadPro, this);
	if (t.joinable())
	{
		t.detach();
	}
}

ThreadRecv::ThreadRecv()
{
	clientChange = true;
}

ThreadRecv::~ThreadRecv()
{

}

/// <summary>
/// 得到客户端映射表大小
/// </summary>
/// <returns></returns>
int ThreadRecv::GetClientMapSize()
{
	std::lock_guard<std::mutex> lock(mutex);
	return (int)clientMap.size();
}

/// <summary>
/// 添加客户端缓存
/// </summary>
/// <param name="client"></param>
void ThreadRecv::Add2ClientCache(ClientObject* client)
{
	std::lock_guard<std::mutex> lock(mutex);
	clientCache.push_back(client);
}

/// <summary>
/// 设置线程所属的TcpServer
/// </summary>
/// <param name="tcpServer"></param>
void ThreadRecv::setBaseServer(TcpServer* tcpServer)
{
	this->baseServer = tcpServer;
}