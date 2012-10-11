/*
 * netrender.cpp
 *
 *  Created on: 28-09-2012
 *      Author: krzysztof marczak
 */

#include "netrender.hpp"
#include <stdio.h>
#include <arpa/inet.h>

CNetRender *netRender;

CNetRender::CNetRender(int myVersion, int CPUs)
{
  memset(&host_info, 0, sizeof host_info);
  host_info_list = NULL;
  status = 0;
  isServer = false;
  isClient = false;
  socketfd = 0;
  clientIndex = 0;
  version = myVersion;
  dataBuffer = NULL;
  dataSize = 0;
  noOfCPUs = CPUs;
}

CNetRender::~CNetRender()
{
	if (dataBuffer) delete [] dataBuffer;
}

bool CNetRender::SetServer(char *portNo, char *statusOut)
{
  memset(&host_info, 0, sizeof host_info);

  host_info.ai_family = AF_UNSPEC;     // IP version not specified. Can be both.
  host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
  host_info.ai_flags = AI_PASSIVE;     // IP Wildcard

  status = getaddrinfo(NULL, portNo, &host_info, &host_info_list);
  if (status != 0)  std::cout << "getaddrinfo error" << gai_strerror(status) ;

  std::cout << "Creating a socket..."  << std::endl;
  socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
  if (socketfd == -1)  std::cout << "socket error " << strerror(errno);

  std::cout << "Binding socket..."  << std::endl;
  // we use to make the setsockopt() function to make sure the port is not in use
  // by a previous execution of our code. (see man page for more information)
  int yes = 1;
  status = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  if (setsockopt (socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
              sizeof(timeout)) < 0)
  	std::cout << "socket options error " << strerror(errno);

  if (setsockopt (socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
              sizeof(timeout)) < 0)
  	std::cout << "socket options error " << strerror(errno);

  status = bind(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1)
  {
  	std::cout << "bind error" << std::endl ;
  	return false;
  }
  else
  {
  	strcpy(statusOut,"status: server enabled");
  	printf("Server enabled\n");
  	isServer = true;
  	return true;
  }
}

void CNetRender::DeleteServer(void)
{
	if(host_info_list) freeaddrinfo(host_info_list);
	if(socketfd > 0) close(socketfd);
	if(clients.size() > 0)
	{
		for(unsigned int i=0; i<clients.size(); i++)
		{
			close(clients[0].socketfd);
		}
		clients.clear();
	}
	isServer = false;
}

bool CNetRender::SetClient(char *portNo, char*name, char *statusOut)
{
	memset(&host_info, 0, sizeof host_info);

	host_info.ai_family = AF_UNSPEC;     // IP version not specified. Can be both.
	host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.

	status = getaddrinfo(name, portNo, &host_info, &host_info_list);
	if (status != 0)  std::cout << "getaddrinfo error" << gai_strerror(status) ;

	std::cout << "Creating a socket..."  << std::endl;

	socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
	host_info_list->ai_protocol);
	if (socketfd == -1)  std::cout << "socket error " << strerror(errno);

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  if (setsockopt (socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
              sizeof(timeout)) < 0)
  	std::cout << "socket options error " << strerror(errno);

  if (setsockopt (socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
              sizeof(timeout)) < 0)
  	std::cout << "socket options error " << strerror(errno);

	//connecting to server
  std::cout << "Connect()ing..."  << std::endl;
  status = connect(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1)
  {
  	std::cout << "connect error" << strerror(errno);
  	strcpy(statusOut, strerror(errno));
  	return false;
  }
  else
  {
  	strcpy(statusOut,"status: client connected to server");
  	printf("Client connected to server\n");

  	//sending version number

		char command[4];
		size_t recvd_bytes = receiveDataFromServer(command);
		if(!strcmp("ver", command))
		{
	  	size_t len = sizeof(version);
	  	sendDataToServer(&version, len, "VER");
		}

  	//checking aswer regarding version
		recvd_bytes = receiveDataFromServer(command);
		if(!strcmp("ok.", command))
		{
    	printf("Client version approved\n");
  		isClient = true;

  		recvd_bytes = receiveDataFromServer(command);
  		if(!strcmp("cpu", command))
  		{
  			sendDataToServer(&noOfCPUs, sizeof(noOfCPUs), "CPU");
  		}

    	return true;
		}
		else if(!strcmp("bad", command))
		{
    	int serverVersion;
    	if(recvd_bytes == sizeof(int)) GetData(&serverVersion);
			printf("Client version refused. Server version is %f\n", serverVersion/1000);
  		isClient = false;
  		sprintf(statusOut,"status: client has wrong version. Server is %f", serverVersion/1000);
  		printf("Client disconnected from server because client version is wrong\n");
    	return false;
		}
		else
		{
			sprintf(statusOut,"status: communication error during version verification");
			printf("communication error during version verification");
			return false;
		}
  }
}

