/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Base tcp/udp network commication module declaration
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
 *
 * ----------------------------------------------------------------------------- */

#include <stdlib.h>

#ifndef __HSM_ROBOT_NETWORK
#define __HSM_ROBOT_NETWORK

namespace HSMRobot {

	const size_t ADDRESS_BUFFER_SIZE = 64;
	const size_t PORT_BUFFER_SIZE = 8;
	
	enum NetworkError {
		networkOK              = 0,
		networkBadArguments,
		networkBadAddress,
		networkTimeout,
		networkConnectionError,
		networkReadError,
		networkWriteError,
		networkDecodeError,
		networkError
	};
	
	class Network {
	public:
		Network();
		virtual ~Network();
		
		NetworkError connect(const char* hostname, unsigned int port, bool tcp = false);
		NetworkError disconnect();
		NetworkError listen(const char* hostname, unsigned int port, bool tcp = false);
		NetworkError has_connection(bool& available);
		NetworkError wait_connection(unsigned int timeout);
		NetworkError accept_connection();
		NetworkError is_connected(bool& connected);
		
		NetworkError has_data(bool& available, unsigned int timeout = false);
		NetworkError send_data(const char* buffer, size_t buffer_len);
		NetworkError recv_data(char* buffer, size_t buffer_len, size_t& received);

	protected:
		bool         connected;
		bool         tcp;
		
	private:
		NetworkError init_socket(const char* hostname, unsigned int port, bool tcp);
		NetworkError set_nonblocking(int fd);
		
		char         hostname[ADDRESS_BUFFER_SIZE];
		char         port[PORT_BUFFER_SIZE];

		bool         server;
		int          listen_fd;
		int          fd;
	};

}

#endif
