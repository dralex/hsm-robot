/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Commands commication module declaration
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
