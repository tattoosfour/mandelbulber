/*
 * netrender.hpp
 *
 *  Created on: 28-09-2012
 *      Author: krzysztof marczak
 */

#ifndef NETRENDER_HPP_
#define NETRENDER_HPP_

#include <iostream>
#include <cstring>
#ifdef WIN32
	#undef WINVER
	#define WINVER WindowsXP
	#define _WIN32_WINNT 0xFFFF
	//#include <windows.h>
	#include <ws2tcpip.h>
	#include <winsock2.h>
	#define INET6_ADDRSTRLEN 46
#else
	#include <sys/socket.h>
	#include <netdb.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <vector>

using namespace std;

struct sClients
{
	int socketfd;
	char ipstr[INET6_ADDRSTRLEN];
	int port;
	int noOfCPU;
};

class CNetRender
{
public:
	CNetRender(int myVersion, int CPUs);
	~CNetRender();
	bool SetServer(char *portNo, char *statusOut);
	void DeleteServer(void);
	bool SetClient(char *portNo, char*name, char *statusOut);
	void DeleteClient(void);
	bool WaitForClient(char *statusOut);
	bool IsServer() {return isServer;}
	bool IsClient() {return isClient;}
	int getNoOfClients() {return clientIndex;};
	int getCpuCount(int index) {return clients[index].noOfCPU;};
	bool sendDataToClient(void *data, size_t size, char *command, int index);
	bool sendDataToServer(void *data, size_t size, char *command);
	size_t receiveDataFromServer(char *command);
	size_t receiveDataFromClient(char *command, int index);
	void GetData(void *data);

private:
	unsigned int CalculateCRC(char *data, size_t len);
  int status;
  struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
  struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
  bool isServer;
  bool isClient;
  int socketfd ; // The socket descripter
  int clientIndex;
  vector<sClients> clients;
  int version;
  char *dataBuffer;
  size_t dataSize;
  int noOfCPUs;
};

extern CNetRender *netRender;

#endif /* NETRENDER_HPP_ */
