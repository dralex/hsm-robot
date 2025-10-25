/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Base tcp/udp network commication module implementation
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see https://www.gnu.org/licenses/
 *
 * ----------------------------------------------------------------------------- */

#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <errno.h>

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "network.h"

using namespace HSMRobot;

const char* TCP_PROTO = "tcp";
const char* UDP_PROTO = "udp";

Network::Network():
	connected(false), tcp(false), server(false), listen_fd(-1), fd(-1)
{
	memset(hostname, 0, sizeof(hostname));
	memset(port, 0, sizeof(port));
	counter = 0;
}

Network::~Network()
{
	if (connected || server) {
		disconnect();
	}
}

NetworkError Network::init_socket_type(const char* _hostname, const char* _port, const char* proto)
{
	if (connected || server) return networkBadArguments;
	if (!_hostname) return networkBadArguments;

	strncpy(hostname, _hostname, ADDRESS_BUFFER_SIZE - 1);
	strncpy(port, _port, PORT_BUFFER_SIZE - 1);
	if (strcmp(proto, TCP_PROTO) == 0) {
		tcp = true;
	} else {
		tcp = false;
		if (strcmp(proto, UDP_PROTO) != 0) {
			printf("Network::init_socket Bad protocol '%s'.\n", proto);
			return networkBadArguments;			
		}
	}
	return networkOK;
}

NetworkError Network::is_connected(bool& _connected)
{
	if (!connected) {
		return networkBadArguments;
	}
	_connected = connected;
	return networkOK;
}

NetworkError Network::connect(const char* _hostname, const char* _port, const char* proto)
{
	NetworkError res = init_socket_type(_hostname, _port, proto);
	if (res != networkOK) {
		return res;
	}

	addrinfo  address;
    addrinfo  *result = NULL, *rp = NULL;
	
    memset(&address, 0, sizeof(addrinfo));
    address.ai_family = AF_UNSPEC;    // IPv4 / IPv6
	if (tcp) {
		address.ai_socktype = SOCK_STREAM;
	} else {
		address.ai_socktype = SOCK_DGRAM;
	}
	
	if (getaddrinfo(hostname, port, &address, &result) < 0) {
		printf("Network::connect Bad connect address %s:%s\n", hostname, port);
		return networkBadAddress;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd < 0) {
			printf("Network::connect Socket init error: %d\n", errno);
			continue;
		}
		if (::connect(fd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}
		printf("Network::connect error: %d\n", errno);
		close(fd);
	}

    freeaddrinfo(result);
			
	if (rp == NULL) {
		printf("Network::connect No connect address %s:%s\n", hostname, port);
		return networkBadAddress;
	}

	res = set_nonblocking(fd);
	if (res != networkOK) {
		close(fd);
		fd = -1;
		return res;
	}
	printf("Network::connect Connected to %s:%s proto %s.\n", hostname, port, proto);
	server = false;
	connected = true;
	return networkOK;
}

NetworkError Network::set_nonblocking(int _fd)
{
	if (_fd < 0) {
		return networkBadArguments;
	}

#ifdef WIN32
	int mode = 1;
	int err;
	if ((err = ioctlsocket(m_socket, FIONBIO, &mode)) != NO_ERROR) {
		printf("Network::set_nonblocking error %d\n", err);		
		return networkError;
	}
#else
	int flags = fcntl(_fd, F_GETFL);
	if (flags < 0) {
		printf("Network::set_nonblocking fcntl error %d\n", errno);
		return networkError;
	}
	int res;
	if ((res = fcntl(_fd, F_SETFL, flags | O_NONBLOCK)) < 0) {
		printf("Network::set_nonblocking flag error %d\n", errno);
		return networkError;
	}
#endif
	
	return networkOK;
	
}

