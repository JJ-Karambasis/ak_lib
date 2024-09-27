#ifndef AK_ATOMIC_TEST_HEADER_H
#define AK_ATOMIC_TEST_HEADER_H

#ifdef AK_ATOMIC_EXCLUDE_STDINT
#define _INT8_T
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
#else
#include <stdint.h>
#endif

void* Allocate_Memory(uint32_t Size);
void Free_Memory(void* Memory);
static void Memory_Clear(void* Dst, uint32_t Size);
#define AK_ATOMIC_MALLOC(size) Allocate_Memory(size)
#define AK_ATOMIC_FREE(memory) Free_Memory(memory)
#define AK_ATOMIC_MEMORY_CLEAR(memory, size) Memory_Clear(memory, size)
void Internal_Assert(void);
#define Assert(c) if (!(c)) Internal_Assert()
#define AK_ATOMIC_ASSERT(c) Assert(c)

#include <ak_atomic.h>

#if defined(__clang__)
#define crash() __builtin_trap()
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4053 4711 5031 5045)

#ifndef crash
#define crash() *((int*)0) = 0
#endif

#endif

void Internal_Assert(void) {
	crash();
}

#if !defined(AK_ATOMIC_OS_WIN32)
#include <stdlib.h>
#endif

void* Allocate_Memory(uint32_t Size) {
#if defined(AK_ATOMIC_OS_WIN32)
	return HeapAlloc(GetProcessHeap(), 0, Size);
#else
	return malloc(Size);
#endif
}

void Free_Memory(void* Memory) {
#if defined(AK_ATOMIC_OS_WIN32)
	HeapFree(GetProcessHeap(), 0, Memory);
#else
	free(Memory);
#endif
}

static void Memory_Clear(void* Dst, uint32_t Size) {
#if defined(AK_ATOMIC_OS_WIN32) && !defined(AK_ATOMIC_DEBUG_BUILD)
	SecureZeroMemory(Dst, Size);
#else
	uint8_t* DstAt = (uint8_t *)Dst;
	while (Size--) {
		*DstAt++ = 0;
	}
#endif
}

#endif