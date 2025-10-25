/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * The Robot API implementation
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
#include <math.h>

#include "robot.h"
#include "pass_labirinth.h"
#include "debug.h"

using namespace HSMRobot;

Robot::Robot()
{
	memset(&p, 0, sizeof(TelemetryPacket));
	v = w = 0.0;
	start_a = target_a = 0.0;
	turning = slowing = accelerating = false;
	window_dist = focus_dist = center_dist = 0.0;
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
	DEBUG("Robot: initialized\n");
	return networkOK;
}

void Robot::go_ahead()
{
	v = MAX_V;
	DEBUG("Robot: go ahead\n");
	accelerating = true;
	send_command();
}

void Robot::stop()
{
	v = 0.0;
	DEBUG("Robot: stop\n");
	accelerating = true;
	send_command();
}

void Robot::stop_turn()
{
	w = 0.0;
	slowing = true;
	DEBUG("Robot: stop turn\n");
	send_command();
}

void Robot::turn_left()
{
	w = MAX_W / 2.0;
	turning = true;
	slowing = false;
	DEBUG("Robot: turn left\n");
	send_command();
}

void Robot::turn_right()
{
	w = -MAX_W / 2.0;
	turning = true;
	slowing = false;
	DEBUG("Robot: turn right\n");
	send_command();
}

void Robot::turn_faster()
{
	w *= 2.0;
	DEBUG("Robot: turn faster\n");
	send_command();
}

void Robot::turn_slower()
{
	w /= 2.0;
	DEBUG("Robot: turn slower\n");
	send_command();
}

void Robot::process()
{
	bool ready;
	get_telemetry(ready);
	if (ready) {
		lidar_analysis();
		if (turning && slowing) {
			if (abs(p.h.vth - w) <= DELTA) {
				slowing = false;
				turning = false;
				DEBUG("Robot: TURN_DONE");
				QEvt e;
				e.sig = TURN_DONE_SIG;
				QMSM_DISPATCH(the_pass_labirinth, (QEvt *)&e);
			}
		}
		if (accelerating) {
			float horiz_v = sqrt(p.h.vx * p.h.vx + p.h.vy * p.h.vy); 
			if (abs(horiz_v) <= DELTA) {
				accelerating = false;
				DEBUG("Robot: STOPPED");
				QEvt e;
				e.sig = STOPPED_SIG;
				QMSM_DISPATCH(the_pass_labirinth, (QEvt *)&e);
			}
		}
	}
}

void Robot::send_command()
{
	cmd.send(v, w);
}

void Robot::get_telemetry(bool& ready)
{
	ready = false;
	
	TelemetryPacket tmp_p;
	memset(&tmp_p, 0, sizeof(TelemetryPacket));
	bool packet, available;
	packet = available = false;
	NetworkError res = telemetry.has_packet(available);
	if (res != networkOK) {
		printf("Robot::get_telemetry has packet error: %d\n", res);
		return;
	}
	if (!available) {
		return ;
	}
	//printf("packet available\n");
	res = telemetry.get_packet(tmp_p, packet);
	if (res != networkOK) {
		printf("Robot::get_telemetry get packet error: %d\n", res);
		return;
	}
	if (packet) {
		ready = true;
		memcpy(&p, &tmp_p, sizeof(TelemetryPacket));
		DEBUG("pos=(%.2f,%.2f) th=%.2f° v=(%.2f,%.2f,%.2f) w=(%.2f,%.2f,%.2f) L n=%u, [%.2f, ..., %.2f] W:%.2f F:%.2f C:%.2f\n",
			  p.h.odom_x, p.h.odom_y, p.h.odom_th, p.h.vx, p.h.vy, p.h.vth, p.h.wx, p.h.wy, p.h.wz, p.h.n,
			  p.points[0], p.points[p.h.n - 1], window_dist, focus_dist, center_dist);
	}
}

void Robot::lidar_detect(float* X, uint32_t n, float& window, float& focus, float& center)
{
	const float WINDOW = 1/6.0; // ...[1/6...5/6]...
	uint32_t i;
	uint32_t window_from_i = uint32_t(WINDOW * n);
	uint32_t window_to_i = uint32_t((1.0 - WINDOW) * n);
	const float FOCUS = 1.0/4.0;   // ...[1/4...3/4]...
	uint32_t focus_from_i = uint32_t(FOCUS * n);
	uint32_t focus_to_i = uint32_t((1.0 - FOCUS) * n);
	const float CENTER = 9.0/20.0;   // ...[9/20...11/20]...
	uint32_t center_from_i = uint32_t(CENTER * n);
	uint32_t center_to_i = uint32_t((1.0 - CENTER) * n);

	float mean_window = 0.0, mean_focus = 0.0, mean_center = 0.0;
	
	for (i = window_from_i; i <= window_to_i; i++) {
		float x = X[i];

		if (i >= center_from_i && i <= center_to_i) {
			mean_center += x;
		}
		if (i >= focus_from_i && i <= focus_to_i) {
			mean_focus += x;
		}
		mean_window += x;
	}
	
	center = mean_center / (center_to_i - center_from_i + 1);
	focus = mean_focus / (focus_to_i - focus_from_i + 1);
	window = mean_center / (center_to_i - center_from_i + 1);
}

void Robot::lidar_analysis()
{
	QEvt e;

	if (p.h.n == 0) {
		return ;
	}
	
	lidar_detect(p.points, p.h.n, window_dist, focus_dist, center_dist);
		
	if (center_dist < OBSTACLE_RANGE) {
		DEBUG("Robot: OBSTACLE_COLLISION\n");
		e.sig = OBSTACLE_COLLISION_SIG;
		QMSM_DISPATCH(the_pass_labirinth, (QEvt *)&e);		
	}
	if (focus_dist >= LIDAR_OK_RANGE) {
		DEBUG("Robot: OPEN_SPACE\n");
		e.sig = OPEN_SPACE_SIG;
		QMSM_DISPATCH(the_pass_labirinth, (QEvt *)&e);
	} else if (focus_dist >= LIDAR_OK_RANGE * 3.0 / 4.0) {
		DEBUG("Robot: HALF_OPEN_SPACE\n");
		e.sig = HALF_OPEN_SPACE_SIG;
		QMSM_DISPATCH(the_pass_labirinth, (QEvt *)&e);		
	} else {
		DEBUG("Robot: CLOSED_SPACE\n");
		e.sig = CLOSED_SPACE_SIG;
		QMSM_DISPATCH(the_pass_labirinth, (QEvt *)&e);		
	}
}

bool Robot::bridge_detected()
{
	return false;
}