NetworkError Network::listen(const char* _hostname, const char* _port, const char* proto)
{
	NetworkError res = init_socket_type(_hostname, _port, proto);
	if (res != networkOK) {
		return res;
	}
	
	addrinfo  address;
    addrinfo  *result = NULL, *rp = NULL;
	
    memset(&address, 0, sizeof(addrinfo));
    address.ai_family = AF_UNSPEC;    // IPv4 / IPv6
	if (tcp) {
		address.ai_socktype = SOCK_STREAM;
	} else {
		address.ai_socktype = SOCK_DGRAM;
	}
	
	if (getaddrinfo(hostname, port, &address, &result) < 0) {
		printf("Network::listen Bad listen address %s:%s\n", hostname, port);
		return networkBadAddress;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (listen_fd < 0) {
			printf("Network::listen Socket init error: %d\n", errno);
			continue;
		}
		if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
			if (!tcp || ::listen(listen_fd, 1) == 0) {
				break;
			}
		}
		printf("Network::listen bind error: %d\n", errno);
		close(listen_fd);
	}

    freeaddrinfo(result);

	if (rp == NULL) {
		printf("Network::listen No listen socket %s:%s\n", hostname, port);
		return networkBadAddress;		
	}

	res = set_nonblocking(listen_fd);
	if (res != networkOK) {
		return res;
	}
	printf("Network::listen Listening on %s:%s proto %s.\n", hostname, port, proto);
	server = true;
	connected = false;
	return networkOK;	
}

NetworkError Network::has_connection(bool& available)
{
	if (!server || fd < 0) {
		return networkBadArguments;
	}
	if (!tcp) {
		available = true;
		return networkOK;		
	}

	fd_set set;
	FD_ZERO(&set); /* clear the set */
	FD_SET(listen_fd, &set); /* add our file descriptor to the set */
	
	int res = select(listen_fd + 1, &set, NULL, NULL, NULL);
	if (res < 0) {
		printf("Network::has_connection select error %d\n", errno);
		return networkError;
	}
	available = res != 0;

	return networkOK;
}

NetworkError Network::wait_connection(unsigned int millisec)
{
	if (!server || listen_fd < 0) {
		return networkBadArguments;
	}
	if (tcp) {
		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = millisec * 1000;
		
		fd_set set;
		FD_ZERO(&set); /* clear the set */
		FD_SET(listen_fd, &set); /* add our file descriptor to the set */
		
		int r = select(listen_fd + 1, &set, NULL, NULL, &timeout);
		if (r < 0) {
			printf("Network::wait_connection select error %d\n", errno);
			return networkError;
		}
		if (r == 0) {
			printf("Network::wait_connection timeout error\n");
			return networkTimeout;
		}

		NetworkError res = accept_connection();
		if (res != networkOK) {
			return res;
		}
	} else {
		fd = listen_fd;
		listen_fd = -1;
	}
	return networkOK;
}

NetworkError Network::accept_connection()
{
	if (!server || listen_fd < 0) {
		return networkBadArguments;
	}
	if (tcp) {
		fd = accept(listen_fd, NULL, NULL);
		if (fd < 0) {
			printf("Network::accept_connection accept error %d\n", errno);
			return networkError;
		}
		
		NetworkError res = set_nonblocking(listen_fd);
		if (res != networkOK) {
			return res;
		}
	} else {
		fd = listen_fd;
		listen_fd = -1;
	}
	printf("Network::accept_connection Connection accepted.\n");
	connected = true;
	return networkOK;
}

NetworkError Network::has_data(bool& available, unsigned int millisec)
{
	if (!connected || fd < 0) {
		return networkBadArguments;
	}

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = millisec;
	
	fd_set set;
	FD_ZERO(&set); /* clear the set */
	FD_SET(fd, &set); /* add our file descriptor to the set */

	int r;
	if (millisec > 0) {
		r = select(fd + 1, &set, NULL, NULL, &timeout);
	} else {
		r = select(fd + 1, &set, NULL, NULL, NULL);
	}
	if (r < 0) {
		printf("Network::has_data select error %d\n", errno);
		return networkError;
	}
	available = r != 0;

	return networkOK;
}

NetworkError Network::send_data(const char* buffer, size_t buffer_len)
{
	int res = send(fd, buffer, buffer_len, 0);
	if (res < 0) {
		return networkReadError;
	}
	return networkOK;
}

NetworkError Network::recv_data(char* buffer, size_t buffer_len, size_t& received)
{
	int res = recv(fd, buffer, buffer_len, 0);
	if (res < 0) {
		return networkReadError;
	} else if (res == 0) {
		received = 0;
	} else {
		counter += (size_t)res;
		received = (size_t)res;
		if (counter > 1024 * 1024) {
			printf("1M\n");
			counter -= 1024 * 1024;
		}
	}
	return networkOK;
}

NetworkError Network::disconnect()
{
	if (!connected && !server) {
		return networkBadArguments;
	}
	if (fd >= 0) { 
		close(fd);
	}
	if (listen_fd >= 0) {
		close(listen_fd);
	}
	fd = listen_fd = -1;
	connected = server = false;
	return networkOK;
}
