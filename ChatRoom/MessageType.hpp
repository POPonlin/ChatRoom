#ifndef _MSGTYPE_HPP_
#define _MSGTYPE_HPP_

#define MAX_ROOM_NUM 50
#define MAX_PACKAGE_SIZE 1024
#define MAX_MSG_LEN 1000

enum MSG_TP
{
	NULL_MSG = 0,
	SHOW_MSG,
	SHOW_INFO_MSG,
	CREAT_MSG,
	CREAT_INFO_MSG,
	JOIN_MSG,
	TALK_MSG,
	LEAVE_MSG
};

/// <summary>
/// 头信息
/// </summary>
class MSGHead
{
public:
	MSG_TP msgType;
	int msgLen;
};

/// <summary>
/// 显示信息类
/// </summary>
class ShowMSG : MSGHead
{
public:
	ShowMSG()
	{
		msgType = SHOW_MSG;
		msgLen = sizeof(ShowMSG);
	}
};

/// <summary>
/// 显示具体信息类
/// </summary>
class ShowInfoMSG : MSGHead
{
public:
	int roomNumber;
	struct RoomInfo
	{
		int roomID;
		int onLineNumber;
		int maxNumber;
	};
	RoomInfo rooms[MAX_ROOM_NUM];

	ShowInfoMSG()
	{
		msgType = SHOW_INFO_MSG;
		msgLen = sizeof(ShowInfoMSG);
	}
};

/// <summary>
/// 创建信息类
/// </summary>
class CreatMSG : MSGHead
{
public:
	CreatMSG()
	{
		msgType = CREAT_MSG;
		msgLen = sizeof(CreatMSG);
	}
};

/// <summary>
/// 显示创建信息类
/// </summary>
class CreatInfoMSG : MSGHead
{
public:
	struct RoomInfo
	{
		int roomID;
		int onLineNumber;
		int maxNumber;
	};
	RoomInfo roomInfo;
	CreatInfoMSG()
	{
		msgType = CREAT_INFO_MSG;
		msgLen = sizeof(CreatInfoMSG);
	}
};

/// <summary>
/// 加入信息类
/// </summary>
class JoinMSG : MSGHead
{
public:
	int roomID;
	explicit JoinMSG(int _roomID) : roomID(_roomID)
	{
		msgType = JOIN_MSG;
		msgLen = sizeof(JoinMSG);
	}
};

/// <summary>
/// 交流信息类
/// </summary>
class TalkMSG : MSGHead
{
public:
	int roomID;
	int userID;
	explicit TalkMSG(int _roomID) : roomID(_roomID), userID(0)
	{
		msgType = TALK_MSG;
		msgLen = sizeof(TalkMSG);
	}

	char* GetMbuff()
	{
		return mbuff;
	}
private:
	char mbuff[MAX_MSG_LEN];
};

/// <summary>
/// 离开信息类
/// </summary>
class LeaveMSG : MSGHead
{
public:
	int roomID = -1;
	int userID = -1;
	LeaveMSG()
	{
		msgType = LEAVE_MSG;
		msgLen = sizeof(LeaveMSG);
	}
};
#endif