/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Commands commication module declaration
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
 *
 * ----------------------------------------------------------------------------- */

#include <stdlib.h>

#ifndef __HSM_ROBOT_COMMANDS
#define __HSM_ROBOT_COMMANDS

#include "network.h"

namespace HSMRobot {
	
	class Commands: protected Network {
	public:
		Commands();
		~Commands();
		
		NetworkError initialize(const char* hostname, const char* port);
		NetworkError send(float v, float a);
	};

}

#endif
