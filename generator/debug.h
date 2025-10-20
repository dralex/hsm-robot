// -----------------------------------------------------------------------------
// Debug function
// -----------------------------------------------------------------------------
 
#ifndef DEBUG_h
#define DEBUG_h

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __DEBUG__
#include <stdio.h>
#define DEBUG(...) printf( __VA_ARGS__)
#else
#define DEBUG(...)
#endif

#ifdef __cplusplus
}
#endif
#endif
