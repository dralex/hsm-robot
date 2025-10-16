/* -----------------------------------------------------------------------------
 * The HSM Robot framework
 *
 * Telemetry network commication module declaration
 *
 * Copyright (C) 2025 Alexey Fedoseev <aleksey@fedoseev.net>
 *
 * ----------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdint.h>

#ifndef __HSM_ROBOT_TELEMETRY
#define __HSM_ROBOT_TELEMETRY

#include "network.h"

namespace HSMRobot {

	const size_t MAX_LIDAR_POINTS = 512;
	const size_t TELEMETRY_PACKET_LEN_MIN = sizeof(float) * 9 + sizeof(uint32_t);
	const size_t TELEMETRY_BUFFER_LEN = TELEMETRY_PACKET_LEN_MIN + MAX_LIDAR_POINTS * sizeof(float);

	struct TelemetryPacketMin {
		float odom_x;                   // одометрия X (м)
		float odom_y;                   // одометрия Y (м)
		float odom_th;                  // угол робота (рад)
		float vx;                       // скорость по X (м/с)
		float vy;                       // скорость по Y (м/с)
		float vth;                      // угловая скорость (рад/с)
		float wx;                       // угловая скорость по оси X (roll rate, рад/с)
		float wy;                       // угловая скорость по оси Y (pitch rate, рад/с) 
		float wz;                       // угловая скорость по оси Z (yaw rate, рад/с) 
		uint32_t n;                     // кол-во точек лидара (N, uint32, 4 байта).
	};
	struct TelemetryPacket {
		TelemetryPacketMin h;
		float              points[MAX_LIDAR_POINTS]; // N значений float32 — массив дистанций лидара (м).
	};

	class Telemetry: protected Network {
	public:
		Telemetry();
		~Telemetry();

		NetworkError initialize(const char* hostname, unsigned int port, bool tcp, unsigned int timeout);
		NetworkError is_connected(bool& connected) { return Network::is_connected(connected); }
		NetworkError has_packet(bool& available);
		NetworkError get_packet(TelemetryPacket& packet);

	private:
		char   buffer[TELEMETRY_BUFFER_LEN];
		size_t buffer_read;
	};

}

#endif
