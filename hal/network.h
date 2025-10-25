/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Base tcp/udp network commication module declaration
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
		
		NetworkError connect(const char* hostname, const char* port, const char* proto);
		NetworkError disconnect();
		NetworkError listen(const char* hostname, const char* port, const char* proto);
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
		NetworkError init_socket_type(const char* hostname, const char* port, const char* proto);
		NetworkError set_nonblocking(int fd);
		
		char         hostname[ADDRESS_BUFFER_SIZE];
		char         port[PORT_BUFFER_SIZE];

		bool         server;
		int          listen_fd;
		int          fd;
		size_t       counter;
	};

}

#endif
