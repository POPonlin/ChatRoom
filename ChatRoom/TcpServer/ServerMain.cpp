#include "TcpServer.h"

void DealWithData(MSGHead* msgHead) { }

class ChatRoom
{
#define MAX_CHATROOM_MEMBER 50
public:
	//房间中的客户端表
	std::map<SOCKET, ClientObject*> roomMbr;
	//房间id
	int roomID = -1;

	/// <summary>
	/// 向房间中添加客户端
	/// </summary>
	/// <param name="client"></param>
	void AddRoomMbr(ClientObject* client)
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (roomMbr.size() < MAX_CHATROOM_MEMBER)
		{
			roomMbr[client->GetSocket()] = client;
		}
	}

	/// <summary>
	/// 在房间中删除客户端
	/// </summary>
	/// <param name="client"></param>
	void DeleteRoomMbr(ClientObject* client)
	{
		//std::lock_guard<std::mutex> lock(mutex);
		if (roomMbr.find(client->GetSocket()) != roomMbr.end())
		{
			roomMbr.erase(client->GetSocket());
		}
	}

	/// <summary>
	/// 将消息广播到房间中所有客户端
	/// </summary>
	/// <param name="client"></param>
	/// <param name="msgHead"></param>
	void SendData2All(ClientObject* client, MSGHead* msgHead)
	{
		std::lock_guard<std::mutex> lock(mutex);
		for (auto& member : roomMbr)
		{
			if (member.first != client->GetSocket())
			{
				member.second->SendData(msgHead);
			}
		}
	}

	ChatRoom& operator=(const ChatRoom& room)
	{
		if (this != &room)
		{
			// 复制房间成员
			this->roomMbr = room.roomMbr;
			this->roomID = room.roomID;
		}
		return *this;
	}
private:
	std::mutex mutex;
};

class RoomManger
{
public:
	/// <summary>
	/// 多线程下懒汉式单例
	/// </summary>
	/// <returns></returns>
	static RoomManger* GetInstance()
	{
		//双重锁定
		if (!instance)
		{
			std::lock_guard<std::mutex> lock(mutex); // 进入临界区
			if (!instance)
			{
				instance.reset(new RoomManger);
			}
		}
		return instance.get();
	}

	/// <summary>
	/// 回收roomID
	/// </summary>
	/// <param name="id"></param>
	void RecycleRoomID(int id)
	{
		std::lock_guard<std::mutex> lock(mutex);
		roomIDQue.push(id);
	}

	/// <summary>
	/// 获得一个roomID
	/// </summary>
	/// <returns></returns>
	int GetRoomID()
	{
		std::lock_guard<std::mutex> lock(mutex);
		int tmp = roomIDQue.front();
		roomIDQue.pop();
		return tmp;
	}

	/// <summary>
	/// 初始化roomID资源
	/// </summary>
	void InitRes()
	{
		for (int i = 1; i <= 1000; i++)
		{
			roomIDQue.push(i);
		}
	}
private:
	static void Init()
	{
		instance.reset(new RoomManger);
	}
	static std::queue<int> roomIDQue;
	//独立智能指针，保证始终只有一份引用
	static std::unique_ptr<RoomManger> instance;

	static std::mutex mutex;

	RoomManger() {}
	RoomManger(const RoomManger& tmp) = delete;
	RoomManger& operator=(const RoomManger& tmp) = delete;
};
//类外定义，分配空间
std::queue<int> RoomManger::roomIDQue;
std::unique_ptr<RoomManger> RoomManger::instance;
std::mutex RoomManger::mutex;

class MyTcpServer : public TcpServer
{
public:
	/// <summary>
	/// 显示房间信息
	/// </summary>
	/// <param name="client"></param>
	/// <param name="msgHead"></param>
	void ShowRoom(ClientObject* client, MSGHead* msgHead)
	{
		ShowInfoMSG showInfo;
		showInfo.roomNumber = (int)roomMap.size();
		int i = 0;
		{
			std::lock_guard<std::mutex> lock(mutex);
			for (auto& room : roomMap)
			{
				showInfo.rooms[i].roomID = room.second.roomID;
				showInfo.rooms[i].onLineNumber = (int)room.second.roomMbr.size();
				showInfo.rooms[i].maxNumber = MAX_CHATROOM_MEMBER;
				i++;
			}
		}
		client->SendData((MSGHead*)&showInfo);
	}

