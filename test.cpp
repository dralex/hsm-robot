/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Test program
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "robot.h"

using namespace HSMRobot;

const unsigned int CONNECTION_TIMEOUT = 3000;

int main(int argc, char** argv)
{
	Robot robot;
	NetworkError res;
	
	const char *cmd_host = "127.0.0.1", *cmd_port = "5555";
	const char *tel_host = "0.0.0.0", *tel_port = "5600", *tel_proto = "tcp";
	
	if (getenv("CMD_HOST")) {
		cmd_host = getenv("CMD_HOST");
	}
	if (getenv("CMD_PORT")) {
		cmd_port = getenv("CMD_PORT");
	}
	if (getenv("TEL_HOST")) {
		tel_host = getenv("TEL_HOST");
	}
	if (getenv("TEL_PORT")) {
		tel_port = getenv("TEL_PORT");
	}
	if (getenv("PROTO")) {
		tel_proto = getenv("PROTO");
	}

	res = robot.initialize(cmd_host, cmd_port,
						   tel_host, tel_port, tel_proto, CONNECTION_TIMEOUT);
	if (res != networkOK) {
		printf("Robot init error: %d\n", res);
		return 1;
	}

	robot.go_ahead();

	while (1) {
		robot.get_telemetry();
	}

	return 0;
}
