/* -----------------------------------------------------------------------------
 * The HSM-to-C++ framework: main function
 * ----------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qhsm.h"
#include "debug.h"
#include "pass_labirinth.h"

/* ----------------------------------------------------------------------------- */
// MAIN_C_INIT initialization from the graphml diagram
#include <stdlib.h>
#include <stdio.h>
#include "robot.h"
#include "assert_exit.h"
#include "debug.h"
#define TICK_LEN 100000                 // TIck in mcs
#define CONNECTION_TIMEOUT 3000 // Telemetry connection timrout
HSMRobot::Robot robot;
/* ----------------------------------------------------------------------------- */
#ifdef WIN32
#include <windows.h>
void usleep(unsigned int usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * (__int64)usec);

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}
#else
#include <unistd.h>
#endif
/* ----------------------------------------------------------------------------- */
int main()
{
	uint32_t        seconds = 0;
	uint32_t        ticks = 0;
#ifdef TICK_LEN
	static uint32_t tick_len = TICK_LEN;
#else
	static uint32_t tick_len = TICK_LEN;
#endif
	static int      tick_cnt = 1;

	DEBUG("Pass_labirinth State Machine");

	// MAIN_INIT initialization from the graphml diargam
	    HSMRobot::NetworkError res;
    const char *cmd_host = "127.0.0.1", *cmd_port = "5555";
    const char *tel_host = "0.0.0.0", *tel_port = "5600", *tel_proto = "tcp";
    if (getenv("CMD_HOST")) {
    cmd_host = getenv("CMD_HOST");
    }
    if (getenv("CMD_PORT")) {
    cmd_port = getenv("CMD_PORT");
    }
    if (getenv("TEL_HOST")) {
    tel_host = getenv("TEL_HOST");
    }
    if (getenv("TEL_PORT")) {
    tel_port = getenv("TEL_PORT");
    }
    if (getenv("PROTO")) {
    tel_proto = getenv("PROTO");
    }
    res = robot.initialize(cmd_host, cmd_port, tel_host, tel_port, tel_proto, CONNECTION_TIMEOUT);
    if (res != HSMRobot::networkOK) {
    printf("Robot init error: %d\n", res);
    return 1;
    }
	// The SM constructor
	Pass_labirinth_ctor(0);


	QMSM_INIT(the_pass_labirinth, (QEvt *)0);
    QEvt e;

    for (;;) {
		/*QState r;
		  const char* msg = "TIME_TICK";*/
		usleep(tick_len);
		e.sig = TIME_TICK_SIG;
		ticks++;
		// dispatch the event into the state machine
		/*r = */QMSM_DISPATCH(the_pass_labirinth, (QEvt *)&e);
        if (--tick_cnt == 0) {                 // time for the tick?
            tick_cnt = 1000 * 1000 / tick_len; // tick per second
            e.sig = TIME_TICK_SEC_SIG;
            /* msg = "TIME_TICK_SEC";*/
            seconds++;
            // dispatch the event into the state machine
            /*r = */QMSM_DISPATCH(the_pass_labirinth,  (QEvt *)&e);
        }

		robot.process();
	}
    return 0;
}

/* ----------------------------------------------------------------------------- */
void assert_exit(const char* file, int line, const char* message)
{
    printf("Assertion failed '%s' in %s, line %d", message, file, line);
    exit(1);
}