void CNetRender::DeleteClient(void)
{
	if(host_info_list) freeaddrinfo(host_info_list);
	if(socketfd > 0) close(socketfd);
}

bool CNetRender::WaitForClient(char *statusOut)
{
	//std::cout << "Listen()ing for connections..." << std::endl;
	status = listen(socketfd, 5);
	if (status == -1)
	{
		//std::cout << "listen error" << std::endl;
		strcpy(statusOut,"status: listen error");
		return false;
	}

	//accepting donnection
	struct sockaddr_storage their_addr;
	socklen_t addr_size = sizeof(their_addr);
	sClients newClient;
	newClient.socketfd = accept(socketfd, (struct sockaddr *) &their_addr, &addr_size);
	if (newClient.socketfd  == -1)
	{
		//std::cout << "listen error" << std::endl;
		strcpy(statusOut,"status: client not found");
		return false;
	}
	else
	{
		std::cout << "Connection accepted. Using new socketfd : " << newClient.socketfd << std::endl;

		//getting IP address
		socklen_t len;
		struct sockaddr_storage addr;
		char ipstr[INET6_ADDRSTRLEN];
		int port;

		len = sizeof addr;
		getpeername(newClient.socketfd, (struct sockaddr*)&addr, &len);

		// deal with both IPv4 and IPv6:
		if (addr.ss_family == AF_INET) {
		    struct sockaddr_in *saddr = (struct sockaddr_in *)&addr;
		    port = ntohs(saddr->sin_port);
		    inet_ntop(AF_INET, &saddr->sin_addr, ipstr, sizeof ipstr);
		} else { // AF_INET6
		    struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&addr;
		    port = ntohs(saddr->sin6_port);
		    inet_ntop(AF_INET6, &saddr->sin6_addr, ipstr, sizeof ipstr);
		}

		strcpy(newClient.ipstr, ipstr);
		newClient.port = port;

		//remembering new client
		clients.push_back(newClient);
		clientIndex = clients.size();

		//preparing status
		char stat[1000];
		sprintf(stat,"status: Client #%d has IP address: %s, port %d\n", clientIndex, ipstr, port);
		strcpy(statusOut, stat);
		std::cout << stat;

		//checking client version
		int clientVersion;
		sendDataToClient(NULL, 0, "ver", clientIndex-1);
		char command[4];
		size_t recvd_bytes = receiveDataFromClient(command, clientIndex-1);
		if(!strcmp("VER", command) && recvd_bytes == sizeof(int))
		{
			GetData(&clientVersion);
		}

		if(clientVersion == version)
		{
			printf("Client version is correct\n");

			//sending answer
			char *accepted = (char*)"accepted";
			sendDataToClient(NULL, 0, "ok.", clientIndex-1);

			//ask for number of CPUs
			sendDataToClient(NULL, 0, "cpu", clientIndex-1);
			recvd_bytes = receiveDataFromClient(command, clientIndex-1);
			if(!strcmp("CPU", command) && recvd_bytes == sizeof(int))
			{
				int noOfCpu;
				GetData(&noOfCpu);
				clients[clientIndex-1].noOfCPU = noOfCpu;
				printf("Client #%d has %d CPUs\n", clientIndex, noOfCpu);
			}

			return true;
		}
		else
		{
			printf("Wrong client version\n");

			//sending answer
			char *refused = (char*)"refused";
			sendDataToClient(&version, sizeof(version), "bad", clientIndex-1);

			//deleting client
			clients.erase(clients.end()-1);
			return false;
		}
	}
}

