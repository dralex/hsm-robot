/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Base tcp/udp network commication module implementation
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
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

Network::Network():
	connected(false), tcp(false), server(false), listen_fd(-1), fd(-1)
{
	*hostname = 0;
}

Network::~Network()
{
	if (connected || server) {
		disconnect();
	}
}

NetworkError Network::init_socket(const char* _hostname, unsigned int _port, bool _tcp)
{
	if (connected || server) return networkBadArguments;
	if (!_hostname) return networkBadArguments;

	strncpy(hostname, _hostname, ADDRESS_BUFFER_SIZE - 1);
	snprintf(port, PORT_BUFFER_SIZE - 1, "%u", _port);
	tcp = _tcp;
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

NetworkError Network::connect(const char* _hostname, unsigned int _port, bool _tcp)
{
	NetworkError res = init_socket(_hostname, _port, _tcp);
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
		fprintf(stderr, "Bad connect address %s:%s\n", hostname, port);
		return networkBadAddress;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (fd < 0) {
			fprintf(stderr, "Socket init error: %d\n", fd);
			continue;
		}
		if (::connect(fd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}
		close(fd);
	}

    freeaddrinfo(result);
			
	if (rp == NULL) {
		fprintf(stderr, "No connect address %s:%s\n", hostname, port);
		return networkBadAddress;
	}

	res = set_nonblocking(fd);
	if (res != networkOK) {
		close(fd);
		fd = -1;
		return res;
	}
	fprintf(stderr, "Connected to %s:%s.\n", hostname, port);
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
	// TODO
	int mode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &mode) != NO_ERROR) {
		return networkError;
	}
#else
	int flags = fcntl(_fd, F_GETFL);
	if (flags < 0) {
		fprintf(stderr, "fcntl error %d\n", flags);
		return networkError;
	}
	int res;
	if ((res = fcntl(_fd, F_SETFL, flags | O_NONBLOCK)) < 0) {
		fprintf(stderr, "set nonblocking flag error %d\n", res);
		return networkError;		
	}
#endif
	
	return networkOK;
	
}

NetworkError Network::listen(const char* _hostname, unsigned int _port, bool _tcp)
{
	NetworkError res = init_socket(_hostname, _port, _tcp);
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
		fprintf(stderr, "Bad listen address %s:%s\n", hostname, port);
		return networkBadAddress;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (listen_fd < 0) {
			continue;
		}
		if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0) {
			if (::listen(listen_fd, 1) == 0) {
				break;
			}
		}
		close(listen_fd);
	}

    freeaddrinfo(result);

	if (rp == NULL) {
		fprintf(stderr, "No connect listen %s:%s\n", hostname, port);
		return networkBadAddress;		
	}

	res = set_nonblocking(listen_fd);
	if (res != networkOK) {
		return res;
	}
	fprintf(stderr, "Listening on %s:%u.\n", _hostname, _port);
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
		fprintf(stderr, "select error %d\n", res);
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
			fprintf(stderr, "select error %d\n", r);
			return networkError;
		}
		if (r == 0) {
			fprintf(stderr, "timeout error\n");
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
			fprintf(stderr, "accept error %d\n", errno);
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
	fprintf(stderr, "Connection accepted.\n");
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
		received = false;
	} else {
		received = (size_t)res;
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
