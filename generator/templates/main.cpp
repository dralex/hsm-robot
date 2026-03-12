/* -----------------------------------------------------------------------------
 * The HSM-to-C++ framework: main function
 * ----------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "qhsm.h"
#include "debug.h"
/*$$INCLUDE$$*/
/* ----------------------------------------------------------------------------- */
// MAIN_C_INIT initialization from the graphml diagram
/*$$MAIN_C_INIT$$*/
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

	DEBUG(/*$$MACHINE$$*/);

	// MAIN_INIT initialization from the graphml diargam
	/*$$MAIN_INIT$$*/
	// The SM constructor
	/*$$CONSTRUCTOR$$*/

	QMSM_INIT(/*$$SM_CALL$$*/, (QEvt *)0);
    QEvt e;

    for (;;) {
		/*QState r;
		  const char* msg = "TIME_TICK";*/
		usleep(tick_len);
		e.sig = TIME_TICK_SIG;
		ticks++;
		// dispatch the event into the state machine
		/*r = */QMSM_DISPATCH(/*$$SM_CALL$$*/, (QEvt *)&e);
        if (--tick_cnt == 0) {                 // time for the tick?
            tick_cnt = 1000 * 1000 / tick_len; // tick per second
            e.sig = TIME_TICK_SEC_SIG;
            /* msg = "TIME_TICK_SEC";*/
            seconds++;
            // dispatch the event into the state machine
            /*r = */QMSM_DISPATCH(/*$$SM_CALL$$*/,  (QEvt *)&e);
        }

		/*$$MAIN_LOOP$$*/
	}
    return 0;
}

/* ----------------------------------------------------------------------------- */
void assert_exit(const char* file, int line, const char* message)
{
    printf("Assertion failed '%s' in %s, line %d", message, file, line);
    exit(1);
}
