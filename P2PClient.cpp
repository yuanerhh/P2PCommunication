#include <unistd.h>
#include <string.h>
#include "Socket.h"
#include "common.h"

using namespace std;

#define BUFLEN_MAX 	100
#define MENU_BUFLEN 10
#define CMD_BUFLEN 10

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
	STATUS_EXIT
};

class CP2PClient
{
public:

	CP2PClient(char *userName, char *serverIP, char *serverPort)
		:m_objSocket(SOCK_DGRAM, AF_INET),
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
			
			usleep(300);
		}
		return 0;
	}
	
private:

	int __SendCommand(COMMAND_TYPE cmd)
	{
		char buf[CMD_BUFLEN] = {0};
		m_objSocket.SendTo(arrCmd[cmd], sizeof(arrCmd[cmd]), m_serverIP, m_serverPort);
		struct sockaddr_in addr;
		m_objSocket.RecvFrom(buf, CMD_BUFLEN, (struct sockaddr *)&addr);
		if (strcmp(buf, arrCmd[cmd+1]) != 0)
		{
			printf("%s execute success!", arrCmd[cmd]);
			return -1;
		}
		else
		{
			printf("%s execute failed!", arrCmd[cmd]);
			return 0;
		}
	}

	void __StepIntoChat()
	{
		
	}

	void __ParseMainMenu(char *key)
	{
		int nKey = atoi(key);
		int status;
		switch(nKey)
		{
		case 1:
			status = __SendCommand(CMD_JOIN);
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

