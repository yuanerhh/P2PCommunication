#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <pthread.h>
#include "Socket.h"
#include "common.h"

using namespace std;

#define BUFLEN_MAX 		10000
#define MENU_BUFLEN 	10
#define CMD_BUFLEN 		30
#define NAMELEN_MAX 	50
#define MAX_WAIT_COUNT	4

#define DEFAULT_RECV_TIME 		5
#define THREAD_SLEEP_TIME_MS 	20

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
	STATUS_EXIT_ROOM,
	STATUS_CHATTING,
};

static void *RoomListenThread(void *arg);

class CP2PClient
{
public:

	CP2PClient(char *userName, char *serverIP, char *serverPort)
		:m_objSocket(SOCK_DGRAM, AF_INET, DEFAULT_RECV_TIME),
		m_userName(userName),
		m_serverIP(serverIP),
		m_serverPort(serverPort),
		m_clientStatus(STATUS_INIT) 
	{
		pthread_mutex_init(&m_threadLock,NULL);
		pthread_mutex_init(&m_scanfLock,NULL);
	}
	
	virtual ~CP2PClient()
	{
		pthread_mutex_destroy(&m_threadLock);
		pthread_mutex_destroy(&m_scanfLock);
	}

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

	// if no data, please input NULL
	int __SendCommand(COMMAND_TYPE cmd, const char *sendData, char *recvData)
	{
		char buf[BUFLEN_MAX] = {0};
		string strCMD = string(arrCmd[cmd]);
		if (NULL != sendData)
			strCMD += string(sendData);

		pthread_mutex_lock(&m_threadLock);		
		LOG("__SendCommand buf : %s\n", strCMD.c_str());
		m_objSocket.SendTo(strCMD.c_str(), strCMD.size(), m_serverIP, m_serverPort);
		struct sockaddr_in addr;
		int status =  m_objSocket.RecvFrom(buf, BUFLEN_MAX, (struct sockaddr *)&addr);
		pthread_mutex_unlock(&m_threadLock);
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

private:

	void __StepIntoChat()
	{
		m_clientStatus = STATUS_JOIN_ROOM;
		char key[MENU_BUFLEN] = {0};
		while(1)
		{
			pthread_mutex_lock(&m_scanfLock);
			printf("%s\n", arrInteract[INTERACT_ROOM_MENU]);
			scanf("%s", key);
			if (m_clientStatus != STATUS_CHATTING)
				__ParseRoomMenu(key);
			memset(key, 0, MENU_BUFLEN);

			if (m_clientStatus == STATUS_EXIT_ROOM)
				break;
			pthread_mutex_unlock(&m_scanfLock);
			usleep(300);
		}
	}

	void __ParseMainMenu(char *key)
	{
		int nKey = atoi(key);
		int status;
		int tStatus;
		switch(nKey)
		{
		case 1:
			status = __SendCommand(CMD_JOIN, m_userName, NULL);
			if(status == 0)
			{
				if ((tStatus = pthread_create(&m_listenThread, NULL, RoomListenThread, this)) != 0)
				{
					printf("pthread_create failed, erroNum = %d\n", tStatus);
					printf("Join the room failed!\n");
					break;
				}
					
				__StepIntoChat();
				
				
			}
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
						printf("SendTo ip->%s, port->%d, Data: %s\n", iter->strUserIP.c_str(), iter->nUserPort, strData.c_str());
						m_objSocket.SendTo(strData.c_str(), strData.size(), iter->strUserIP.c_str(), iter->nUserPort);
						break;
					}
				}

				if (iter == m_vecUserList.end())
					break;
				
				string strChat = string(m_userName)+string("+")+string(frientName);
				status = __SendCommand(CMD_BEGIN_CHAT, strChat.c_str(), recvData);
				int nCount = 0;
				char charAckBuf[CMD_BUFLEN] = {0};
				struct sockaddr_in apply_addr;
				do
				{
					status = m_objSocket.RecvFrom(charAckBuf, CMD_BUFLEN, (struct sockaddr *)&apply_addr);
					nCount++;
				} while (status != 0 && nCount < MAX_WAIT_COUNT);

				if (nCount != MAX_WAIT_COUNT &&
					strcmp(inet_ntoa(apply_addr.sin_addr), iter->strUserIP.c_str()) == 0  && 
					ntohs(apply_addr.sin_port) ==  iter->nUserPort &&
					strcmp(charAckBuf, arrCmd[CMD_CHAT_AGREE]) == 0)
				{
					printf("applyer agree chat!\n");					
				}
						
			}
			break;
		
		case 3:
			status = __SendCommand(CMD_EXIT_ROOM, m_userName, NULL);		
			m_clientStatus = STATUS_EXIT_ROOM;
			pthread_mutex_unlock(&m_scanfLock);
			pthread_join(m_listenThread, NULL);
			break;
		
		default:
			break;
		}
	}

private:
	
	char *m_userName;
	char *m_serverIP;
	char *m_serverPort;
	pthread_t m_listenThread;

public:
	CSocket m_objSocket;
	int m_clientStatus;
	pthread_mutex_t m_threadLock;
	pthread_mutex_t m_scanfLock;
	vector<CUser> m_vecUserList;
	
};


