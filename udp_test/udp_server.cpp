#include "../Socket.h"
#include <unistd.h>

#define BUFLEN_MAX 		10000

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		LOG("Usage: ./P2PServer PORT\n");
		return 0;
	}

	CSocket m_objSocket(SOCK_DGRAM, AF_INET, INADDR_ANY, argv[1], 5);

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
			if(SOCK_AGAIN != status)
				LOG("RecvFrom Error!\n");
			usleep(300);
			continue;
		}

		LOG("RecvFrom Success!\n");
		LOG("ip: %s, port: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
		LOG("data: %s\n\n", buf);
		usleep(300);
	}

	return 0;
}