// -----------------------------------------------------------------------------
// Model: pass_labirinth
// File:  pass_labirinth.cpp
// -----------------------------------------------------------------------------
#include "qhsm.h"
#include "pass_labirinth.h"
#include <stdint.h>

#include "robot.h"
#include "assert_exit.h"
#include "debug.h"
extern HSMRobot::Robot robot;

/* global-scope definitions -----------------------------------------*/
// the only instance of the Pass_labirinth class
static Pass_labirinth pass_labirinth;
QHsm * const the_pass_labirinth = (QHsm *) &pass_labirinth; // the opaque pointer

void Pass_labirinth_ctor(
    unsigned int turn)
{
    Pass_labirinth *me = &pass_labirinth;

    QHsm_ctor(&me->super, Q_STATE_CAST(&Pass_labirinth_initial));
    me->turn = turn;
}
QState Pass_labirinth_initial(Pass_labirinth * const me, void const * const par) {
    /* SM::Pass_labirinth::SM::initial
    left */
    return Q_TRAN(&Pass_labirinth_left);
}
/* SM::pass_labirinth::pass_labirinth */
QState Pass_labirinth_pass_labirinth(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth */
        case Q_ENTRY_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
/* SM::pass_labirinth::pass_labirinth::stop */
QState Pass_labirinth_stop(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth::stop */
        case Q_ENTRY_SIG: {
            robot.stop();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth::stop} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        /*. SM::pass_labirinth::pass_labirinth::stop::STOPPED */
        case STOPPED_SIG: {
            if (me->turn % 2 == 0) {
                status_ = Q_TRAN(&Pass_labirinth_left);
            }
            else if (me->turn % 2 == 1) {
                status_ = Q_TRAN(&Pass_labirinth_right);
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Pass_labirinth_pass_labirinth);
            break;
        }
    }
    return status_;
}
/* SM::pass_labirinth::pass_labirinth::go */
QState Pass_labirinth_go(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth::go */
        case Q_ENTRY_SIG: {
            robot.go_ahead();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth::go} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        /*. SM::pass_labirinth::pass_labirinth::go::OBSTACLE_COLLISION */
        case OBSTACLE_COLLISION_SIG: {
            if (!robot.bridge_detected()) {
                status_ = Q_TRAN(&Pass_labirinth_stop);
            }
            else {
                status_ = Q_UNHANDLED();
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Pass_labirinth_pass_labirinth);
            break;
        }
    }
    return status_;
}
/* SM::pass_labirinth::pass_labirinth::guided_turn */
QState Pass_labirinth_guided_turn(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth::guided_turn */
        case Q_ENTRY_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth::guided_turn} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        /*. SM::pass_labirinth::pass_labirinth::guided_turn::TURN_DONE */
        case TURN_DONE_SIG: {
            status_ = Q_TRAN(&Pass_labirinth_go);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pass_labirinth_pass_labirinth);
            break;
        }
    }
    return status_;
}
/* SM::pass_labirinth::pass_labirinth::guided_turn::fast_turn */
QState Pass_labirinth_fast_turn(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth::guided_turn::fast_turn */
        case Q_ENTRY_SIG: {
            robot.turn_faster();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth::guided_turn::fast_turn} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        /*. SM::pass_labirinth::pass_labirinth::guided_turn::fast_turn::HALF_OPEN_SPACE */
        case HALF_OPEN_SPACE_SIG: {
            status_ = Q_TRAN(&Pass_labirinth_slow_turn);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pass_labirinth_guided_turn);
            break;
        }
    }
    return status_;
}
/* SM::pass_labirinth::pass_labirinth::guided_turn::slow_turn */
QState Pass_labirinth_slow_turn(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth::guided_turn::slow_turn */
        case Q_ENTRY_SIG: {
            robot.turn_slower();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth::guided_turn::slow_turn} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        /*. SM::pass_labirinth::pass_labirinth::guided_turn::slow_turn::CLOSED_SPACE */
        case CLOSED_SPACE_SIG: {
            status_ = Q_TRAN(&Pass_labirinth_fast_turn);
            break;
        }
        /*. SM::pass_labirinth::pass_labirinth::guided_turn::slow_turn::OPEN_SPACE */
        case OPEN_SPACE_SIG: {
            status_ = Q_TRAN(&Pass_labirinth_stop_turn);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pass_labirinth_guided_turn);
            break;
        }
    }
    return status_;
}
/* SM::pass_labirinth::pass_labirinth::guided_turn::stop_turn */
QState Pass_labirinth_stop_turn(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth::guided_turn::stop_turn */
        case Q_ENTRY_SIG: {
            robot.stop_turn();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth::guided_turn::stop_turn} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Pass_labirinth_guided_turn);
            break;
        }
    }
    return status_;
}
/* SM::pass_labirinth::pass_labirinth::start_turn */
QState Pass_labirinth_start_turn(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth::start_turn */
        case Q_ENTRY_SIG: {
            me->turn++;
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth::start_turn} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        /*. SM::pass_labirinth::pass_labirinth::start_turn::TIME_TICK */
        case TIME_TICK_SIG: {
            status_ = Q_TRAN(&Pass_labirinth_fast_turn);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pass_labirinth_pass_labirinth);
            break;
        }
    }
    return status_;
}
/* SM::pass_labirinth::pass_labirinth::start_turn::left */
QState Pass_labirinth_left(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth::start_turn::left */
        case Q_ENTRY_SIG: {
            robot.turn_left();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth::start_turn::left} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Pass_labirinth_start_turn);
            break;
        }
    }
    return status_;
}
/* SM::pass_labirinth::pass_labirinth::start_turn::right */
QState Pass_labirinth_right(Pass_labirinth * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /* SM::pass_labirinth::pass_labirinth::start_turn::right */
        case Q_ENTRY_SIG: {
            robot.turn_right();
            status_ = Q_HANDLED();
            break;
        }
        /*.${SM::pass_labirinth::pass_labirinth::start_turn::right} */
        case Q_EXIT_SIG: {
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&Pass_labirinth_start_turn);
            break;
        }
    }
    return status_;
}
