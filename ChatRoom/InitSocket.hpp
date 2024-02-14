#ifndef _SOCKET_INIT_HPP_
#define _SOCKET_INIT_HPP_

#include <stdio.h>
#include <Winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class  InitSocket
{
public:
	InitSocket()
	{
		//选择winsock2.2
		WORD sockV = MAKEWORD(2, 2);
		//用于接收关于winsock信息
		WSADATA wsaData;
		//初始化winsock库
		if (WSAStartup(sockV, &wsaData) != 0)
		{
			printf("初始化socket环境失败");
		}
	}

	~InitSocket()
	{
		//释放winsock库资源
		WSACleanup();
	}
};
#endif
