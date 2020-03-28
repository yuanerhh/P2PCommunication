#pragma once
#include <string>
#include <string.h>

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
	CMD_CHAT_AGREE,
	CMD_CHAT_AGREE_ACK,
		
} COMMAND_TYPE;

char *arrCmd[] = {"CMD_JOIN", "CMD_JOIN_ACK", "CMD_LIST_USERS", "CMD_LIST_USERS_ACK", \
				"CMD_EXIT_ROOM", "CMD_EXIT_ROOM_ACK", "CMD_BEGIN_CHAT", "CMD_BEGIN_CHAT_ACK",  \
				"CMD_END_CHAT", "CMD_END_CHAT_ACK", "CMD_CHAT_AGREE", "CMD_CHAT_AGREE_ACK", NULL};


int ParseCmd(char *buf, char *cmdData)
{
	char *pos = NULL;
	for (int i = 0; ; i++)
	{
		if(arrCmd[i] == NULL)
			return -1;

		if ((pos = strstr(buf, arrCmd[i])) != NULL)
		{
			strcpy(cmdData, buf+strlen(arrCmd[i]));
			return i;				
		}
	}
}

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


