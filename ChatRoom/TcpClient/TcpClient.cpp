#include "TcpClient.h"

TcpClient::TcpClient()
{
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == SOCKET_ERROR)
	{
		printf("监听套接字创建失败...\n");
		return;
	}
}

TcpClient::~TcpClient()
{
	closesocket(sClient);
}

void TcpClient::ConnectServer(const char* ip, unsigned short port)
{
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &saddr.sin_addr);
	int ret = connect(sClient, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret == SOCKET_ERROR)
	{
		printf("连接服务器失败...\n");
		return;
	}
}

void TcpClient::RecvData()
{
	while (true)
	{
		char buff[MAX_PACKAGE_SIZE] = { 0 };
		FD_SET fdRead;
		FD_ZERO(&fdRead);
		FD_SET(sClient, &fdRead);
		timeval time = { 0, 1 };
		int ret = select(0, &fdRead, nullptr, nullptr, &time);
		if (ret > 0)
		{
			int rec = recv(sClient, buff, MAX_PACKAGE_SIZE, 0);
			if (rec > 0)
			{
				//正常接收到信息，进一步处理信息
				MSGPross((MSGHead*)buff);
			}
		}
	}
}

void TcpClient::SendData(MSGHead* msgHead)
{
	FD_SET fdSend;
	FD_ZERO(&fdSend);
	FD_SET(sClient, &fdSend);
	const timeval time = { 0, 1 };
	int ret = select(0, nullptr, &fdSend, nullptr, &time);
	if (ret > 0)
	{
		send(sClient, (const char*)msgHead, msgHead->msgLen, 0);
	}
	else
	{
		printf("客户端发送信息失败...\n");
		return;
	}
}

void TcpClient::MSGPross(MSGHead* msgHead)
{
	switch (msgHead->msgType)
	{
	case SHOW_INFO_MSG:
	{
		ShowInfoMSG* showInfo = (ShowInfoMSG*)msgHead;
		int num = showInfo->roomNumber;
		printf("———————————房间信息———————————\n");
		for (int i = 0; i < num; ++i)
		{
			printf("房间ID(RoomID): %d ( 在线人数: %d / 可容纳人数: %d )\n", showInfo->rooms[i].roomID, showInfo->rooms[i].onLineNumber, showInfo->rooms[i].maxNumber);
		}
		printf("————————————END————————————\n");
		break;
	}
	case CREAT_INFO_MSG:
	{
		CreatInfoMSG* creatInfo = (CreatInfoMSG*)msgHead;
		printf("————————新创建房间信息————————\n");
		printf("房间ID(RoomID): %d ( 在线人数: %d / 可容纳人数: %d )\n", creatInfo->roomInfo.roomID, creatInfo->roomInfo.onLineNumber, creatInfo->roomInfo.maxNumber);
		printf("————————————END————————————\n");
		break;
	}
	case TALK_MSG:
	{
		TalkMSG* talk = (TalkMSG*)msgHead;
		printf("————来自 %d 房间 %d 用户的消息————\n", talk->roomID, talk->userID);
		printf("%s...\n", talk->GetMbuff());
		printf("————————————END————————————\n");
		break;
	}
	case LEAVE_MSG:
	{
		LeaveMSG* leave = (LeaveMSG*)msgHead;
		printf("——用户 %d 已经离开 %d 房间——\n", leave->userID, leave->roomID);
		break;
	}
	default:
		break;
	}
}

SOCKET TcpClient::GetSocket()
{
	return sClient;
}