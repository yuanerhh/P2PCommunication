#include <unistd.h>
#include <string.h>
#include "Socket.h"
#include "common.h"

#define BUFLEN_MAX 100

char strErr[] = "ERROR CMD!!!";

class CP2PServer
{
public:

	CP2PServer(char *strPort)
		:m_objSocket(SOCK_DGRAM, AF_INET, INADDR_ANY, strPort)
	{
		
	}
	
	virtual ~CP2PServer(){};

	// cooperate with client ReadAndSend
	int AcceptAndShow()   
	{
		char buf[BUFLEN_MAX] = {0};
		int status;
		m_objSocket.Bind();

		LOG("server receving data ......\n");
		
		while(1)
		{
			struct sockaddr_in addr;
			status = m_objSocket.RecvFrom(buf, BUFLEN_MAX, (struct sockaddr *)&addr);
			if (SOCK_SUCCESS != status)
			{
				LOG("RecvFrom Error!\n");
				usleep(300);
				continue;
			}

			LOG("RecvFrom Success!\n");
			LOG("ip: %s, port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			LOG("data: %s\n\n", buf);
			memset(buf, BUFLEN_MAX, 0);
			usleep(300);
		}
	}

	int Start()
	{
		char buf[BUFLEN_MAX] = {0};
		int status;
		m_objSocket.Bind();

		LOG("server receving data ......\n");
		
		while(1)
		{
			struct sockaddr_in addr;
			status = m_objSocket.RecvFrom(buf, BUFLEN_MAX, (struct sockaddr *)&addr);
			if (SOCK_SUCCESS != status)
			{
				LOG("RecvFrom Error!\n");
				usleep(300);
				continue;
			}

			int cmd = __ParseCmd(buf);
			if (cmd < 0)
			{
				LOG("__ParseCmd failed!\n");
				m_objSocket.SendTo(strErr, sizeof(strErr), (struct sockaddr *)&addr);
				goto out;
			}

			LOG("#### ip: %s, port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			LOG("#### data: %s\n\n", buf);
			LOG("cmd num = %d\n", cmd);
			LOG("strlen(arrCmd[cmd]) = %d, strlen(arrCmd[cmd+1]) = %d\n", strlen(arrCmd[cmd]), strlen(arrCmd[cmd+1]));

			m_objSocket.SendTo(arrCmd[cmd+1], strlen(arrCmd[cmd+1]), (struct sockaddr *)&addr);
		
	out:	memset(buf, BUFLEN_MAX, 0);
			usleep(300);
		}
	}

private:

	int __ParseCmd(char *buf)
	{
		for (int i = 0; ; i++)
		{
			if(arrCmd[i] == NULL)
				return -1;

			if (strstr(buf, arrCmd[i]) != NULL)
				return i;
		}
	}

private:
	CSocket m_objSocket;
};


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		LOG("Usage: ./P2PServer PORT\n");
		return 0;
	}

	CP2PServer server(argv[1]);
	server.Start();
	//server.AcceptAndShow();

	return 0;
}

