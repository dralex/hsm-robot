/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * The Robot API implementation
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
 *
 * ----------------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

#include "robot.h"

using namespace HSMRobot;

Robot::Robot()
{
}

Robot::~Robot()
{
}

NetworkError Robot::initialize(const char* cmd_host, const char* cmd_port,
							   const char* tel_host, const char* tel_port, const char* tel_proto,
							   unsigned int start_timeout_ms)
{
	NetworkError res;
	res = cmd.initialize(cmd_host, cmd_port);
	if (res != networkOK) {
		return res;
	}
	res = telemetry.initialize(tel_host, tel_port, tel_proto, start_timeout_ms);
	if (res != networkOK) {
		return res;
	}
	return networkOK;
}

// go ahead until we found a wall (see MAX_VELOCITY; STOP_BEFORE)
void Robot::go_ahead()
{
	cmd.send(MAX_VELOCITY, 0.0);
}

void Robot::get_telemetry()
{
	bool available = false;
	NetworkError res = telemetry.has_packet(available);
	if (res != networkOK) {
		printf("Robot::get_telemetry has packet error: %d\n", res);
		return;
	}
	if (available) {
		TelemetryPacket p;
		memset(&p, 0, sizeof(TelemetryPacket));
		res = telemetry.get_packet(p);
		if (res != networkOK) {
			printf("Robot::get_telemetry get packet error: %d\n", res);
			return;
		}
		printf("pos=(%.2f,%.2f) th=%.2f° v=(%.2f,%.2f,%.2f) w=(%.2f,%.2f,%.2f) lidar n=%u, [%.2f, ... %.2f]\n",
			   p.h.odom_x, p.h.odom_y, p.h.odom_th, p.h.vx, p.h.vy, p.h.vth, p.h.wx, p.h.wy, p.h.wz, p.h.n,
			   p.points[0], p.points[p.h.n - 1]);
	}
}
