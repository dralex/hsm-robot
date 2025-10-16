/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Test program
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
 *
 * ----------------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "robot.h"

using namespace HSMRobot;

const char* cmd_host = "127.0.0.1";
const unsigned int cmd_port = 5555;

const char* tel_host = "0.0.0.0";
const unsigned int tel_port = 5600;
const bool tel_tcp = true;

const unsigned int CONNECTION_TIMEOUT = 1000;

int main(int argc, char** argv)
{
	Robot robot;
	NetworkError res;

	res = robot.initialize(cmd_host, cmd_port,
						   tel_host, tel_port, tel_tcp, CONNECTION_TIMEOUT);
	if (res != networkOK) {
		fprintf(stderr, "init error: %d\n", res);
		return 1;
	}

	robot.go_ahead();

	while (1) {
		robot.get_telemetry();
	}

	return 0;
}
