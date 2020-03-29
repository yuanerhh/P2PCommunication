#include "../Socket.h"
#include <unistd.h>

#define BUFLEN_MAX 		10000

int main(int argc, char* argv[])
{
	if (argc < 5)
	{
		LOG("Usage: ./P2PClient IP Port IP Port\n");
		return 0;
	}
	CSocket m_objSocket(SOCK_DGRAM, AF_INET, 5);
	char buf[BUFLEN_MAX] = {0};
	while(1)
	{
		printf("please input content (\"quit\" to exit): \n");
		scanf("%s", buf);
		printf("\n");
		if (strcmp(buf, "quit") == 0)
			break;
		m_objSocket.SendTo(buf, BUFLEN_MAX, argv[1], argv[2]);
		m_objSocket.SendTo(buf, BUFLEN_MAX, argv[3], argv[4]);
		usleep(300);
	}
}

