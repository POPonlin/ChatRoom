#include "TcpClient.h"

void ThreadProc(TcpClient* sClient)
{
	printf("show create join talk leave exit\n");
	while (true)
	{
		char buff[1024] = { 0 };
		gets_s(buff, 1024);
		if (strcmp(buff, "show") == 0)
		{
			ShowMSG show;
			sClient->SendData((MSGHead*)&show);
		}
		else if (strncmp(buff, "join", strlen("join")) == 0)
		{
			char* joinParam = buff + strlen("join");
			int r = atoi(joinParam);
			if (r != 0)
			{
				JoinMSG join(r);
				sClient->SendData((MSGHead*)&join);
				sClient->roomID = r;
			}
		}
		else if (strcmp(buff, "create") == 0)
		{
			CreatMSG creat;
			sClient->SendData((MSGHead*)&creat);
		}
		else if (strcmp(buff, "talk") == 0)
		{
			printf("开始聊天\n");
			while (true)
			{
				TalkMSG talk(sClient->roomID);
				gets_s(talk.GetMbuff(), 1000);
				if (strcmp(talk.GetMbuff(), "leave") == 0)
				{
					printf("聊天结束\n");
					LeaveMSG leave;
					leave.roomID = talk.roomID;
					leave.userID = (int)sClient->GetSocket();
					sClient->SendData((MSGHead*)&leave);
					break;
				}
				sClient->SendData((MSGHead*)&talk);
			}
		}
		else if (strcmp(buff, "exit") == 0)
		{
			break;
		}
	}
}

#define TCPCLIENT_NUM 1

int main()
{
	TcpClient client;
	client.ConnectServer("192.168.0.106", 9999);

	// 创建新线程执行client.RecvData()
	std::thread recvThread([&client]
		{
			while (true)
			{
				client.RecvData();
			}
		});

	// 执行用户输入逻辑
	ThreadProc(&client);

	// 主线程阻塞等待接收线程结束
	recvThread.join();

	return 0;
}