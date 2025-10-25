/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * The Robot API
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

#ifndef __HSM_ROBOT_ROBOT
#define __HSM_ROBOT_ROBOT

#include "telemetry.h"
#include "commands.h"

namespace HSMRobot {

	const float MAX_V = 0.6;
	const float MAX_W = 1.0;
	const float LIDAR_MAX_RANGE = 8.0;
	const float LIDAR_OK_RANGE = 4.0;
	const float OBSTACLE_RANGE = 1.5;
	const float DELTA = 0.0001;
	
	class Robot {
	public:
		Robot();
		~Robot();
		
		NetworkError    initialize(const char* cmd_host, const char* cmd_port,
								   const char* tel_host, const char* tel_port, const char* tel_proto,
								   unsigned int start_timeout_ms);

		void            go_ahead();               // go ahead until we found a wall (see MAX_VELOCITY; STOP_BEFORE)
		void            stop();                   // stop movement
		
		void            turn_left();              // turn left
		void            turn_right();             // turn right
		void            turn_faster();            // turn faster
		void            turn_slower();            // turn slower
		void            stop_turn();              // stop turn
		bool            bridge_detected();        // is on bridge

		void            process();                // process the robot machine
		
	private:
		void            send_command();
		void            get_telemetry(bool& ready);
		void            lidar_analysis();
		void            lidar_detect(float* X, uint32_t n,
									 float& window, float& focus, float& center);

		Commands        cmd;
		Telemetry       telemetry;
		TelemetryPacket p;

		float           v, w;
		float           start_a, target_a;
		bool            turning, slowing;
		bool            accelerating;
		float           window_dist, focus_dist, center_dist;
	};

}

#endif
