#define AK_ATOMIC_EXCLUDE_STDINT
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

void* Allocate_Memory(uint32_t Size);
void Free_Memory(void* Memory);
void Memory_Set(void* Dst, int Value, uint32_t Size);
#define AK_ATOMIC_MALLOC(size) Allocate_Memory(size)
#define AK_ATOMIC_FREE(memory) Free_Memory(memory)
#define AK_ATOMIC_MEMSET(dst, value, size) Memory_Set(dst, value, size)


#if defined(__clang__)
#define terminate() __builtin_trap()
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4053 4711)
	
#ifndef terminate
#define terminate() *((int*)0) = 0
#endif

#endif

#define Assert(c) if(!(c)) terminate()
#define AK_ATOMIC_ASSERT(c) Assert(c)

#include <ak_atomic.h>

void* Allocate_Memory(uint32_t Size) {
	return HeapAlloc(GetProcessHeap(), 0, Size);
}

void Free_Memory(void* Memory) {
	HeapFree(GetProcessHeap(), 0, Memory);
}

void Memory_Set(void* Dst, int Value, uint32_t Size) {
	uint8_t* DstAt = (uint8_t *)Dst;
	while (Size--) {
		*DstAt++ = (uint8_t)Value;
	}
}

#define Assert(c) if(!(c)) terminate()
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