static void *RoomListenThread(void *arg)
{
	CP2PClient *client = (CP2PClient *)arg;
	char recvBuf[BUFLEN_MAX] = {0};
	char cmdData[NAMELEN_MAX] = {0};
	char scanfBuf[CMD_BUFLEN] = {0};
	struct sockaddr_in addr;
	int status = 0;

	while(client->m_clientStatus != STATUS_EXIT_ROOM)
	{
		memset(recvBuf, 0, BUFLEN_MAX);
		memset(cmdData, 0, NAMELEN_MAX);
		pthread_mutex_lock(&client->m_threadLock);
		client->m_objSocket.SetTimeOut(0, 1000 * THREAD_SLEEP_TIME_MS);
		status = client->m_objSocket.RecvFrom(recvBuf, BUFLEN_MAX, (struct sockaddr *)&addr);
		if (status != SOCK_SUCCESS)
		{
			//printf("RoomListenThread: m_objSocket.RecvFrom ----- 0 \n");
			client->m_objSocket.SetTimeOut(DEFAULT_RECV_TIME, 0);
			pthread_mutex_unlock(&client->m_threadLock);
			usleep(1000 * THREAD_SLEEP_TIME_MS);
			continue;
		}

		printf("RoomListenThread buf: %s\n", recvBuf);

		int cmd = ParseCmd(recvBuf, cmdData);
		if (cmd < 0)
		{
			client->m_objSocket.SetTimeOut(DEFAULT_RECV_TIME, 0);
			pthread_mutex_unlock(&client->m_threadLock);
			LOG("ParseCmd failed!\n");
			continue;
		}

		switch (client->m_clientStatus)
		{
		case STATUS_JOIN_ROOM:
			if (cmd == CMD_BEGIN_CHAT)
			{
				client->m_clientStatus = STATUS_CHATTING;
				pthread_mutex_lock(&client->m_scanfLock);
				if (client->m_clientStatus == STATUS_EXIT_ROOM)
					break;
				
				printf("%s apply for chatting, (y/n):\n", cmdData);
				scanf("%s", scanfBuf);
				if (strstr(scanfBuf, "y") != NULL)
				{
					printf("RoomListenThread -------------------- 0\n");
					pthread_mutex_unlock(&client->m_threadLock);
					char recvData[BUFLEN_MAX] = {0};
					if(client->__SendCommand(CMD_LIST_USERS, NULL, recvData) == 0)
						client->__ParseRoomerInfo(recvData);
					pthread_mutex_lock(&client->m_threadLock);
					printf("RoomListenThread -------------------- 2\n");
					vector<CUser>::iterator iter;
					int status = 0, nCount = 0;
					char chatAckBuf[CMD_BUFLEN] = {0};
					struct sockaddr_in apply_addr;
					printf("RoomListenThread -------------------- 3\n");
					for (iter = client->m_vecUserList.begin(); iter != client->m_vecUserList.end(); iter++)
					{
						if (iter->strUserName.find(cmdData) != string::npos)
						if (strcmp(iter->strUserName.c_str(), cmdData) == 0)
						{
							printf("send agree chat buf: %s, ip->%s, port->%d\n", arrCmd[CMD_CHAT_AGREE], iter->strUserIP.c_str(), iter->nUserPort);
							client->m_objSocket.SendTo(arrCmd[CMD_CHAT_AGREE], sizeof(arrCmd[CMD_CHAT_AGREE]), iter->strUserIP.c_str(), iter->nUserPort);
							client->m_objSocket.SendTo(arrCmd[CMD_CHAT_AGREE], sizeof(arrCmd[CMD_CHAT_AGREE]), iter->strUserIP.c_str(), iter->nUserPort);
							client->m_objSocket.SendTo(arrCmd[CMD_CHAT_AGREE], sizeof(arrCmd[CMD_CHAT_AGREE]), iter->strUserIP.c_str(), iter->nUserPort);
							do
							{
								status = client->m_objSocket.RecvFrom(chatAckBuf, CMD_BUFLEN, (struct sockaddr *)&apply_addr);
								nCount++;
							} while (status != 0 && nCount < MAX_WAIT_COUNT);
						}
					}
					printf("RoomListenThread -------------------- 4  nCount = %d\n", MAX_WAIT_COUNT);

					if (iter != client->m_vecUserList.end() &&
						nCount != MAX_WAIT_COUNT &&
						strcmp(inet_ntoa(apply_addr.sin_addr), iter->strUserIP.c_str()) == 0  && 
						ntohs(apply_addr.sin_port) ==  iter->nUserPort &&
						strcmp(chatAckBuf, arrCmd[CMD_CHAT_AGREE_ACK]) == 0)
					{
						printf("recver agree chat!\n", iter->strUserName.c_str());
					}
					printf("RoomListenThread -------------------- 5\n");
				}

				pthread_mutex_unlock(&client->m_scanfLock);
			}
			break;
		
		case STATUS_CHATTING:
			break;
		}

		client->m_clientStatus = STATUS_JOIN_ROOM;

		client->m_objSocket.SetTimeOut(DEFAULT_RECV_TIME, 0);
		pthread_mutex_unlock(&client->m_threadLock);
		usleep(1000 * THREAD_SLEEP_TIME_MS);
	}
}



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

