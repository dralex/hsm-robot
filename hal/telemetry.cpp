/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Telemetry network commication module implementation
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
 *
 * ----------------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

#include "telemetry.h"

using namespace HSMRobot;

const char* HEADER = "WBTG";

Telemetry::Telemetry():
	Network()
{
}

Telemetry::~Telemetry()
{
}

NetworkError Telemetry::initialize(const char* _hostname, const char* _port, const char* proto, unsigned int timeout)
{
	NetworkError res = Network::listen(_hostname, _port, proto);
	if (res != networkOK) {
		return res;
	}

	res = wait_connection(timeout);
	if (res != networkOK) {
		return res;
	}

	buffer_read = 0;
	
	return networkOK;
}

NetworkError Telemetry::has_packet(bool& available)
{
	if (!connected) {
		return networkBadArguments;
	}
	NetworkError res = has_data(available);
	return res;
}

NetworkError Telemetry::get_packet(TelemetryPacket& packet)
{
	NetworkError res;
	size_t bytes_read = 0;
	if (!tcp) {
		res = recv_data(buffer, TELEMETRY_BUFFER_LEN, bytes_read);
		if (res != networkOK) {
			return res;
		}
	} else {
		char bytes[4];
		res = recv_data(bytes, 4, bytes_read);
		if (res != networkOK || bytes_read < 4) {
			printf("Telemetry::get_packet Cannot read packet size %d\n", res);
			return res;
		}
		size_t packet_len = *((uint32_t*)bytes); // little-endian
		if (packet_len > TELEMETRY_BUFFER_LEN) {
			printf("Telemetry::get_packet Bad packet size %lu\n", packet_len);
			packet_len = TELEMETRY_BUFFER_LEN;
		}
		res = recv_data(buffer, packet_len, bytes_read);
		if (res != networkOK) {
			printf("Telemetry::get_packet Cannot read packet %d\n", res);
			return res;
		}
		if (bytes_read < packet_len) {
			printf("Telemetry::get_packet Not all bytes read %lu from %lu\n", bytes_read, packet_len);
			return networkDecodeError;
		}
	}
	if (bytes_read < 4 + sizeof(TelemetryPacketMin)) {
		printf("Telemetry::get_packet Small package %lu\n", bytes_read);
		return networkDecodeError;
	}
	if (strncmp(buffer, HEADER, 4) != 0) {
		printf("Telemetry::get_packet Bad packet header %c%c%c%c\n", buffer[0], buffer[1], buffer[2], buffer[3]);
		return networkDecodeError;
	}
	TelemetryPacketMin* p = (TelemetryPacketMin*)(buffer + 4);
	if (p->n > MAX_LIDAR_POINTS) {
		printf("Telemetry::get_packet Too many lidar points%u\n", p->n);
		return networkDecodeError;
	}
	packet.h = *p;
	float* b = (float*)buffer + 4 + sizeof(TelemetryPacketMin);
	for (uint32_t i = 0; i < p->n; i++) {
		packet.points[i] = b[i];
	}

	return networkOK;
}


