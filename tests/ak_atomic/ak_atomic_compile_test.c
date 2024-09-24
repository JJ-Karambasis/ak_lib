#define AK_ATOMIC_EXCLUDE_STDINT
#include "ak_atomic_test_header.h"

int main(void) {
	return 0;
}

int mainCRTStartup(void) {
	return main();
}

#ifdef AK_ATOMIC_COMPILER_MSVC
#pragma warning(pop)
#endif

#ifndef AK_ATOMIC_IMPLEMENTATION
#define AK_ATOMIC_IMPLEMENTATION
#include <ak_atomic.h>
#endif