	/// <summary>
	/// 创建房间，并添加第一个客户端
	/// </summary>
	/// <param name="client"></param>
	void CreatRoom(ClientObject* client)
	{
		RoomManger* instance = RoomManger::GetInstance();
		instance->InitRes();
		int roomID = instance->GetRoomID();

		std::shared_ptr<ChatRoom> cr(new ChatRoom);
		cr->roomID = roomID;
		cr->AddRoomMbr(client);

		{
			std::lock_guard<std::mutex> lock(mutex);
			roomMap[roomID] = *cr;
		}

		CreatInfoMSG creatInfo;
		creatInfo.roomInfo.roomID = roomID;
		creatInfo.roomInfo.onLineNumber = 1;
		creatInfo.roomInfo.maxNumber = MAX_CHATROOM_MEMBER;
		client->SendData((MSGHead*)&creatInfo);
	}

	/// <summary>
	/// 加入房间
	/// </summary>
	/// <param name="client"></param>
	/// <param name="msgHead"></param>
	void JoinRoom(ClientObject* client, MSGHead* msgHead)
	{
		JoinMSG* joinMsg = (JoinMSG*)msgHead;
		{
			std::lock_guard<std::mutex> lock(mutex);
			auto iter = roomMap.find(joinMsg->roomID);
			if (iter != roomMap.end())
			{
				iter->second.AddRoomMbr(client);
			}
		}
		//进入ShowRoom前要先释放mutex
		ShowRoom(client, msgHead);
	}

	/// <summary>
	/// 向某房内全部客户端广播
	/// </summary>
	/// <param name="client"></param>
	/// <param name="msgHead"></param>
	void Talking(ClientObject* client, MSGHead* msgHead)
	{
		TalkMSG* talkMsg = (TalkMSG*)msgHead;
		std::lock_guard<std::mutex> lock(mutex);
		auto iter = roomMap.find(talkMsg->roomID);
		if (iter != roomMap.end())
		{
			talkMsg->userID = (int)client->GetSocket();
			iter->second.SendData2All(client, msgHead);
		}
	}

	/// <summary>
	/// 离开房间
	/// </summary>
	/// <param name="client"></param>
	/// <param name="msgHead"></param>
	void LeaveRoom(ClientObject* client, MSGHead* msgHead)
	{
		LeaveMSG* leaveMsg = (LeaveMSG*)msgHead;
		std::lock_guard<std::mutex> lock(mutex);
		auto iter = roomMap.find(leaveMsg->roomID);
		if (iter != roomMap.end())
		{
			iter->second.SendData2All(client, msgHead);
			iter->second.DeleteRoomMbr(client);
		}

	}

	/// <summary>
	/// 消息处理
	/// </summary>
	/// <param name="client"></param>
	/// <param name="msgHead"></param>
	void MSGPross(ClientObject* client, MSGHead* msgHead)
	{
		switch (msgHead->msgType)
		{
		case SHOW_MSG:
			printf("请求显示全部聊天室信息...\n");
			ShowRoom(client, msgHead);
			break;
		case JOIN_MSG:
		{
			JoinMSG* join = (JoinMSG*)msgHead;
			printf("请求加入 %d 聊天室...\n", join->roomID);
			JoinRoom(client, msgHead);
		}
		break;
		case CREAT_MSG:
			printf("请求创建聊天室...\n");
			CreatRoom(client);
			break;
		case TALK_MSG:
		{
			TalkMSG* talk = (TalkMSG*)msgHead;
			printf("%s...\n", talk->GetMbuff());
			Talking(client, msgHead);
		}
		break;
		case LEAVE_MSG:
		{
			LeaveMSG* leave = (LeaveMSG*)msgHead;
			printf("请求离开聊天室 %d ...\n", leave->roomID);
			LeaveRoom(client, msgHead);
		}
		break;
		default:
			printf("信息解析失败...\n");
		}
	}
private:
	std::mutex mutex;
	std::map<int, ChatRoom> roomMap;
};

int main()
{
	MyTcpServer tcpServer;
	//绑定
	tcpServer.Bind(9999);
	tcpServer.Listen();
	tcpServer.StartThread(3);
	while (true)
	{
		tcpServer.ListenUpdate();
	}

	while (true);

	return 0;
}