bool CNetRender::sendDataToClient(void *data, size_t size, char *command, int index)
{
	//printf("Sending %d bytes data with command %s...\n", size, command);
	send(clients[index].socketfd, command, 4, 0);
	send(clients[index].socketfd, &size, sizeof(size_t), 0);

	size_t send_left = size;
	char *dataPointer = (char*)data;

	while(send_left > 0)
	{
		ssize_t bytes_send = send(clients[index].socketfd, dataPointer, send_left, 0);
		if (bytes_send == -1) return false;
		send_left -= bytes_send;
		dataPointer += bytes_send;
		//printf("Sent %d bytes\n", bytes_send);
	}
	return true;
}

bool CNetRender::sendDataToServer(void *data, size_t size, char *command)
{
	//printf("Sending %d bytes data with command %s...\n", size, command);
	send(socketfd, command, 4, 0);
	send(socketfd, &size, sizeof(size_t), 0);

	size_t send_left = size;
	char *dataPointer = (char*)data;

	while(send_left > 0)
	{
		ssize_t bytes_send = send(socketfd, dataPointer, send_left, 0);
		if (bytes_send == -1) return false;
		send_left -= bytes_send;
		dataPointer += bytes_send;
		//printf("Sent %d bytes\n", bytes_send);
	}
	return true;
}

size_t CNetRender::receiveDataFromServer(char *command)
{
	//printf("Waiting for data...\n");
	memset(command,0,4);
	ssize_t bytes_recvd = recv(socketfd, command, 4, 0);

	if (bytes_recvd <= 0)
	{
		if(errno != 11)
		{
			std::cout << errno << strerror(errno) << endl;
		}
		return 0;
	}

	//printf("Received command: %s\n", command);

	size_t size = 0;
	recv(socketfd, &size, sizeof(size_t), 0);

	//printf("Will be received %d bytes\n", size);

	if (size > 0)
	{
		if(dataBuffer) delete [] dataBuffer;
		dataBuffer = new char[size];

		char *dataPointer = dataBuffer;
		size_t rcv_left = size;

		while(rcv_left > 0)
		{
			bytes_recvd = recv(socketfd, dataPointer, rcv_left, 0);
			//printf("%d bytes received\n", bytes_recvd);
			if(bytes_recvd == -1)
			{
				printf("Data receive error\n");
				return 0;
			}

			rcv_left -= bytes_recvd;
			dataPointer += bytes_recvd;
		}

		dataSize = size;
	}
	return size;
}

size_t CNetRender::receiveDataFromClient(char *command, int index)
{
	//printf("Waiting for data...\n");
	memset(command,0,4);
	ssize_t bytes_recvd = recv(clients[index].socketfd, command, 4, 0);

	if (bytes_recvd <= 0)
	{
		if(errno != 11)
		{
			std::cout << errno << strerror(errno) << endl;
		}
		return 0;
	}

	//printf("Received command: %s\n", command);

	size_t size = 0;
	recv(clients[index].socketfd, &size, sizeof(size_t), 0);

	//printf("Will be received %d bytes\n", size);

	if (size > 0)
	{
		if(dataBuffer) delete [] dataBuffer;
		dataBuffer = new char[size];

		char *dataPointer = dataBuffer;
		size_t rcv_left = size;

		while(rcv_left > 0)
		{
			bytes_recvd = recv(clients[index].socketfd, dataPointer, rcv_left, 0);
			//printf("%d bytes received\n", bytes_recvd);
			if(bytes_recvd == -1)
			{
				printf("Data receive error\n");
				return 0;
			}

			rcv_left -= bytes_recvd;
			dataPointer += bytes_recvd;
		}

		dataSize = size;
	}
	return size;
}

void CNetRender::GetData(void *data)
{
	memcpy(data, dataBuffer, dataSize);
}
