#pragma once

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <arpa/inet.h> 	// for inet_addr
#include <stdlib.h>  	// for atoi itoa
#include <errno.h>


#define SOCK_SUCCESS 		0
#define SOCK_AGAIN			1
#define SOCK_ERR   			-1
#define SOCK_ERR_SOCKET  	-2
#define SOCK_ERR_BIND  		-3
#define SOCK_ERR_SEND  		-4
#define SOCK_ERR_RECV  		-5

#define LOG(format,...) printf(format, ##__VA_ARGS__)

#define ASSERT_SOCKET(x) 			\
	if (x < 0) 						\
	{								\
		LOG("Invalid socket\n"); 	\
		return SOCK_ERR_SOCKET;			\
	}	


class CSocket
{
public:

	CSocket(int nProtocol, int nAddrFamily, int nTimeOut = 0)
	{
		m_socket = socket(nAddrFamily, nProtocol, 0);
		if (m_socket < 0)
		{
			LOG("Invalid socket\n");
			return;
		}

		if (nTimeOut != 0)
		{
			/* 设置阻塞超时 */
		    struct timeval timeOut;
		    timeOut.tv_sec = nTimeOut;                 //设置超时
		    timeOut.tv_usec = 0;
		    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut)) < 0)
		    {
		        LOG("time out setting failed\n");
		    }
		}
	}
	
	CSocket(int nProtocol, int nAddrFamily, char *strIP, char *strPort, int nTimeOut = 0)
	{
		m_socket = socket(nAddrFamily, nProtocol, 0);
		if (m_socket < 0)
		{
			LOG("Invalid socket\n");
			return;
		}

		if (nTimeOut != 0)
		{
			/* 设置阻塞超时 */
		    struct timeval timeOut;
		    timeOut.tv_sec = nTimeOut;                 //设置超时
		    timeOut.tv_usec = 0;
		    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut)) < 0)
		    {
		        LOG("time out setting failed\n");
		    }
		}

		m_addr.sin_family = nAddrFamily;
		m_addr.sin_addr.s_addr = inet_addr(strIP);
		m_addr.sin_port = htons(atoi(strPort));
	}

	CSocket(int nProtocol, int nAddrFamily, in_addr_t i32IP, char *strPort, int nTimeOut = 0)
	{
		m_socket = socket(nAddrFamily, nProtocol, 0);
		if (m_socket < 0)
		{
			LOG("Invalid socket\n");
			return;
		}

		if (nTimeOut != 0)
		{
			/* 设置阻塞超时 */
		    struct timeval timeOut;
		    timeOut.tv_sec = nTimeOut;                 //设置超时
		    timeOut.tv_usec = 0;
		    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut)) < 0)
		    {
		        LOG("time out setting failed\n");
		    }
		}

		m_addr.sin_family = nAddrFamily;
		m_addr.sin_addr.s_addr = i32IP;
		m_addr.sin_port = htons(atoi(strPort));
	}

	CSocket(int nProtocol, int nAddrFamily, in_addr_t i32IP, int16_t i16Port, int nTimeOut = 0)
	{
		m_socket = socket(nAddrFamily, nProtocol, 0);
		if (m_socket < 0)
		{
			LOG("Invalid socket\n");
			return;
		}

		if (nTimeOut != 0)
		{
			/* 设置阻塞超时 */
		    struct timeval timeOut;
		    timeOut.tv_sec = nTimeOut;                 //设置超时
		    timeOut.tv_usec = 0;
		    if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut)) < 0)
		    {
		        LOG("time out setting failed\n");
		    }
		}

		m_addr.sin_family = nAddrFamily;
		m_addr.sin_addr.s_addr = i32IP;
		m_addr.sin_port = htons(i16Port);
	}
	
	virtual ~CSocket(){};

	int Bind()
	{
		ASSERT_SOCKET(m_socket);
		int status;
		if ((status = bind(m_socket, (struct sockaddr *)&m_addr, sizeof(struct sockaddr))) < 0)
		{			
			LOG("Error Bind: %s\n", strerror(errno));
			return SOCK_ERR_BIND;
		}

		return SOCK_SUCCESS;
	}

	int SendTo(const void *buf, size_t len, const char *strIP, char *strPort)
	{
		ASSERT_SOCKET(m_socket);
		int status;
		socklen_t addrLen = sizeof(struct sockaddr);
		struct sockaddr_in dest_addr;
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_addr.s_addr = inet_addr(strIP);
		dest_addr.sin_port = htons(atoi(strPort));
		if ((status = sendto(m_socket, buf, len, 0, (struct sockaddr *)&dest_addr, addrLen)) < 0)
		{
			LOG("Error sendto: %s\n", strerror(errno));
			return SOCK_ERR_SEND;
		}	

		return SOCK_SUCCESS;
	}

	int SendTo(const void *buf, size_t len, const char *strIP, int nPort)
	{
		ASSERT_SOCKET(m_socket);
		int status;
		socklen_t addrLen = sizeof(struct sockaddr);
		struct sockaddr_in dest_addr;
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_addr.s_addr = inet_addr(strIP);
		dest_addr.sin_port = htons(nPort);
		if ((status = sendto(m_socket, buf, len, 0, (struct sockaddr *)&dest_addr, addrLen)) < 0)
		{
			LOG("Error sendto: %s\n", strerror(errno));
			return SOCK_ERR_SEND;
		}	

		return SOCK_SUCCESS;
	}

	int SendTo(const void *buf, size_t len, struct sockaddr *dest_addr)
	{
		ASSERT_SOCKET(m_socket);
		int status;
		socklen_t addrLen = sizeof(struct sockaddr);
		if ((status = sendto(m_socket, buf, len, 0, dest_addr, addrLen)) < 0)
		{
			LOG("Error sendto: %s\n", strerror(errno));
			return SOCK_ERR_SEND;
		}	

		return SOCK_SUCCESS;
	}

	int RecvFrom(void *buf, size_t len, struct sockaddr *src_addr)
	{
		ASSERT_SOCKET(m_socket);
		int status;
		socklen_t addrLen = sizeof(struct sockaddr);
		if ((status = recvfrom(m_socket, buf, len, 0, src_addr, &addrLen)) < 0)
		{
			if (errno == EAGAIN)
				return SOCK_AGAIN;
				
			LOG("Error recvfrom: %s\n", strerror(errno));
			return SOCK_ERR_RECV;
		}

		return SOCK_SUCCESS;
	}

private:
	int m_socket;
	struct sockaddr_in m_addr;
};
