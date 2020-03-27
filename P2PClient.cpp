#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include "Socket.h"
#include "common.h"

using namespace std;

#define BUFLEN_MAX 		10000
#define MENU_BUFLEN 	10
#define CMD_BUFLEN 		30
#define NAMELEN_MAX 	50


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
	int __SendCommand(COMMAND_TYPE cmd, const char *sendData, char *recvData)
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

	void __ParseRoomerInfo(char *info)
	{
		m_vecUserList.clear();		
		string strInfo(info);
		vector<string> vecUserInfo;

		string::size_type position;
		int pos_begin = 0;
		while((position = strInfo.find("\n", position)) != string::npos)
	    {
			vecUserInfo.push_back(string(strInfo, pos_begin, position - pos_begin));
			position++;
			pos_begin = position;
	    }

		string::size_type position1 = 0;
		string::size_type position2 = 0;

		position = 0;

		for(int i = 0; i < vecUserInfo.size(); i++)
		{
			printf("vecUserInfo: %s\n", vecUserInfo[i].c_str());
			position1 = vecUserInfo[i].find(",", 0);
			position2 = vecUserInfo[i].find(",", position1 + 1);
			
			position = vecUserInfo[i].find("name: ", 0);
			string name(vecUserInfo[i], position+strlen("name: "), position1-position-strlen("name: "));

			position = vecUserInfo[i].find("ip: ", 0);
			string ip(vecUserInfo[i], position+strlen("ip: "), position2-position-strlen("ip: "));

			position = vecUserInfo[i].find("port: ", 0);
			int port = atoi(string(vecUserInfo[i], position+strlen("port: ")).c_str());
			//printf("vec: name:%s; ip:%s; port:%d;\n", name.c_str(), ip.c_str(), port);
			m_vecUserList.push_back(CUser(name, ip, port));
		}
	}

	void __ParseRoomMenu(char *key)
	{
		int nKey = atoi(key);
		int status;
		char recvData[BUFLEN_MAX] = {0};
		char frientName[NAMELEN_MAX] = {0};
		vector<CUser>::iterator iter;
		switch(nKey)
		{
		case 1:
			status = __SendCommand(CMD_LIST_USERS, NULL, recvData);
			if(status == 0)
				printf("%s\n", recvData);
			break;
		case 2:	
			{
				status = __SendCommand(CMD_LIST_USERS, NULL, recvData);
				__ParseRoomerInfo(recvData);
				memset(recvData, 0, BUFLEN_MAX);
				printf("please input the name to chat: \n");
				scanf("%s", frientName);
				for (iter = m_vecUserList.begin(); iter != m_vecUserList.end(); iter++)
				{
					if (iter->strUserName.find(frientName) != string::npos)
					{
						string strData(m_userName);
						strData += string(" trying connecting...");
						m_objSocket.SendTo(strData.c_str(), strData.size(), iter->strUserIP.c_str(), iter->nUserPort);
						break;
					}
				}

				if (iter == m_vecUserList.end())
					break;
				
				string strChat = string(m_userName)+string("+")+string(frientName);
				status = __SendCommand(CMD_BEGIN_CHAT, strChat.c_str(), recvData);
				//if (status == 0 &&)
				
				break;				
			}
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
	vector<CUser> m_vecUserList;
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

