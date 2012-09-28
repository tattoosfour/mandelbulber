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
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

using namespace std;

class CNetRender
{
public:
	CNetRender();
	~CNetRender();
	void SetServer(char *portNo);
	void SetClient(char *portNo, char*name);

private:
  int status;
  struct addrinfo host_info;       // The struct that getaddrinfo() fills up with data.
  struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.
  bool isServer;
  int socketfd ; // The socket descripter
};


#endif /* NETRENDER_HPP_ */
