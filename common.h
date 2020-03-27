#pragma once
#include <string>

using namespace std;

typedef enum
{
	CMD_JOIN,
	CMD_JOIN_ACK,
	CMD_LIST_USERS,
	CMD_LIST_USERS_ACK,
	CMD_EXIT_ROOM,
	CMD_EXIT_ROOM_ACK,
	CMD_BEGIN_CHAT,
	CMD_BEGIN_CHAT_ACK,
	CMD_END_CHAT,
	CMD_END_CHAT_ACK,
		
} COMMAND_TYPE;

char *arrCmd[] = {"CMD_JOIN", "CMD_JOIN_ACK", "CMD_LIST_USERS", "CMD_LIST_USERS_ACK", \
				"CMD_EXIT_ROOM", "CMD_EXIT_ROOM_ACK", "CMD_BEGIN_CHAT", "CMD_BEGIN_CHAT_ACK",  \
				"CMD_END_CHAT", "CMD_END_CHAT_ACK", NULL};


class CUser
{
public:
	CUser(string userName, string userIP, int userPort)
		:strUserName(userName),
		 strUserIP(userIP),
		 nUserPort(userPort)
	{}	
	~CUser(){}

	//int nUserNum;
	string strUserName;
	string strUserIP;
	int nUserPort;
};


