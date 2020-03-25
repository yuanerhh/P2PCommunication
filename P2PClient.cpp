#include <unistd.h>
#include <string.h>
#include "Socket.h"

#define BUFLEN_MAX 100

class CP2PClient
{
public:

	CP2PClient()
		:m_objSocket(SOCK_DGRAM, AF_INET)
	{
		
	}
	
	virtual ~CP2PClient(){};

	int ReadAndSend(char *strIP, char *strPort)
	{	
		char buf[BUFLEN_MAX] = {0};
		while(1)
		{
			printf("please input content (\"quit\" to exit): \n");
			scanf("%s", buf);
			printf("\n");
			if (strcmp(buf, "quit") == 0)
				break;
			m_objSocket.SendTo(buf, BUFLEN_MAX, strIP, strPort);
			usleep(300);
		}
	}

private:
	CSocket m_objSocket;
};


int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		LOG("Usage: ./P2PClient IP PORT\n");
		return 0;
	}

	CP2PClient client;
	client.ReadAndSend(argv[1], argv[2]);
	return 0;
}

