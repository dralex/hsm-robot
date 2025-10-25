/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Commands commication module implementation
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

#include "commands.h"

using namespace HSMRobot;
	
Commands::Commands():
	Network()
{
}

Commands::~Commands()
{
}

NetworkError Commands::initialize(const char* _hostname, const char* _port)
{
	NetworkError res = Network::connect(_hostname, _port, "udp");
	if (res != networkOK) {
		return res;
	}
	
	return networkOK;
}

NetworkError Commands::send(float v, float a)
{
	char buffer[8];
	float* fbuf = (float*)buffer;
	fbuf[0] = v;
	fbuf[1] = a;	
	NetworkError r = send_data(buffer, 8);
	return r;
}
