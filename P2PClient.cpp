#include <unistd.h>
#include <string.h>
#include <string>
#include "Socket.h"
#include "common.h"

using namespace std;

#define BUFLEN_MAX 	10000
#define MENU_BUFLEN 10
#define CMD_BUFLEN 30

enum INTERACT
{
	INTERACT_MAIN_MENU,
	INTERACT_ROOM_MENU,
};

char *arrInteract[] = {
" [1] Join the room\n\
 [2] Exit\n",
" [1] List all users.\n\
 [2] Select one to chat.\n\
 [3] Exit room."
};

enum CLIENT_STATUS
{
	STATUS_INIT,
	STATUS_EXIT,
	STATUS_JOIN_ROOM,
	STATUS_EXIT_ROOM
};

class CP2PClient
{
public:

	CP2PClient(char *userName, char *serverIP, char *serverPort)
		:m_objSocket(SOCK_DGRAM, AF_INET, 5),
		m_userName(userName),
		m_serverIP(serverIP),
		m_serverPort(serverPort),
		m_clientStatus(STATUS_INIT)
	{}
	
	virtual ~CP2PClient(){}

	int ReadAndSend()
	{	
		char buf[BUFLEN_MAX] = {0};
		while(1)
		{
			printf("please input content (\"quit\" to exit): \n");
			scanf("%s", buf);
			printf("\n");
			if (strcmp(buf, "quit") == 0)
				break;
			m_objSocket.SendTo(buf, BUFLEN_MAX, m_serverIP, m_serverPort);
			usleep(300);
		}
	}

	int BeginChat()
	{
		char key[MENU_BUFLEN] = {0};
		while(1)
		{
			printf("%s\n", arrInteract[INTERACT_MAIN_MENU]);
			scanf("%s", key);
			__ParseMainMenu(key);
			if (STATUS_EXIT == m_clientStatus)
				break;
			
			memset(key, 0, MENU_BUFLEN);
			usleep(300);
		}
		return 0;
	}
	
private:

	// if no data, please input NULL
	int __SendCommand(COMMAND_TYPE cmd, char *sendData, char *recvData)
	{
		char buf[BUFLEN_MAX] = {0};
		string strCMD = string(arrCmd[cmd]);
		if (NULL != sendData)
			strCMD += string(sendData);
		
		//LOG("__SendCommand buf : %s\n", strCMD.c_str());
		m_objSocket.SendTo(strCMD.c_str(), strCMD.size(), m_serverIP, m_serverPort);
		struct sockaddr_in addr;
		int status =  m_objSocket.RecvFrom(buf, BUFLEN_MAX, (struct sockaddr *)&addr);
		if (SOCK_SUCCESS != status)
		{
			LOG("recv ACK failed!\n");
			return -1;
		}
		
		if (strstr(buf, arrCmd[cmd+1]) != NULL)
		{
			printf("%s execute success!\n", arrCmd[cmd]);
			if (NULL != recvData && strlen(buf) != strlen(arrCmd[cmd+1]))
				strcpy(recvData, buf+strlen(arrCmd[cmd+1]));
			return 0;
		}
		else
		{
			printf("cmd recv = %s\n", buf);
			printf("%s execute failed!\n", arrCmd[cmd]);
			return -1;
		}
	}

	void __StepIntoChat()
	{
		m_clientStatus = STATUS_JOIN_ROOM;
		char key[MENU_BUFLEN] = {0};
		while(1)
		{
			printf("%s\n", arrInteract[INTERACT_ROOM_MENU]);
			scanf("%s", key);
			__ParseRoomMenu(key);
			memset(key, 0, MENU_BUFLEN);

			if (m_clientStatus == STATUS_EXIT_ROOM)
				break;
			
			usleep(300);
		}
	}

	void __ParseMainMenu(char *key)
	{
		int nKey = atoi(key);
		int status;
		switch(nKey)
		{
		case 1:
			status = __SendCommand(CMD_JOIN, m_userName, NULL);
			if(status == 0)
				__StepIntoChat();
			break;
		case 2:	
			m_clientStatus = STATUS_EXIT;
			break;
		default:
			break;
		}
	}

	void __ParseRoomMenu(char *key)
	{
		int nKey = atoi(key);
		int status;
		char recvData[BUFLEN_MAX] = {0};
		switch(nKey)
		{
		case 1:
			status = __SendCommand(CMD_LIST_USERS, NULL, recvData);
			if(status == 0)
				printf("%s\n", recvData);
			break;
		case 2:	
			break;

		case 3:
			status = __SendCommand(CMD_EXIT_ROOM, m_userName, NULL);		
			m_clientStatus = STATUS_EXIT_ROOM;
			break;
		
		default:
			break;
		}
	}

private:
	CSocket m_objSocket;
	char *m_userName;
	char *m_serverIP;
	char *m_serverPort;
	int m_clientStatus;
};


int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		LOG("Usage: ./P2PClient UserName IP Port\n");
		return 0;
	}

	CP2PClient client(argv[1], argv[2], argv[3]);
	//client.ReadAndSend();
	client.BeginChat();
	printf("Exit the client!\n");
	return 0;
}

