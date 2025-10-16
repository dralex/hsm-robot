/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * The Robot API
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
 *
 * ----------------------------------------------------------------------------- */

#ifndef __HSM_ROBOT_ROBOT
#define __HSM_ROBOT_ROBOT

#include "telemetry.h"
#include "commands.h"

namespace HSMRobot {

	const float MAX_VELOCITY = 0.6;
	const float STOP_BEFORE = 1.5;
	
	class Robot {
	public:
		Robot();
		~Robot();
		
		NetworkError initialize(const char* cmd_host, unsigned int cmd_port,
								const char* tel_host, unsigned int tel_port, bool tel_tcp,
								unsigned int start_timeout_ms);

		void         go_ahead();         // go ahead until we found a wall (see MAX_VELOCITY; STOP_BEFORE)
		//void         turn_to_find_way();

		void         get_telemetry();
		
	private:
		Commands     cmd;
		Telemetry    telemetry;
	};

}

#endif
