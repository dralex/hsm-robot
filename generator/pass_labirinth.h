// -----------------------------------------------------------------------------
// Model: pass_labirinth
// File:  pass_labirinth.h
// -----------------------------------------------------------------------------

#ifndef pass_labirinth_h
#define pass_labirinth_h

#include "qhsm.h"         // include the framework header
#include "assert_exit.h"  // assertion functions

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
/* protected: */
    QHsm super;

/* public: */
    unsigned int turn;
} Pass_labirinth;

/* protected: */
QState Pass_labirinth_initial(Pass_labirinth * const me, void const * const par);
QState Pass_labirinth_pass_labirinth(Pass_labirinth * const me, QEvt const * const e);
QState Pass_labirinth_stop(Pass_labirinth * const me, QEvt const * const e);
QState Pass_labirinth_go(Pass_labirinth * const me, QEvt const * const e);
QState Pass_labirinth_guided_turn(Pass_labirinth * const me, QEvt const * const e);
QState Pass_labirinth_fast_turn(Pass_labirinth * const me, QEvt const * const e);
QState Pass_labirinth_slow_turn(Pass_labirinth * const me, QEvt const * const e);
QState Pass_labirinth_stop_turn(Pass_labirinth * const me, QEvt const * const e);
QState Pass_labirinth_start_turn(Pass_labirinth * const me, QEvt const * const e);
QState Pass_labirinth_left(Pass_labirinth * const me, QEvt const * const e);
QState Pass_labirinth_right(Pass_labirinth * const me, QEvt const * const e);
typedef struct pass_labirinth_QEvt {
    QEvt super;
    
} pass_labirinth_QEvt;

enum PlayerSignals {
  TIME_TICK_SEC_SIG = Q_USER_SIG,

  TIME_TICK_SIG,
CLOSED_SPACE_SIG,
  OBSTACLE_COLLISION_SIG,
  STOPPED_SIG,
  TURN_DONE_SIG,
  HALF_OPEN_SPACE_SIG,
  OPEN_SPACE_SIG,

  LAST_USER_SIG
};
extern QHsm * const the_pass_labirinth; /* opaque pointer to the pass_labirinth HSM */

void Pass_labirinth_ctor(
    unsigned int turn);
#ifdef __cplusplus
}
#endif
#endif // pass_labirinth_h
