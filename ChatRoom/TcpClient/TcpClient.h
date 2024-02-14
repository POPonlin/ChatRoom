#pragma once
#include <stdio.h>
#include <thread>
#include <vector>
#include "../InitSocket.hpp"
#include "../MessageType.hpp"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma warning(disable:4996)
class TcpClient
{
public:
	int roomID = 0;
	TcpClient();
	virtual ~TcpClient();
	void ConnectServer(const char* ip, unsigned short port);
	void RecvData();
	void SendData(MSGHead* msgHead);
	void MSGPross(MSGHead* msgHead);
	SOCKET GetSocket();
private:
	InitSocket initSocket;
	SOCKET sClient;
};