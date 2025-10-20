// -----------------------------------------------------------------------------
// Assertion function
// -----------------------------------------------------------------------------
 
#ifndef ASSERT_EXIT_h
#define ASSERT_EXIT_h

#ifdef __cplusplus
extern "C" {
#endif

#define SM_ASSERT(q, msg)   if (!(q)) {								   \
	                            assert_exit(__FILE__, __LINE__, (msg)) \
                            }

	void assert_exit(const char* file, int line, const char* message);

#ifdef __cplusplus
}
#endif
#endif
