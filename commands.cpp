/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Commands commication module implementation
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
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

NetworkError Commands::initialize(const char* hostname, unsigned int port)
{
	NetworkError res = Network::connect(hostname, port, false);
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
