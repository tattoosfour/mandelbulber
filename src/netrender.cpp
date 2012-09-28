/*
 * netrender.cpp
 *
 *  Created on: 28-09-2012
 *      Author: krzysztof marczak
 */

#include "netrender.hpp"

CNetRender *netRender;

CNetRender::CNetRender()
{
  memset(&host_info, 0, sizeof host_info);
  host_info_list = NULL;
  status = 0;
  isServer = false;
  socketfd = 0;
}

void CNetRender::SetServer(char *portNo, char *statusOut)
{
	isServer = true;
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
  timeout.tv_sec = 60;
  timeout.tv_usec = 0;

  if (setsockopt (socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
              sizeof(timeout)) < 0)
  	std::cout << "socket options error " << strerror(errno);

  if (setsockopt (socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
              sizeof(timeout)) < 0)
  	std::cout << "socket options error " << strerror(errno);

  status = bind(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1)
  	std::cout << "bind error" << std::endl ;
  else
  {
  	strcpy(statusOut,"status: server enabled");
  	printf("Server enabled\n");
  }
}
