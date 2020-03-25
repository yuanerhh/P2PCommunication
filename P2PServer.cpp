#include <unistd.h>
#include "Socket.h"

#define BUFLEN_MAX 100

class CP2PServer
{
public:

	CP2PServer(char *strPort)
		:m_objSocket(SOCK_DGRAM, AF_INET, INADDR_ANY, strPort)
	{
		
	}
	
	virtual ~CP2PServer(){};

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

			LOG("RecvFrom Success!\n");
			LOG("ip: %s, port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
			LOG("data: %s\n\n", buf);

			usleep(300);
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

	return 0;
}

