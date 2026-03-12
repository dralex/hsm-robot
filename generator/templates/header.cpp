// -----------------------------------------------------------------------------
// Model: /*$$STATE_MACHINE_NAME$$*/
// File:  /*$$STATE_MACHINE_NAME$$*/.cpp
// -----------------------------------------------------------------------------
#include "qhsm.h"
#include "/*$$STATE_MACHINE_NAME$$*/.h"
#include <stdint.h>

/*$$GLOBAL_INIT$$*/

/* global-scope definitions -----------------------------------------*/
// the only instance of the /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/ class 
static /*$$STATE_MACHINE_CAPITALIZED_NAME$$*/ /*$$STATE_MACHINE_NAME$$*/;
QHsm * const the_/*$$STATE_MACHINE_NAME$$*/ = (QHsm *) &/*$$STATE_MACHINE_NAME$$*/; // the opaque pointer

