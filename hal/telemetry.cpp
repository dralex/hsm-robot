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
#include <math.h>

#include "telemetry.h"

using namespace HSMRobot;

const char* HEADER = "WBTG";

Telemetry::Telemetry():
	Network(), buffer_read(0), bytes_left(0)
{
	FILE* f = fopen(LOG_FILE, "w");
	fclose(f);
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
	return has_data(available);
}

NetworkError Telemetry::get_packet(TelemetryPacket& packet, bool& ready)
{
	NetworkError res;
	size_t bytes_read = 0, bytes_left = 0, skip = 0;
	char *sbuffer = buffer + 4, *pbuffer = buffer;
	ready = false;
	if (!tcp) {
		res = recv_data(buffer, BIG_BUFFER_LEN, bytes_read);
		if (res != networkOK) {
			return res;
		}
		buffer_read = bytes_read;
	} else {
		res = recv_data(buffer + buffer_read, BIG_BUFFER_LEN - buffer_read, bytes_read);
		if (res != networkOK) {
			printf("Telemetry::get_packet Cannot read packet size %d\n", res);
			return res;
		}
		buffer_read += bytes_read;
		// find last full packet
		bytes_left = 0;
		while (pbuffer < buffer + buffer_read) {
			size_t plen = *((uint32_t*)pbuffer); // little-endian
			if (pbuffer + sizeof(uint32_t) + plen <= buffer + buffer_read) {
				sbuffer = pbuffer + sizeof(uint32_t);
				pbuffer += sizeof(uint32_t) + plen;
			} else {
				bytes_left = pbuffer + sizeof(uint32_t) + plen - (buffer + buffer_read);
				break;
			}
			skip++;
		}
	}
	if (bytes_read < 4 + 4 + sizeof(TelemetryPacketMin)) {
		printf("Not enough data");
		ready = false;
		return networkOK;
	}
	if (strncmp(sbuffer, HEADER, 4) != 0) {
		printf("Telemetry::get_packet Bad packet header %c%c%c%c\n",
			   sbuffer[0], sbuffer[1], sbuffer[2], sbuffer[3]);
		return networkDecodeError;
	}
	TelemetryPacketMin* p = (TelemetryPacketMin*)(sbuffer + 4);
	if (p->n > MAX_LIDAR_POINTS) {
		printf("Telemetry::get_packet Too many lidar points%u\n", p->n);
		return networkDecodeError;
	}
	packet.h = *p;
	packet.h.odom_th = packet.h.odom_th * 180.0 / M_PI;  
	float* b = (float*)sbuffer + 4 + sizeof(TelemetryPacketMin);
	for (uint32_t i = 0; i < p->n; i++) {
		float x = b[i];
		if (x < 0) x = 0.0;
		if (x > 8.0) x = 8.0; // TODO: remove hardcode
		packet.points[i] = x;
	}
	log_telemetry(packet);
	ready = true;
	if (bytes_left > 0) {
		memcpy(buffer, pbuffer, bytes_left);
	}
	buffer_read = bytes_left;
	printf("skip %lu ", skip);
	return networkOK;
}

void Telemetry::log_telemetry(const TelemetryPacket& packet)
{
	FILE* f = fopen(LOG_FILE, "a");
	for (uint32_t i = 0; i < packet.h.n; i++) {
		fprintf(f, "%.2f", packet.points[i]);
		if (i < packet.h.n - 1) {
			fprintf(f, ",");
		}
	}
	fprintf(f, "\n");
	fclose(f);
}
