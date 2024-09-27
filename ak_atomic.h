#ifndef AK_ATOMIC_H
#define AK_ATOMIC_H

/*Platform detection macros*/
#if defined(_MSC_VER)
	#define AK_ATOMIC_OS_WIN32
	#define AK_ATOMIC_COMPILER_MSVC
	
	#if defined(_M_X64)
		#define AK_ATOMIC_PTR_SIZE 8
	#elif defined(_M_IX86)
		#define AK_ATOMIC_PTR_SIZE 4
	#else
		#error "Unrecognized Platform!"
	#endif
#elif defined(__GNUC__)
	#define AK_ATOMIC_COMPILER_GCC
	#define AK_ATOMIC_OS_POSIX

	#if defined(__APPLE__)
		#define AK_ATOMIC_OS_OSX
	#endif

	#if defined(__arm__)
		#define AK_ATOMIC_CPU_ARM
		#define AK_ATOMIC_PTR_SIZE 4
	#elif defined(__aarch64__)
		#define AK_ATOMIC_CPU_AARCH64
		#define AK_ATOMIC_PTR_SIZE 8
	#elif defined(__x86_64__)
		#define AK_ATOMIC_CPU_X86
		#define AK_ATOMIC_PTR_SIZE 8   
	#elif defined(__i386__)
		#define AK_ATOMIC_CPU_X86
		#define AK_ATOMIC_PTR_SIZE 4    
	#else
		#error "Not Implemented"
	#endif
#else
	#error "Unrecognized Platform!"
#endif

#if defined(__clang__)
	#define AK_ATOMIC_COMPILER_CLANG
#endif

/*If we are using c11 or c++11 (or greater) we wrap the atomic api's to get
  compiler intrinsics which should (in theory) have the best performance*/

#if __STDC_VERSION__ > 199901L && !defined(AK_ATOMIC_NO_C11_STD)
	#define AK_ATOMIC_C11
#endif

#if __cplusplus > 199711L && !defined(AK_ATOMIC_NO_CPP11_STD)
	#define AK_ATOMIC_CPP11
#endif

#ifdef AK_ATOMIC_C11
#include <stdatomic.h>
#endif

#ifdef AK_ATOMIC_CPP11
#include <atomic>
#endif

#ifndef AK_ATOMIC_EXCLUDE_STDINT
/*In Ansi MSVC, stdint.h includes comments that have single line comments which breaks ansi c standard.
  Need to disable this warning for MSVC only
*/
#ifdef AK_ATOMIC_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4001)
#endif

#include <stdint.h>

#ifdef AK_ATOMIC_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif

#define AK_ATOMIC__STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]
#define AK_ATOMIC__COMPILE_TIME_ASSERT3(X,L) AK_ATOMIC__STATIC_ASSERT(X,static_assertion_at_line_##L)
#define AK_ATOMIC__COMPILE_TIME_ASSERT2(X,L) AK_ATOMIC__COMPILE_TIME_ASSERT3(X,L)
#define AK_ATOMIC__COMPILE_TIME_ASSERT(X)    AK_ATOMIC__COMPILE_TIME_ASSERT2(X,__LINE__)

/*Verify datatypes are correct byte sizes*/
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(uint8_t)  == 1);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(uint16_t) == 2);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(uint32_t) == 4);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(uint64_t) == 8);

AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(int8_t)  == 1);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(int16_t) == 2);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(int32_t) == 4);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(int64_t) == 8);

#ifdef __cplusplus
#define AK_ATOMIC_EXTERN_C extern "C"
#else
#define AK_ATOMIC_EXTERN_C extern
#endif

#ifndef AKATOMICDEF
#ifdef AK_ATOMIC_STATIC
#define AKATOMICDEF static
#else
#define AKATOMICDEF AK_ATOMIC_EXTERN_C
#endif
#endif

#if defined(AK_ATOMIC_C11)
typedef struct {
	atomic_uchar Internal;
} ak_atomic_u8;

typedef struct {
	atomic_ushort Internal;
} ak_atomic_u16;

typedef struct {
	atomic_uint Internal;
} ak_atomic_u32;

typedef struct {
	atomic_ullong Internal;
} ak_atomic_u64;

typedef struct {
	atomic_size_t Internal;
} ak_atomic_ptr;

#define AK_Atomic_Fence_Acquire() atomic_thread_fence(memory_order_acquire)
#define AK_Atomic_Fence_Release() atomic_thread_fence(memory_order_release)
#define AK_Atomic_Fence_Seq_Cst() atomic_thread_fence(memory_order_seq_cst)

#elif defined(AK_ATOMIC_CPP11)
typedef struct {
	std::atomic_uchar Internal;
} ak_atomic_u8;

typedef struct {
	std::atomic_ushort Internal;
} ak_atomic_u16;

typedef struct {
	std::atomic_uint Internal;
} ak_atomic_u32;

typedef struct {
	std::atomic_ullong Internal;
} ak_atomic_u64;

typedef struct {
	std::atomic_size_t Internal;
} ak_atomic_ptr;

#define AK_Atomic_Fence_Acquire() atomic_thread_fence(std::memory_order_acquire)
#define AK_Atomic_Fence_Release() atomic_thread_fence(std::memory_order_release)
#define AK_Atomic_Fence_Seq_Cst() atomic_thread_fence(std::memory_order_seq_cst)

#elif defined(AK_ATOMIC_COMPILER_MSVC)

#elif defined(AK_ATOMIC_COMPILER_GCC) && (defined(AK_ATOMIC_CPU_AARCH64) || defined(AK_ATOMIC_CPU_ARM))

typedef struct {
	uint8_t Nonatomic;
} ak_atomic_u8;

typedef struct {
	uint16_t Nonatomic;
} __attribute__((aligned(2))) ak_atomic_u16;

typedef struct {
	uint32_t Nonatomic;
} __attribute__((aligned(4))) ak_atomic_u32;

typedef struct {
	uint64_t Nonatomic;
} __attribute__((aligned(8))) ak_atomic_u64;

typedef struct {
	void* Nonatomic;
} __attribute__((aligned(AK_ATOMIC_PTR_SIZE))) ak_atomic_ptr;

#if AK_ATOMIC_ARM_VERSION >= 7
#define AK_Atomic_Fence_Acquire() __asm__ volatile("dmb ish" ::: "memory")
#define AK_Atomic_Fence_Release() __asm__ volatile("dmb ish" ::: "memory")
#define AK_Atomic_Fence_Seq_Cst() __asm__ volatile("dmb ish" ::: "memory")
#else
#define AK_Atomic_Fence_Acquire() __asm__ volatile("mcr p15, 0, %0, c7, c10, 5" :: "r"(0) : "memory")
#define AK_Atomic_Fence_Release() __asm__ volatile("mcr p15, 0, %0, c7, c10, 5" :: "r"(0) : "memory")
#define AK_Atomic_Fence_Seq_Cst() __asm__ volatile("mcr p15, 0, %0, c7, c10, 5" :: "r"(0) : "memory")
#endif

#elif defined(AK_ATOMIC_COMPILER_GCC) && defined(AK_ATOMIC_CPU_X86)

typedef struct {
	volatile uint8_t Nonatomic;
} ak_atomic_u8;

typedef struct {
	volatile uint16_t Nonatomic;
} __attribute__((aligned(2))) ak_atomic_u16;

typedef struct {
	volatile uint32_t Nonatomic;
} __attribute__((aligned(4))) ak_atomic_u32;

typedef struct {
	volatile uint64_t Nonatomic;
} __attribute__((aligned(8))) ak_atomic_u64;

typedef struct {
	volatile void* Nonatomic;
} __attribute__((aligned(AK_ATOMIC_PTR_SIZE))) ak_atomic_ptr;

#define AK_Atomic_Fence_Acquire() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Fence_Release() __asm__ volatile("" ::: "memory")

#if AK_ATOMIC_PTR_SIZE == 8
#define AK_Atomic_Fence_Seq_Cst() __asm__ volatile("lock; orl $0, (%%rsp)" ::: "memory")
#else
#define AK_Atomic_Fence_Seq_Cst() __asm__ volatile("lock; orl $0, (%%esp)" ::: "memory")
#endif

#else
#error "Not Implemented!"
#endif

AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u8)  == 1);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u16) == 2);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u32) == 4);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u64) == 8);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_ptr) == AK_ATOMIC_PTR_SIZE);

typedef enum {
    AK_ATOMIC_MEMORY_ORDER_RELAXED,
    AK_ATOMIC_MEMORY_ORDER_ACQUIRE,
    AK_ATOMIC_MEMORY_ORDER_RELEASE,
    AK_ATOMIC_MEMORY_ORDER_ACQ_REL,
	AK_ATOMIC_MEMORY_ORDER_SEQ_CST
} ak_atomic_memory_order;

/*Atomic functions with memory order parameters*/
AKATOMICDEF uint8_t AK_Atomic_Load_U8(const ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void    AK_Atomic_Store_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Exchange_U8(ak_atomic_u8* Object, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Compare_Exchange_Strong_U8(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Compare_Exchange_Weak_U8(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Fetch_Add_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Fetch_Sub_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Fetch_And_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Fetch_Or_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Fetch_XOr_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Increment_U8(ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Decrement_U8(ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint16_t AK_Atomic_Load_U16(const ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Exchange_U16(ak_atomic_u16* Object, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Compare_Exchange_Strong_U16(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Compare_Exchange_Weak_U16(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Fetch_Add_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Fetch_Sub_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Fetch_And_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Fetch_Or_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Fetch_XOr_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Increment_U16(ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Decrement_U16(ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint32_t AK_Atomic_Load_U32(const ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Exchange_U32(ak_atomic_u32* Object, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_Strong_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_Weak_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Sub_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Fetch_And_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Or_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Fetch_XOr_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Increment_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Decrement_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint64_t AK_Atomic_Load_U64(const ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Exchange_U64(ak_atomic_u64* Object, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_Strong_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_Weak_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Fetch_Sub_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Fetch_And_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Fetch_Or_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Fetch_XOr_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Increment_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t AK_Atomic_Decrement_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);

typedef struct ak_thread ak_thread;
#define AK_THREAD_CALLBACK_DEFINE(name) int32_t name(ak_thread* Thread, void* UserData)
typedef AK_THREAD_CALLBACK_DEFINE(ak_thread_callback_func);

struct ak_thread {
	ak_thread_callback_func* Callback;
	void* 					 UserData;
};

#if defined(AK_ATOMIC_OS_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif

#define NOIME
#define NOWINRES
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOUSER
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOIME
#define NOPROXYSTUB
#define NOIMAGE
#define NO
#define NOTAPE
#define ANSI_ONLY

#pragma warning(push, 0)
#include <Windows.h>
#pragma warning(pop)

typedef struct {
	ak_thread Base;
	HANDLE    Handle;
	DWORD 	  ID;
	DWORD     Padding;
} ak_win32_thread;

typedef struct {
	CRITICAL_SECTION CriticalSection;
} ak_mutex;

typedef struct {
	HANDLE Handle;
} ak_semaphore;

typedef struct {
	CONDITION_VARIABLE ConditionVariable;
} ak_condition_variable;

typedef struct {
	DWORD Index;
} ak_tls;

#elif defined(AK_ATOMIC_OS_POSIX)

#include <pthread.h>

typedef struct {
	ak_thread Base;
	pthread_t Thread;
} ak_posix_thread;

typedef struct {
	pthread_mutex_t Mutex;
} ak_mutex;

#if defined(AK_ATOMIC_OS_OSX)
#include <mach/mach.h>

typedef struct {
	semaphore_t Semaphore;
} ak_semaphore;
#else
#error "Not Implemented!"
#endif

typedef struct {
	pthread_cond_t ConditionVariable;
} ak_condition_variable;

typedef struct {
	pthread_key_t Key;
} ak_tls;

#else
#error "Not Implemented!"
#endif

/*OS Primitives*/

/*Threads*/
AKATOMICDEF ak_thread* AK_Thread_Create(ak_thread_callback_func* Callback, void* UserData);
AKATOMICDEF void AK_Thread_Delete(ak_thread* Thread);
AKATOMICDEF void AK_Thread_Wait(ak_thread* Thread);
AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread);
AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void);
AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void);

/*Mutexes*/
AKATOMICDEF int8_t AK_Mutex_Create(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex);
AKATOMICDEF int8_t AK_Mutex_Try_Lock(ak_mutex* Mutex);

/*Semaphores*/
AKATOMICDEF int8_t AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount);
AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore);
AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore);
AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore);
AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Increment);

/*Condition variables*/
AKATOMICDEF int8_t AK_Condition_Variable_Create(ak_condition_variable* ConditionVariable);
AKATOMICDEF void AK_Condition_Variable_Delete(ak_condition_variable* ConditionVariable);
AKATOMICDEF void AK_Condition_Variable_Wait(ak_condition_variable* ConditionVariable, ak_mutex* Mutex);
AKATOMICDEF void AK_Condition_Variable_Wake_One(ak_condition_variable* ConditionVariable);
AKATOMICDEF void AK_Condition_Variable_Wake_All(ak_condition_variable* ConditionVariable);

/*Thread local storage*/
AKATOMICDEF int8_t AK_TLS_Create(ak_tls* TLS);
AKATOMICDEF void AK_TLS_Delete(ak_tls* TLS);
AKATOMICDEF void* AK_TLS_Get(ak_tls* TLS);
AKATOMICDEF void AK_TLS_Set(ak_tls* TLS, void* Data);

/*High resolution performance counters & timers*/
AKATOMICDEF void AK_Sleep(uint32_t Milliseconds);
AKATOMICDEF uint64_t AK_Query_Performance_Counter(void);
AKATOMICDEF uint64_t AK_Query_Performance_Frequency(void);

#endif

#ifdef AK_ATOMIC_IMPLEMENTATION

/*Compiler warnings*/
#ifdef AK_ATOMIC_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4061 4062 4711)
#endif

#ifdef AK_ATOMIC_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
#endif

/*Custom allocators*/
#if !defined(AK_ATOMIC_MALLOC) || !defined(AK_ATOMIC_FREE)
#include <stdlib.h>
#define AK_ATOMIC_MALLOC(size) malloc(size)
#define AK_ATOMIC_FREE(memory) free(memory)
#endif

#if !defined(AK_ATOMIC_MEMORY_CLEAR)
#include <string.h>
#define AK_ATOMIC_MEMORY_CLEAR(dst, size) memset(dst, 0, size)
#endif

#if !defined(AK_ATOMIC_ASSERT)
#include <assert.h>
#define AK_ATOMIC_ASSERT(c) assert(c)
#endif

/*Special macros for true and false for c89 since booleans don't really exist without stdbool 
 (which some compilers might not have)*/
#define ak_atomic_true 1
#define ak_atomic_false 0

#define AK_ATOMIC__UNREFERENCED_PARAMETER(param) (void)(param)

/*CPU and compiler specific architecture (all other atomics are built ontop of these)*/
#if defined(AK_ATOMIC_C11)

#define AK_Atomic_Load_U8_Relaxed(object) atomic_load_explicit(&(object)->Internal, memory_order_relaxed)
#define AK_Atomic_Load_U8_Acquire(object) atomic_load_explicit(&(object)->Internal, memory_order_acquire)
#define AK_Atomic_Load_U8_Seq_Cst(object) atomic_load_explicit(&(object)->Internal, memory_order_seq_cst)

#define AK_Atomic_Store_U8_Relaxed(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_relaxed)
#define AK_Atomic_Store_U8_Release(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_release)
#define AK_Atomic_Store_U8_Seq_Cst(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_seq_cst)

#define AK_Atomic_Exchange_U8_Relaxed(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_relaxed)
#define AK_Atomic_Exchange_U8_Acquire(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_acquire)
#define AK_Atomic_Exchange_U8_Release(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_release)
#define AK_Atomic_Exchange_U8_Acq_Rel(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_acq_rel)
#define AK_Atomic_Exchange_U8_Seq_Cst(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Add_U8_Relaxed(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Add_U8_Acquire(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Add_U8_Release(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Add_U8_Acq_Rel(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Add_U8_Seq_Cst(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Sub_U8_Relaxed(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Sub_U8_Acquire(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Sub_U8_Release(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Sub_U8_Acq_Rel(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Sub_U8_Seq_Cst(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_And_U8_Relaxed(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_And_U8_Acquire(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_And_U8_Release(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_And_U8_Acq_Rel(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_And_U8_Seq_Cst(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Or_U8_Relaxed(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Or_U8_Acquire(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Or_U8_Release(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Or_U8_Acq_Rel(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Or_U8_Seq_Cst(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_XOr_U8_Relaxed(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_XOr_U8_Acquire(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_XOr_U8_Release(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_XOr_U8_Acq_Rel(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_XOr_U8_Seq_Cst(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Increment_U8_Relaxed(object) AK_Atomic_Fetch_Add_U8_Relaxed(object, 1) + 1
#define AK_Atomic_Increment_U8_Release(object) AK_Atomic_Fetch_Add_U8_Release(object, 1) + 1
#define AK_Atomic_Increment_U8_Seq_Cst(object) AK_Atomic_Fetch_Add_U8_Seq_Cst(object, 1) + 1

#define AK_Atomic_Decrement_U8_Relaxed(object) AK_Atomic_Fetch_Sub_U8_Relaxed(object, 1) - 1
#define AK_Atomic_Decrement_U8_Release(object) AK_Atomic_Fetch_Sub_U8_Release(object, 1) - 1
#define AK_Atomic_Decrement_U8_Seq_Cst(object) AK_Atomic_Fetch_Sub_U8_Seq_Cst(object, 1) - 1

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Relaxed(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_relaxed, memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Acquire(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acquire, memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Release(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_release, memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acq_rel, memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_seq_cst, memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Relaxed(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_relaxed, memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Acquire(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acquire, memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Release(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_release, memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acq_rel, memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_seq_cst, memory_order_relaxed);
	return OldValue;
}

#define AK_Atomic_Load_U16_Relaxed(object) atomic_load_explicit(&(object)->Internal, memory_order_relaxed)
#define AK_Atomic_Load_U16_Acquire(object) atomic_load_explicit(&(object)->Internal, memory_order_acquire)
#define AK_Atomic_Load_U16_Seq_Cst(object) atomic_load_explicit(&(object)->Internal, memory_order_seq_cst)

#define AK_Atomic_Store_U16_Relaxed(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_relaxed)
#define AK_Atomic_Store_U16_Release(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_release)
#define AK_Atomic_Store_U16_Seq_Cst(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_seq_cst)

#define AK_Atomic_Exchange_U16_Relaxed(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_relaxed)
#define AK_Atomic_Exchange_U16_Acquire(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_acquire)
#define AK_Atomic_Exchange_U16_Release(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_release)
#define AK_Atomic_Exchange_U16_Acq_Rel(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_acq_rel)
#define AK_Atomic_Exchange_U16_Seq_Cst(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Add_U16_Relaxed(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Add_U16_Acquire(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Add_U16_Release(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Add_U16_Acq_Rel(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Add_U16_Seq_Cst(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Sub_U16_Relaxed(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Sub_U16_Acquire(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Sub_U16_Release(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Sub_U16_Acq_Rel(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Sub_U16_Seq_Cst(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_And_U16_Relaxed(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_And_U16_Acquire(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_And_U16_Release(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_And_U16_Acq_Rel(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_And_U16_Seq_Cst(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Or_U16_Relaxed(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Or_U16_Acquire(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Or_U16_Release(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Or_U16_Acq_Rel(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Or_U16_Seq_Cst(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_XOr_U16_Relaxed(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_XOr_U16_Acquire(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_XOr_U16_Release(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_XOr_U16_Acq_Rel(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_XOr_U16_Seq_Cst(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Increment_U16_Relaxed(object) AK_Atomic_Fetch_Add_U16_Relaxed(object, 1) + 1
#define AK_Atomic_Increment_U16_Release(object) AK_Atomic_Fetch_Add_U16_Release(object, 1) + 1
#define AK_Atomic_Increment_U16_Seq_Cst(object) AK_Atomic_Fetch_Add_U16_Seq_Cst(object, 1) + 1

#define AK_Atomic_Decrement_U16_Relaxed(object) AK_Atomic_Fetch_Sub_U16_Relaxed(object, 1) - 1
#define AK_Atomic_Decrement_U16_Release(object) AK_Atomic_Fetch_Sub_U16_Release(object, 1) - 1
#define AK_Atomic_Decrement_U16_Seq_Cst(object) AK_Atomic_Fetch_Sub_U16_Seq_Cst(object, 1) - 1

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Relaxed(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_relaxed, memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Acquire(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acquire, memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Release(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_release, memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acq_rel, memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_seq_cst, memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Relaxed(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_relaxed, memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Acquire(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acquire, memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Release(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_release, memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acq_rel, memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_seq_cst, memory_order_relaxed);
	return OldValue;
}

#define AK_Atomic_Load_U32_Relaxed(object) atomic_load_explicit(&(object)->Internal, memory_order_relaxed)
#define AK_Atomic_Load_U32_Acquire(object) atomic_load_explicit(&(object)->Internal, memory_order_acquire)
#define AK_Atomic_Load_U32_Seq_Cst(object) atomic_load_explicit(&(object)->Internal, memory_order_seq_cst)

#define AK_Atomic_Store_U32_Relaxed(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_relaxed)
#define AK_Atomic_Store_U32_Release(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_release)
#define AK_Atomic_Store_U32_Seq_Cst(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_seq_cst)

#define AK_Atomic_Exchange_U32_Relaxed(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_relaxed)
#define AK_Atomic_Exchange_U32_Acquire(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_acquire)
#define AK_Atomic_Exchange_U32_Release(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_release)
#define AK_Atomic_Exchange_U32_Acq_Rel(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_acq_rel)
#define AK_Atomic_Exchange_U32_Seq_Cst(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Add_U32_Relaxed(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Add_U32_Acquire(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Add_U32_Release(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Add_U32_Acq_Rel(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Add_U32_Seq_Cst(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Sub_U32_Relaxed(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Sub_U32_Acquire(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Sub_U32_Release(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Sub_U32_Acq_Rel(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Sub_U32_Seq_Cst(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_And_U32_Relaxed(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_And_U32_Acquire(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_And_U32_Release(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_And_U32_Acq_Rel(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_And_U32_Seq_Cst(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Or_U32_Relaxed(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Or_U32_Acquire(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Or_U32_Release(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Or_U32_Acq_Rel(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Or_U32_Seq_Cst(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_XOr_U32_Relaxed(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_XOr_U32_Acquire(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_XOr_U32_Release(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_XOr_U32_Acq_Rel(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_XOr_U32_Seq_Cst(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Increment_U32_Relaxed(object) AK_Atomic_Fetch_Add_U32_Relaxed(object, 1) + 1
#define AK_Atomic_Increment_U32_Release(object) AK_Atomic_Fetch_Add_U32_Release(object, 1) + 1
#define AK_Atomic_Increment_U32_Seq_Cst(object) AK_Atomic_Fetch_Add_U32_Seq_Cst(object, 1) + 1

#define AK_Atomic_Decrement_U32_Relaxed(object) AK_Atomic_Fetch_Sub_U32_Relaxed(object, 1) - 1
#define AK_Atomic_Decrement_U32_Release(object) AK_Atomic_Fetch_Sub_U32_Release(object, 1) - 1
#define AK_Atomic_Decrement_U32_Seq_Cst(object) AK_Atomic_Fetch_Sub_U32_Seq_Cst(object, 1) - 1

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_relaxed, memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Acquire(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acquire, memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Release(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_release, memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acq_rel, memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_seq_cst, memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_relaxed, memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Acquire(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acquire, memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Release(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_release, memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acq_rel, memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_seq_cst, memory_order_relaxed);
	return OldValue;
}

#define AK_Atomic_Load_U64_Relaxed(object) atomic_load_explicit(&(object)->Internal, memory_order_relaxed)
#define AK_Atomic_Load_U64_Acquire(object) atomic_load_explicit(&(object)->Internal, memory_order_acquire)
#define AK_Atomic_Load_U64_Seq_Cst(object) atomic_load_explicit(&(object)->Internal, memory_order_seq_cst)

#define AK_Atomic_Store_U64_Relaxed(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_relaxed)
#define AK_Atomic_Store_U64_Release(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_release)
#define AK_Atomic_Store_U64_Seq_Cst(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, memory_order_seq_cst)

#define AK_Atomic_Exchange_U64_Relaxed(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_relaxed)
#define AK_Atomic_Exchange_U64_Acquire(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_acquire)
#define AK_Atomic_Exchange_U64_Release(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_release)
#define AK_Atomic_Exchange_U64_Acq_Rel(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_acq_rel)
#define AK_Atomic_Exchange_U64_Seq_Cst(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Add_U64_Relaxed(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Add_U64_Acquire(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Add_U64_Release(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Add_U64_Acq_Rel(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Add_U64_Seq_Cst(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Sub_U64_Relaxed(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Sub_U64_Acquire(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Sub_U64_Release(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Sub_U64_Acq_Rel(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Sub_U64_Seq_Cst(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_And_U64_Relaxed(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_And_U64_Acquire(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_And_U64_Release(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_And_U64_Acq_Rel(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_And_U64_Seq_Cst(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_Or_U64_Relaxed(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_Or_U64_Acquire(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_Or_U64_Release(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_Or_U64_Acq_Rel(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_Or_U64_Seq_Cst(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Fetch_XOr_U64_Relaxed(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_relaxed)
#define AK_Atomic_Fetch_XOr_U64_Acquire(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_acquire)
#define AK_Atomic_Fetch_XOr_U64_Release(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_release)
#define AK_Atomic_Fetch_XOr_U64_Acq_Rel(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_acq_rel)
#define AK_Atomic_Fetch_XOr_U64_Seq_Cst(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, memory_order_seq_cst)

#define AK_Atomic_Increment_U64_Relaxed(object) AK_Atomic_Fetch_Add_U64_Relaxed(object, 1) + 1
#define AK_Atomic_Increment_U64_Release(object) AK_Atomic_Fetch_Add_U64_Release(object, 1) + 1
#define AK_Atomic_Increment_U64_Seq_Cst(object) AK_Atomic_Fetch_Add_U64_Seq_Cst(object, 1) + 1

#define AK_Atomic_Decrement_U64_Relaxed(object) AK_Atomic_Fetch_Sub_U64_Relaxed(object, 1) - 1
#define AK_Atomic_Decrement_U64_Release(object) AK_Atomic_Fetch_Sub_U64_Release(object, 1) - 1
#define AK_Atomic_Decrement_U64_Seq_Cst(object) AK_Atomic_Fetch_Sub_U64_Seq_Cst(object, 1) - 1

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_relaxed, memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Acquire(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acquire, memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Release(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_release, memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acq_rel, memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, memory_order_seq_cst, memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_relaxed, memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Acquire(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acquire, memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Release(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_release, memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_acq_rel, memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, memory_order_seq_cst, memory_order_relaxed);
	return OldValue;
}

#elif defined(AK_ATOMIC_CPP11)

#define AK_Atomic_Load_U8_Relaxed(object) atomic_load_explicit(&(object)->Internal, std::memory_order_relaxed)
#define AK_Atomic_Load_U8_Acquire(object) atomic_load_explicit(&(object)->Internal, std::memory_order_acquire)
#define AK_Atomic_Load_U8_Seq_Cst(object) atomic_load_explicit(&(object)->Internal, std::memory_order_seq_cst)

#define AK_Atomic_Store_U8_Relaxed(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_relaxed)
#define AK_Atomic_Store_U8_Release(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_release)
#define AK_Atomic_Store_U8_Seq_Cst(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_seq_cst)

#define AK_Atomic_Exchange_U8_Relaxed(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_relaxed)
#define AK_Atomic_Exchange_U8_Acquire(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_acquire)
#define AK_Atomic_Exchange_U8_Release(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_release)
#define AK_Atomic_Exchange_U8_Acq_Rel(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_acq_rel)
#define AK_Atomic_Exchange_U8_Seq_Cst(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Add_U8_Relaxed(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Add_U8_Acquire(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Add_U8_Release(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Add_U8_Acq_Rel(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Add_U8_Seq_Cst(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Sub_U8_Relaxed(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Sub_U8_Acquire(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Sub_U8_Release(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Sub_U8_Acq_Rel(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Sub_U8_Seq_Cst(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_And_U8_Relaxed(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_And_U8_Acquire(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_And_U8_Release(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_And_U8_Acq_Rel(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_And_U8_Seq_Cst(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Or_U8_Relaxed(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Or_U8_Acquire(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Or_U8_Release(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Or_U8_Acq_Rel(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Or_U8_Seq_Cst(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_XOr_U8_Relaxed(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_XOr_U8_Acquire(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_XOr_U8_Release(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_XOr_U8_Acq_Rel(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_XOr_U8_Seq_Cst(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Increment_U8_Relaxed(object) AK_Atomic_Fetch_Add_U8_Relaxed(object, 1) + 1
#define AK_Atomic_Increment_U8_Release(object) AK_Atomic_Fetch_Add_U8_Release(object, 1) + 1
#define AK_Atomic_Increment_U8_Seq_Cst(object) AK_Atomic_Fetch_Add_U8_Seq_Cst(object, 1) + 1

#define AK_Atomic_Decrement_U8_Relaxed(object) AK_Atomic_Fetch_Sub_U8_Relaxed(object, 1) - 1
#define AK_Atomic_Decrement_U8_Release(object) AK_Atomic_Fetch_Sub_U8_Release(object, 1) - 1
#define AK_Atomic_Decrement_U8_Seq_Cst(object) AK_Atomic_Fetch_Sub_U8_Seq_Cst(object, 1) - 1

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Relaxed(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_relaxed, std::memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Acquire(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acquire, std::memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Release(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_release, std::memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acq_rel, std::memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_seq_cst, std::memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Relaxed(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_relaxed, std::memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Acquire(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acquire, std::memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Release(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_release, std::memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acq_rel, std::memory_order_relaxed);
	return OldValue;
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_seq_cst, std::memory_order_relaxed);
	return OldValue;
}

#define AK_Atomic_Load_U16_Relaxed(object) atomic_load_explicit(&(object)->Internal, std::memory_order_relaxed)
#define AK_Atomic_Load_U16_Acquire(object) atomic_load_explicit(&(object)->Internal, std::memory_order_acquire)
#define AK_Atomic_Load_U16_Seq_Cst(object) atomic_load_explicit(&(object)->Internal, std::memory_order_seq_cst)

#define AK_Atomic_Store_U16_Relaxed(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_relaxed)
#define AK_Atomic_Store_U16_Release(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_release)
#define AK_Atomic_Store_U16_Seq_Cst(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_seq_cst)

#define AK_Atomic_Exchange_U16_Relaxed(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_relaxed)
#define AK_Atomic_Exchange_U16_Acquire(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_acquire)
#define AK_Atomic_Exchange_U16_Release(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_release)
#define AK_Atomic_Exchange_U16_Acq_Rel(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_acq_rel)
#define AK_Atomic_Exchange_U16_Seq_Cst(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Add_U16_Relaxed(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Add_U16_Acquire(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Add_U16_Release(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Add_U16_Acq_Rel(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Add_U16_Seq_Cst(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Sub_U16_Relaxed(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Sub_U16_Acquire(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Sub_U16_Release(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Sub_U16_Acq_Rel(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Sub_U16_Seq_Cst(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_And_U16_Relaxed(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_And_U16_Acquire(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_And_U16_Release(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_And_U16_Acq_Rel(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_And_U16_Seq_Cst(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Or_U16_Relaxed(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Or_U16_Acquire(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Or_U16_Release(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Or_U16_Acq_Rel(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Or_U16_Seq_Cst(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_XOr_U16_Relaxed(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_XOr_U16_Acquire(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_XOr_U16_Release(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_XOr_U16_Acq_Rel(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_XOr_U16_Seq_Cst(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Increment_U16_Relaxed(object) AK_Atomic_Fetch_Add_U16_Relaxed(object, 1) + 1
#define AK_Atomic_Increment_U16_Release(object) AK_Atomic_Fetch_Add_U16_Release(object, 1) + 1
#define AK_Atomic_Increment_U16_Seq_Cst(object) AK_Atomic_Fetch_Add_U16_Seq_Cst(object, 1) + 1

#define AK_Atomic_Decrement_U16_Relaxed(object) AK_Atomic_Fetch_Sub_U16_Relaxed(object, 1) - 1
#define AK_Atomic_Decrement_U16_Release(object) AK_Atomic_Fetch_Sub_U16_Release(object, 1) - 1
#define AK_Atomic_Decrement_U16_Seq_Cst(object) AK_Atomic_Fetch_Sub_U16_Seq_Cst(object, 1) - 1

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Relaxed(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_relaxed, std::memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Acquire(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acquire, std::memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Release(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_release, std::memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acq_rel, std::memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_seq_cst, std::memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Relaxed(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_relaxed, std::memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Acquire(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acquire, std::memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Release(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_release, std::memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acq_rel, std::memory_order_relaxed);
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_seq_cst, std::memory_order_relaxed);
	return OldValue;
}

#define AK_Atomic_Load_U32_Relaxed(object) atomic_load_explicit(&(object)->Internal, std::memory_order_relaxed)
#define AK_Atomic_Load_U32_Acquire(object) atomic_load_explicit(&(object)->Internal, std::memory_order_acquire)
#define AK_Atomic_Load_U32_Seq_Cst(object) atomic_load_explicit(&(object)->Internal, std::memory_order_seq_cst)

#define AK_Atomic_Store_U32_Relaxed(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_relaxed)
#define AK_Atomic_Store_U32_Release(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_release)
#define AK_Atomic_Store_U32_Seq_Cst(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_seq_cst)

#define AK_Atomic_Exchange_U32_Relaxed(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_relaxed)
#define AK_Atomic_Exchange_U32_Acquire(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_acquire)
#define AK_Atomic_Exchange_U32_Release(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_release)
#define AK_Atomic_Exchange_U32_Acq_Rel(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_acq_rel)
#define AK_Atomic_Exchange_U32_Seq_Cst(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Add_U32_Relaxed(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Add_U32_Acquire(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Add_U32_Release(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Add_U32_Acq_Rel(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Add_U32_Seq_Cst(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Sub_U32_Relaxed(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Sub_U32_Acquire(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Sub_U32_Release(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Sub_U32_Acq_Rel(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Sub_U32_Seq_Cst(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_And_U32_Relaxed(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_And_U32_Acquire(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_And_U32_Release(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_And_U32_Acq_Rel(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_And_U32_Seq_Cst(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Or_U32_Relaxed(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Or_U32_Acquire(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Or_U32_Release(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Or_U32_Acq_Rel(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Or_U32_Seq_Cst(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_XOr_U32_Relaxed(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_XOr_U32_Acquire(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_XOr_U32_Release(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_XOr_U32_Acq_Rel(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_XOr_U32_Seq_Cst(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Increment_U32_Relaxed(object) AK_Atomic_Fetch_Add_U32_Relaxed(object, 1) + 1
#define AK_Atomic_Increment_U32_Release(object) AK_Atomic_Fetch_Add_U32_Release(object, 1) + 1
#define AK_Atomic_Increment_U32_Seq_Cst(object) AK_Atomic_Fetch_Add_U32_Seq_Cst(object, 1) + 1

#define AK_Atomic_Decrement_U32_Relaxed(object) AK_Atomic_Fetch_Sub_U32_Relaxed(object, 1) - 1
#define AK_Atomic_Decrement_U32_Release(object) AK_Atomic_Fetch_Sub_U32_Release(object, 1) - 1
#define AK_Atomic_Decrement_U32_Seq_Cst(object) AK_Atomic_Fetch_Sub_U32_Seq_Cst(object, 1) - 1

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_relaxed, std::memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Acquire(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acquire, std::memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Release(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_release, std::memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acq_rel, std::memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_seq_cst, std::memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_relaxed, std::memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Acquire(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acquire, std::memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Release(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_release, std::memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acq_rel, std::memory_order_relaxed);
	return OldValue;
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_seq_cst, std::memory_order_relaxed);
	return OldValue;
}

#define AK_Atomic_Load_U64_Relaxed(object) atomic_load_explicit(&(object)->Internal, std::memory_order_relaxed)
#define AK_Atomic_Load_U64_Acquire(object) atomic_load_explicit(&(object)->Internal, std::memory_order_acquire)
#define AK_Atomic_Load_U64_Seq_Cst(object) atomic_load_explicit(&(object)->Internal, std::memory_order_seq_cst)

#define AK_Atomic_Store_U64_Relaxed(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_relaxed)
#define AK_Atomic_Store_U64_Release(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_release)
#define AK_Atomic_Store_U64_Seq_Cst(object, new_value) atomic_store_explicit(&(object)->Internal, new_value, std::memory_order_seq_cst)

#define AK_Atomic_Exchange_U64_Relaxed(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_relaxed)
#define AK_Atomic_Exchange_U64_Acquire(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_acquire)
#define AK_Atomic_Exchange_U64_Release(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_release)
#define AK_Atomic_Exchange_U64_Acq_Rel(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_acq_rel)
#define AK_Atomic_Exchange_U64_Seq_Cst(object, new_value) atomic_exchange_explicit(&(object)->Internal, new_value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Add_U64_Relaxed(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Add_U64_Acquire(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Add_U64_Release(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Add_U64_Acq_Rel(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Add_U64_Seq_Cst(object, value) atomic_fetch_add_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Sub_U64_Relaxed(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Sub_U64_Acquire(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Sub_U64_Release(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Sub_U64_Acq_Rel(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Sub_U64_Seq_Cst(object, value) atomic_fetch_sub_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_And_U64_Relaxed(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_And_U64_Acquire(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_And_U64_Release(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_And_U64_Acq_Rel(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_And_U64_Seq_Cst(object, value) atomic_fetch_and_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_Or_U64_Relaxed(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_Or_U64_Acquire(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_Or_U64_Release(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_Or_U64_Acq_Rel(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_Or_U64_Seq_Cst(object, value) atomic_fetch_or_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Fetch_XOr_U64_Relaxed(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_relaxed)
#define AK_Atomic_Fetch_XOr_U64_Acquire(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_acquire)
#define AK_Atomic_Fetch_XOr_U64_Release(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_release)
#define AK_Atomic_Fetch_XOr_U64_Acq_Rel(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_acq_rel)
#define AK_Atomic_Fetch_XOr_U64_Seq_Cst(object, value) atomic_fetch_xor_explicit(&(object)->Internal, value, std::memory_order_seq_cst)

#define AK_Atomic_Increment_U64_Relaxed(object) AK_Atomic_Fetch_Add_U64_Relaxed(object, 1) + 1
#define AK_Atomic_Increment_U64_Release(object) AK_Atomic_Fetch_Add_U64_Release(object, 1) + 1
#define AK_Atomic_Increment_U64_Seq_Cst(object) AK_Atomic_Fetch_Add_U64_Seq_Cst(object, 1) + 1

#define AK_Atomic_Decrement_U64_Relaxed(object) AK_Atomic_Fetch_Sub_U64_Relaxed(object, 1) - 1
#define AK_Atomic_Decrement_U64_Release(object) AK_Atomic_Fetch_Sub_U64_Release(object, 1) - 1
#define AK_Atomic_Decrement_U64_Seq_Cst(object) AK_Atomic_Fetch_Sub_U64_Seq_Cst(object, 1) - 1

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_relaxed, std::memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Acquire(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acquire, std::memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Release(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_release, std::memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acq_rel, std::memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_strong_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_seq_cst, std::memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_relaxed, std::memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Acquire(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acquire, std::memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Release(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_release, std::memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_acq_rel, std::memory_order_relaxed);
	return OldValue;
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	atomic_compare_exchange_weak_explicit(&Object->Internal, &OldValue, NewValue, std::memory_order_seq_cst, std::memory_order_relaxed);
	return OldValue;
}

#elif defined(AK_ATOMIC_COMPILER_MSVC)

#error "Not Implemented!"

#elif defined(AK_ATOMIC_COMPILER_GCC) && defined(AK_ATOMIC_CPU_AARCH64)
AK_ATOMIC__COMPILE_TIME_ASSERT(AK_ATOMIC_PTR_SIZE == 8);

static uint8_t AK_Atomic_Load_U8_Relaxed(const ak_atomic_u8* Object) {
	return Object->Nonatomic;
}

static uint8_t AK_Atomic_Load_U8_Acquire(const ak_atomic_u8* Object) {
	uint8_t Result = 0;
#if __ARM_FEATURE_RCPC
	/*Use ldapr if its available*/
	__asm__ volatile(
		"ldaprb %w0, [%1]" 
        : "=r"(Result)
        : "r"(Object));
#else
	__asm__ volatile(
		"ldarb %w0, [%1]" 
        : "=r"(Result)
        : "r"(Object));
#endif
	return Result;
}

static uint8_t AK_Atomic_Load_U8_Seq_Cst(const ak_atomic_u8* Object) {
	uint8_t Result;
	__asm__ volatile(
		"ldarb %w0, [%1]" 
        : "=r"(Result)
        : "r"(Object));
	return Result;
}

static void AK_Atomic_Store_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value) {
	Object->Nonatomic = Value;
}

static void AK_Atomic_Store_U8_Release(ak_atomic_u8* Object, uint8_t Value) {
	__asm__ volatile(
		"stlrb %w1, %0" 
		: "+Q" (Object->Nonatomic)
		: "r" (Value)
		: "cc");
}

static void AK_Atomic_Store_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t Value) {
	AK_Atomic_Store_U8_Release(Object, Value);
}

static uint8_t AK_Atomic_Exchange_U8_Relaxed(ak_atomic_u8* Object, uint8_t NewValue) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldxrb %w0, %2\n"
		"	stxrb %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint8_t AK_Atomic_Exchange_U8_Acquire(ak_atomic_u8* Object, uint8_t NewValue) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpab %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldaxrb %w0, %2\n"
		"	stxrb %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint8_t AK_Atomic_Exchange_U8_Release(ak_atomic_u8* Object, uint8_t NewValue) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swplb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldxrb %w0, %2\n"
		"	stlxrb %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint8_t AK_Atomic_Exchange_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t NewValue) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpalb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldaxrb %w0, %2\n"
		"	stlxrb %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint8_t AK_Atomic_Exchange_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t NewValue) {
	return AK_Atomic_Exchange_U8_Acq_Rel(Object, NewValue);
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Relaxed(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casb %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint8_t Result, Status;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stxrb %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Acquire(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casab %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint8_t Result, Status;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stxrb %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Release(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"caslb %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint8_t Result, Status;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stlxrb %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casalb %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint8_t Result, Status;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stlxrb %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint8_t AK_Atomic_Compare_Exchange_Strong_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	return AK_Atomic_Compare_Exchange_Strong_U8_Acq_Rel(Object, OldValue, NewValue);
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Relaxed(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U8_Relaxed(Object, OldValue, NewValue);
#else
	uint8_t Result;
	__asm__ volatile(
		"	ldxrb %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stxrb wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Acquire(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U8_Acquire(Object, OldValue, NewValue);
#else
	uint8_t Result;
	__asm__ volatile(
		"	ldaxrb %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stxrb wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Release(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U8_Release(Object, OldValue, NewValue);
#else
	uint8_t Result;
	__asm__ volatile(
		"	ldxrb %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stlxrb wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U8_Acq_Rel(Object, OldValue, NewValue);
#else
	uint8_t Result;
	__asm__ volatile(
		"	ldaxrb %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stlxrb wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint8_t AK_Atomic_Compare_Exchange_Weak_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	return AK_Atomic_Compare_Exchange_Weak_U8_Acq_Rel(Object, OldValue, NewValue);
}

static uint8_t AK_Atomic_Fetch_Add_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_Add_U8_Acquire(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddab %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_Add_U8_Release(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddlb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stlxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_Add_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddalb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stlxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_Add_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t Value) {
	return AK_Atomic_Fetch_Add_U8_Acq_Rel(Object, Value);
}

static uint8_t AK_Atomic_Fetch_Sub_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value) {
	return AK_Atomic_Fetch_Add_U8_Relaxed(Object, -Value);
}

static uint8_t AK_Atomic_Fetch_Sub_U8_Acquire(ak_atomic_u8* Object, uint8_t Value) {
	return AK_Atomic_Fetch_Add_U8_Acquire(Object, -Value);
}

static uint8_t AK_Atomic_Fetch_Sub_U8_Release(ak_atomic_u8* Object, uint8_t Value) {
	return AK_Atomic_Fetch_Add_U8_Release(Object, -Value);
}

static uint8_t AK_Atomic_Fetch_Sub_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t Value) {
	return AK_Atomic_Fetch_Add_U8_Acq_Rel(Object, -Value);
}

static uint8_t AK_Atomic_Fetch_Sub_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t Value) {
	return AK_Atomic_Fetch_Add_U8_Seq_Cst(Object, -Value);
}

static uint8_t AK_Atomic_Fetch_And_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclrb w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_And_U8_Acquire(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclrab %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_And_U8_Release(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclrlb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stlxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_And_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclralb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stlxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_And_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t Value) {
	return AK_Atomic_Fetch_And_U8_Acq_Rel(Object, Value);
}

static uint8_t AK_Atomic_Fetch_Or_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_Or_U8_Acquire(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetab %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_Or_U8_Release(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetlb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stlxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_Or_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetalb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stlxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_Or_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t Value) {
	return AK_Atomic_Fetch_Or_U8_Acq_Rel(Object, Value);
}

static uint8_t AK_Atomic_Fetch_XOr_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeorb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_XOr_U8_Acquire(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeorab %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_XOr_U8_Release(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeorlb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stlxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_XOr_U8_Acq_Rel(ak_atomic_u8* Object, uint8_t Value) {
	uint8_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeoralb %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint8_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrb %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stlxrb %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint8_t AK_Atomic_Fetch_XOr_U8_Seq_Cst(ak_atomic_u8* Object, uint8_t Value) {
	return AK_Atomic_Fetch_XOr_U8_Acq_Rel(Object, Value);
}

static uint8_t AK_Atomic_Increment_U8_Relaxed(ak_atomic_u8* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddb %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (1)
		: "cc");
#else
	uint8_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxrb %w1, %0\n"
		"	add %w2, %w1, 1\n"
		"	stxrb %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint8_t AK_Atomic_Increment_U8_Release(ak_atomic_u8* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddlb %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (1)
		: "cc");
#else
	uint8_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxrb %w1, %0\n"
		"	add %w2, %w1, 1\n"
		"	stlxrb %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint8_t AK_Atomic_Increment_U8_Seq_Cst(ak_atomic_u8* Object) {
	return AK_Atomic_Increment_U8_Release(Object);
}

static uint8_t AK_Atomic_Decrement_U8_Relaxed(ak_atomic_u8* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddb %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (-1)
		: "cc");
#else
	uint8_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxrb %w1, %0\n"
		"	add %w2, %w1, -1\n"
		"	stxrb %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint8_t AK_Atomic_Decrement_U8_Release(ak_atomic_u8* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddlb %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (-1)
		: "cc");
#else
	uint8_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxrb %w1, %0\n"
		"	add %w2, %w1, -1\n"
		"	stlxrb %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint8_t AK_Atomic_Decrement_U8_Seq_Cst(ak_atomic_u8* Object) {
	return AK_Atomic_Decrement_U8_Release(Object);
}

static uint16_t AK_Atomic_Load_U16_Relaxed(const ak_atomic_u16* Object) {
	return Object->Nonatomic;
}

static uint16_t AK_Atomic_Load_U16_Acquire(const ak_atomic_u16* Object) {
	uint16_t Result;
#if __ARM_FEATURE_RCPC
	/*Use ldapr if its available*/
	__asm__ volatile(
		"ldaprh %w0, [%1]" 
        : "=r"(Result)
        : "r"(Object));
#else
	__asm__ volatile(
		"ldarh %w0, [%1]" 
        : "=&r"(Result)
        : "r"(Object));
#endif
	return Result;
}

static uint16_t AK_Atomic_Load_U16_Seq_Cst(const ak_atomic_u16* Object) {
	uint16_t Result;
	__asm__ volatile(
		"ldarh %w0, [%1]" 
        : "=&r"(Result)
        : "r"(Object));
	return Result;
}

static void AK_Atomic_Store_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value) {
	Object->Nonatomic = Value;
}

static void AK_Atomic_Store_U16_Release(ak_atomic_u16* Object, uint16_t Value) {
	__asm__ volatile(
		"stlrh %w1, %0" 
		: "+Q" (Object->Nonatomic)
		: "r" (Value)
		: "cc");
}

static void AK_Atomic_Store_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t Value) {
	AK_Atomic_Store_U16_Release(Object, Value);
}

static uint16_t AK_Atomic_Exchange_U16_Relaxed(ak_atomic_u16* Object, uint16_t NewValue) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swph %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldxrh %w0, %2\n"
		"	stxrh %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint16_t AK_Atomic_Exchange_U16_Acquire(ak_atomic_u16* Object, uint16_t NewValue) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpah %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldaxrh %w0, %2\n"
		"	stxrh %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint16_t AK_Atomic_Exchange_U16_Release(ak_atomic_u16* Object, uint16_t NewValue) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swplh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldxrh %w0, %2\n"
		"	stlxrh %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint16_t AK_Atomic_Exchange_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t NewValue) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpalh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldaxrh %w0, %2\n"
		"	stlxrh %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint16_t AK_Atomic_Exchange_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t NewValue) {
	return AK_Atomic_Exchange_U16_Acq_Rel(Object, NewValue);
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Relaxed(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"cash %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint16_t Result, Status;
	__asm__ volatile(
		"1:	ldxrh %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stxrh %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Acquire(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casah %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint16_t Result, Status;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stxrh %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Release(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"caslh %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint16_t Result, Status;
	__asm__ volatile(
		"1:	ldxrb %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stlxrb %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}
static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casalh %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
#else
	uint16_t Result, Status;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stlxrh %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
	return OldValue;
}

static uint16_t AK_Atomic_Compare_Exchange_Strong_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	return AK_Atomic_Compare_Exchange_Strong_U16_Acq_Rel(Object, OldValue, NewValue);
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Relaxed(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U16_Relaxed(Object, OldValue, NewValue);
#else
	uint16_t Result;
	__asm__ volatile(
		"	ldxrh %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stxrh wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Acquire(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U16_Acquire(Object, OldValue, NewValue);
#else
	uint16_t Result;
	__asm__ volatile(
		"	ldaxrh %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stxrh wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Release(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U16_Release(Object, OldValue, NewValue);
#else
	uint16_t Result;
	__asm__ volatile(
		"	ldxrh %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stlxrh wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U16_Acq_Rel(Object, OldValue, NewValue);
#else
	uint16_t Result;
	__asm__ volatile(
		"	ldaxrh %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stlxrh wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint16_t AK_Atomic_Compare_Exchange_Weak_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	return AK_Atomic_Compare_Exchange_Weak_U16_Acq_Rel(Object, OldValue, NewValue);
}

static uint16_t AK_Atomic_Fetch_Add_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrh %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_Add_U16_Acquire(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddah %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_Add_U16_Release(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddlh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrh %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stlxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_Add_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddalh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stlxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_Add_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t Value) {
	return AK_Atomic_Fetch_Add_U16_Acq_Rel(Object, Value);
}

static uint16_t AK_Atomic_Fetch_Sub_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value) {
	return AK_Atomic_Fetch_Add_U16_Relaxed(Object, -Value);
}

static uint16_t AK_Atomic_Fetch_Sub_U16_Acquire(ak_atomic_u16* Object, uint16_t Value) {
	return AK_Atomic_Fetch_Add_U16_Acquire(Object, -Value);
}

static uint16_t AK_Atomic_Fetch_Sub_U16_Release(ak_atomic_u16* Object, uint16_t Value) {
	return AK_Atomic_Fetch_Add_U16_Release(Object, -Value);
}

static uint16_t AK_Atomic_Fetch_Sub_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t Value) {
	return AK_Atomic_Fetch_Add_U16_Acq_Rel(Object, -Value);
}

static uint16_t AK_Atomic_Fetch_Sub_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t Value) {
	return AK_Atomic_Fetch_Add_U16_Seq_Cst(Object, -Value);
}

static uint16_t AK_Atomic_Fetch_And_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclrh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrh %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_And_U16_Acquire(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclrah %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_And_U16_Release(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclrlh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrh %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stlxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_And_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclralh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stlxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_And_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t Value) {
	return AK_Atomic_Fetch_And_U16_Acq_Rel(Object, Value);
}

static uint16_t AK_Atomic_Fetch_Or_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldseth %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrh %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_Or_U16_Acquire(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetah %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_Or_U16_Release(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetlh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrh %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stlxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_Or_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetalh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stlxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_Or_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t Value) {
	return AK_Atomic_Fetch_Or_U16_Acq_Rel(Object, Value);
}

static uint16_t AK_Atomic_Fetch_XOr_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeorh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrh %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_XOr_U16_Acquire(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeorah %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_XOr_U16_Release(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeorlh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxrh %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stlxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_XOr_U16_Acq_Rel(ak_atomic_u16* Object, uint16_t Value) {
	uint16_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeoralh %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint16_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxrh %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stlxrh %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint16_t AK_Atomic_Fetch_XOr_U16_Seq_Cst(ak_atomic_u16* Object, uint16_t Value) {
	return AK_Atomic_Fetch_XOr_U16_Acq_Rel(Object, Value);
}

static uint16_t AK_Atomic_Increment_U16_Relaxed(ak_atomic_u16* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddh %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (1)
		: "cc");
#else
	uint16_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxrh %w1, %0\n"
		"	add %w2, %w1, 1\n"
		"	stxrh %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint16_t AK_Atomic_Increment_U16_Release(ak_atomic_u16* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddlh %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (1)
		: "cc");
#else
	uint16_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxrh %w1, %0\n"
		"	add %w2, %w1, 1\n"
		"	stlxrh %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint16_t AK_Atomic_Increment_U16_Seq_Cst(ak_atomic_u16* Object) {
	return AK_Atomic_Increment_U16_Release(Object);
}

static uint16_t AK_Atomic_Decrement_U16_Relaxed(ak_atomic_u16* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddh %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (-1)
		: "cc");
#else
	uint16_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxrh %w1, %0\n"
		"	add %w2, %w1, -1\n"
		"	stxrh %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint16_t AK_Atomic_Decrement_U16_Release(ak_atomic_u16* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddlh %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (-1)
		: "cc");
#else
	uint16_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxrh %w1, %0\n"
		"	add %w2, %w1, -1\n"
		"	stlxrh %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint16_t AK_Atomic_Decrement_U16_Seq_Cst(ak_atomic_u16* Object) {
	return AK_Atomic_Decrement_U16_Release(Object);
}

static uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
	return Object->Nonatomic;
}

static uint32_t AK_Atomic_Load_U32_Acquire(const ak_atomic_u32* Object) {
	uint32_t Result;
#if __ARM_FEATURE_RCPC
	/*Use ldapr if its available*/
	__asm__ volatile(
		"ldapr %w0, [%1]" 
        : "=r"(Result)
        : "r"(Object));
#else
	__asm__ volatile(
		"ldar %w0, [%1]" 
        : "=&r"(Result)
        : "r"(Object));
#endif
	return Result;
}

static uint32_t AK_Atomic_Load_U32_Seq_Cst(const ak_atomic_u32* Object) {
	uint32_t Result;
	__asm__ volatile(
		"ldar %w0, [%1]" 
        : "=&r"(Result)
        : "r"(Object));
	return Result;
}

static void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
	Object->Nonatomic = Value;
}

static void AK_Atomic_Store_U32_Release(ak_atomic_u32* Object, uint32_t Value) {
	__asm__ volatile(
		"stlr %w1, %0" 
		: "+Q" (Object->Nonatomic)
		: "r" (Value)
		: "cc");
}

static void AK_Atomic_Store_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t Value) {
	AK_Atomic_Store_U32_Release(Object, Value);
}

static uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swp %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldxr %w0, %2\n"
		"	stxr %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint32_t AK_Atomic_Exchange_U32_Acquire(ak_atomic_u32* Object, uint32_t NewValue) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpa %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldaxr %w0, %2\n"
		"	stxr %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint32_t AK_Atomic_Exchange_U32_Release(ak_atomic_u32* Object, uint32_t NewValue) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpl %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldxr %w0, %2\n"
		"	stlxr %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint32_t AK_Atomic_Exchange_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t NewValue) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpal %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldaxr %w0, %2\n"
		"	stlxr %w1, %w3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint32_t AK_Atomic_Exchange_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t NewValue) {
	return AK_Atomic_Exchange_U32_Acq_Rel(Object, NewValue);
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"cas %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint32_t Result, Status;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stxr %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Acquire(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casa %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint32_t Result, Status;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stxr %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Release(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casl %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint32_t Result, Status;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stlxr %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casal %w1, %w2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint32_t Result, Status;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	cmp %w1, %w3\n"
		"	bne 2f\n"
		"	stlxr %w2, %w4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint32_t AK_Atomic_Compare_Exchange_Strong_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	return AK_Atomic_Compare_Exchange_Strong_U32_Acq_Rel(Object, OldValue, NewValue);
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U32_Relaxed(Object, OldValue, NewValue);
#else
	uint32_t Result;
	__asm__ volatile(
		"	ldxr %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stxr wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Acquire(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U32_Acquire(Object, OldValue, NewValue);
#else
	uint32_t Result;
	__asm__ volatile(
		"	ldaxr %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stxr wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Release(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U32_Release(Object, OldValue, NewValue);
#else
	uint32_t Result;
	__asm__ volatile(
		"	ldxr %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stlxr wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U32_Acq_Rel(Object, OldValue, NewValue);
#else
	uint32_t Result;
	__asm__ volatile(
		"	ldaxr %w1, %0\n"
		"	cmp %w1, %w2\n"
		"	bne 1f\n"
		"	stlxr wzr, %w3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint32_t AK_Atomic_Compare_Exchange_Weak_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	return AK_Atomic_Compare_Exchange_Weak_U32_Acq_Rel(Object, OldValue, NewValue);
}

static uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldadd %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_Add_U32_Acquire(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldadda %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_Add_U32_Release(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddl %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stlxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_Add_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddal %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	add %w3, %w1, %w4\n"
		"	stlxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_Add_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t Value) {
	return AK_Atomic_Fetch_Add_U32_Acq_Rel(Object, Value);
}

static uint32_t AK_Atomic_Fetch_Sub_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
	return AK_Atomic_Fetch_Add_U32_Relaxed(Object, -Value);
}

static uint32_t AK_Atomic_Fetch_Sub_U32_Acquire(ak_atomic_u32* Object, uint32_t Value) {
	return AK_Atomic_Fetch_Add_U32_Acquire(Object, -Value);
}

static uint32_t AK_Atomic_Fetch_Sub_U32_Release(ak_atomic_u32* Object, uint32_t Value) {
	return AK_Atomic_Fetch_Add_U32_Release(Object, -Value);
}

static uint32_t AK_Atomic_Fetch_Sub_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t Value) {
	return AK_Atomic_Fetch_Add_U32_Acq_Rel(Object, -Value);
}

static uint32_t AK_Atomic_Fetch_Sub_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t Value) {
	return AK_Atomic_Fetch_Add_U32_Seq_Cst(Object, -Value);
}

static uint32_t AK_Atomic_Fetch_And_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclr %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_And_U32_Acquire(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclra %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_And_U32_Release(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclrl %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stlxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_And_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclral %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	and %w3, %w1, %w4\n"
		"	stlxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_And_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t Value) {
	return AK_Atomic_Fetch_And_U32_Acq_Rel(Object, Value);
}

static uint32_t AK_Atomic_Fetch_Or_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldset %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_Or_U32_Acquire(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldseta %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_Or_U32_Release(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetl %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stlxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_Or_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetal %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	orr %w3, %w1, %w4\n"
		"	stlxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_Or_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t Value) {
	return AK_Atomic_Fetch_Or_U32_Acq_Rel(Object, Value);
}

static uint32_t AK_Atomic_Fetch_XOr_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeor %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_XOr_U32_Acquire(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeora %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_XOr_U32_Release(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeorl %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldxr %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stlxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_XOr_U32_Acq_Rel(ak_atomic_u32* Object, uint32_t Value) {
	uint32_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeoral %w2, %w1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status, TempRegister;
	__asm__ volatile(
		"1:	ldaxr %w1, %0\n"
		"	eor %w3, %w1, %w4\n"
		"	stlxr %w2, %w3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint32_t AK_Atomic_Fetch_XOr_U32_Seq_Cst(ak_atomic_u32* Object, uint32_t Value) {
	return AK_Atomic_Fetch_XOr_U32_Acq_Rel(Object, Value);
}

static uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"stadd %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (1)
		: "cc");
#else
	uint32_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxr %w1, %0\n"
		"	add %w2, %w1, 1\n"
		"	stxr %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint32_t AK_Atomic_Increment_U32_Release(ak_atomic_u32* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddl %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (1)
		: "cc");
#else
	uint32_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxr %w1, %0\n"
		"	add %w2, %w1, 1\n"
		"	stlxr %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint32_t AK_Atomic_Increment_U32_Seq_Cst(ak_atomic_u32* Object) {
	return AK_Atomic_Increment_U32_Release(Object);
}

static uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"stadd %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (-1)
		: "cc");
#else
	uint32_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxr %w1, %0\n"
		"	add %w2, %w1, -1\n"
		"	stxr %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint32_t AK_Atomic_Decrement_U32_Release(ak_atomic_u32* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddl %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (-1)
		: "cc");
#else
	uint32_t OldValue, Status, TempRegister;
	__asm__ volatile(
		"1: ldxrb %w1, %0\n"
		"	add %w2, %w1, -1\n"
		"	stlxrb %w3, %w2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint32_t AK_Atomic_Decrement_U32_Seq_Cst(ak_atomic_u32* Object) {
	return AK_Atomic_Decrement_U32_Release(Object);
}

static uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
	return Object->Nonatomic;
}

static uint64_t AK_Atomic_Load_U64_Acquire(const ak_atomic_u64* Object) {
	uint64_t Result;
#if __ARM_FEATURE_RCPC
	/*Use ldapr if its available*/
	__asm__ volatile(
		"ldapr %0, [%1]" 
        : "=r"(Result)
        : "r"(Object));
#else
	__asm__ volatile(
		"ldar %0, [%1]" 
        : "=&r"(Result)
        : "r"(Object));
#endif
	return Result;
}

static uint64_t AK_Atomic_Load_U64_Seq_Cst(const ak_atomic_u64* Object) {
	uint64_t Result;
	__asm__ volatile(
		"ldar %0, [%1]" 
        : "=&r"(Result)
        : "r"(Object));
	return Result;
}

static void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
	Object->Nonatomic = Value;
}

static void AK_Atomic_Store_U64_Release(ak_atomic_u64* Object, uint64_t Value) {
	__asm__ volatile(
		"stlr %1, %0" 
		: "+Q" (Object->Nonatomic)
		: "r" (Value)
		: "cc");
}

static void AK_Atomic_Store_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t Value) {
	AK_Atomic_Store_U64_Release(Object, Value);
}

static uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swp %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldxr %0, %2\n"
		"	stxr %w1, %3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint64_t AK_Atomic_Exchange_U64_Acquire(ak_atomic_u64* Object, uint64_t NewValue) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpa %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldaxr %0, %2\n"
		"	stxr %w1, %3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint64_t AK_Atomic_Exchange_U64_Release(ak_atomic_u64* Object, uint64_t NewValue) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpl %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldxr %0, %2\n"
		"	stlxr %w1, %3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint64_t AK_Atomic_Exchange_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t NewValue) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"swpal %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=r" (Result)
		: "r" (NewValue)
		: "cc");
#else
	uint32_t Status;
	__asm__ volatile(
		"1: ldaxr %0, %2\n"
		"	stlxr %w1, %3, %2\n"
		"	cbnz %w1, 1b"
		: "=&r" (Result), "=&r" (Status), "+Q" (Object->Nonatomic)
		: "r" (NewValue)
		: "cc");
#endif
	return Result;
}

static uint64_t AK_Atomic_Exchange_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t NewValue) {
	return AK_Atomic_Exchange_U64_Acq_Rel(Object, NewValue);
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"cas %1, %2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint64_t Result;
	uint32_t Status;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	cmp %1, %3\n"
		"	bne 2f\n"
		"	stxr %w2, %4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Acquire(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casa %1, %2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint64_t Result;
	uint32_t Status;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	cmp %1, %3\n"
		"	bne 2f\n"
		"	stxr %w2, %4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Release(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casl %1, %2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint64_t Result;
	uint32_t Status;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	cmp %1, %3\n"
		"	bne 2f\n"
		"	stlxr %w2, %4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"casal %1, %2, %0"
		: "+Q" (Object->Nonatomic), "+r" (OldValue)
		: "r" (NewValue)
		: "cc");
	return OldValue;
#else
	uint64_t Result;
	uint32_t Status;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	cmp %1, %3\n"
		"	bne 2f\n"
		"	stlxr %w2, %4, %0\n"
		"	cbnz %w2, 1b\n"
		"	b 3f\n"
		"2:	clrex\n" /*Is this necessary?*/
		"3:"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint64_t AK_Atomic_Compare_Exchange_Strong_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	return AK_Atomic_Compare_Exchange_Strong_U64_Acq_Rel(Object, OldValue, NewValue);
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U64_Relaxed(Object, OldValue, NewValue);
#else
	uint64_t Result;
	__asm__ volatile(
		"	ldxr %1, %0\n"
		"	cmp %1, %2\n"
		"	bne 1f\n"
		"	stxr wzr, %3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Acquire(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U64_Acquire(Object, OldValue, NewValue);
#else
	uint64_t Result;
	__asm__ volatile(
		"	ldaxr %1, %0\n"
		"	cmp %1, %2\n"
		"	bne 1f\n"
		"	stxr wzr, %3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Release(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U64_Release(Object, OldValue, NewValue);
#else
	uint64_t Result;
	__asm__ volatile(
		"	ldxr %1, %0\n"
		"	cmp %1, %2\n"
		"	bne 1f\n"
		"	stlxr wzr, %3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
#if  __ARM_FEATURE_ATOMICS
	return AK_Atomic_Compare_Exchange_Strong_U64_Acq_Rel(Object, OldValue, NewValue);
#else
	uint64_t Result;
	__asm__ volatile(
		"	ldaxr %1, %0\n"
		"	cmp %1, %2\n"
		"	bne 1f\n"
		"	stlxr wzr, %3, %0\n"
		"	b 2f\n"
		"1:	clrex\n"  /*Is this necessary?*/
		"2:"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (OldValue), "r" (NewValue)
		: "cc"
	);
	return Result;
#endif
}

static uint64_t AK_Atomic_Compare_Exchange_Weak_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	return AK_Atomic_Compare_Exchange_Weak_U64_Acq_Rel(Object, OldValue, NewValue);
}

static uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldadd %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	add %3, %1, %4\n"
		"	stxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_Add_U64_Acquire(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldadda %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	add %3, %1, %4\n"
		"	stxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_Add_U64_Release(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddl %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	add %3, %1, %4\n"
		"	stlxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_Add_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldaddal %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	add %3, %1, %4\n"
		"	stlxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_Add_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t Value) {
	return AK_Atomic_Fetch_Add_U64_Acq_Rel(Object, Value);
}

static uint64_t AK_Atomic_Fetch_Sub_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
	return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -Value);
}

static uint64_t AK_Atomic_Fetch_Sub_U64_Acquire(ak_atomic_u64* Object, uint64_t Value) {
	return AK_Atomic_Fetch_Add_U64_Acquire(Object, -Value);
}

static uint64_t AK_Atomic_Fetch_Sub_U64_Release(ak_atomic_u64* Object, uint64_t Value) {
	return AK_Atomic_Fetch_Add_U64_Release(Object, -Value);
}

static uint64_t AK_Atomic_Fetch_Sub_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t Value) {
	return AK_Atomic_Fetch_Add_U64_Acq_Rel(Object, -Value);
}

static uint64_t AK_Atomic_Fetch_Sub_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t Value) {
	return AK_Atomic_Fetch_Add_U64_Seq_Cst(Object, -Value);
}

static uint64_t AK_Atomic_Fetch_And_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclr %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	and %3, %1, %4\n"
		"	stxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_And_U64_Acquire(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclra %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	and %3, %1, %4\n"
		"	stxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_And_U64_Release(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclrl %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	and %3, %1, %4\n"
		"	stlxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_And_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldclral %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (~Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	and %3, %1, %4\n"
		"	stlxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_And_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t Value) {
	return AK_Atomic_Fetch_And_U64_Acq_Rel(Object, Value);
}

static uint64_t AK_Atomic_Fetch_Or_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldset %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	orr %3, %1, %4\n"
		"	stxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_Or_U64_Acquire(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldseta %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	orr %3, %1, %4\n"
		"	stxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_Or_U64_Release(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetl %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	orr %3, %1, %4\n"
		"	stlxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_Or_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldsetal %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	orr %3, %1, %4\n"
		"	stlxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_Or_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t Value) {
	return AK_Atomic_Fetch_Or_U64_Acq_Rel(Object, Value);
}

static uint64_t AK_Atomic_Fetch_XOr_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeor %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	eor %3, %1, %4\n"
		"	stxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_XOr_U64_Acquire(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeora %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	eor %3, %1, %4\n"
		"	stxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_XOr_U64_Release(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeorl %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldxr %1, %0\n"
		"	eor %3, %1, %4\n"
		"	stlxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_XOr_U64_Acq_Rel(ak_atomic_u64* Object, uint64_t Value) {
	uint64_t Result;
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"ldeoral %2, %1, %0"
		: "+Q" (Object->Nonatomic), "=&r" (Result)
		: "r" (Value)
		: "cc");
#else
	uint32_t Status;
	uint64_t TempRegister;
	__asm__ volatile(
		"1:	ldaxr %1, %0\n"
		"	eor %3, %1, %4\n"
		"	stlxr %w2, %3, %0\n"
		"	cbnz %w2, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (Result), "=&r" (Status), "=&r" (TempRegister)
		: "r" (Value)
		: "cc"
	);
#endif
	return Result;
}

static uint64_t AK_Atomic_Fetch_XOr_U64_Seq_Cst(ak_atomic_u64* Object, uint64_t Value) {
	return AK_Atomic_Fetch_XOr_U64_Acq_Rel(Object, Value);
}

static uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"stadd %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (1)
		: "cc");
#else
	uint64_t OldValue, TempRegister;
	uint32_t Status;
	__asm__ volatile(
		"1: ldxr %1, %0\n"
		"	add %2, %1, 1\n"
		"	stxr %w3, %2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint64_t AK_Atomic_Increment_U64_Release(ak_atomic_u64* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddl %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (1)
		: "cc");
#else
	uint64_t OldValue, TempRegister;
	uint32_t Status;
	__asm__ volatile(
		"1: ldxr %1, %0\n"
		"	add %2, %1, 1\n"
		"	stlxr %w3, %2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint64_t AK_Atomic_Increment_U64_Seq_Cst(ak_atomic_u64* Object) {
	return AK_Atomic_Increment_U64_Release(Object);
}

static uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"stadd %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (-1)
		: "cc");
#else
	uint64_t OldValue, TempRegister;
	uint32_t Status;
	__asm__ volatile(
		"1: ldxr %1, %0\n"
		"	add %2, %1, -1\n"
		"	stxr %w3, %2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint64_t AK_Atomic_Decrement_U64_Release(ak_atomic_u64* Object) {
#if  __ARM_FEATURE_ATOMICS
	__asm__ volatile(
		"staddl %w1, %0"
		: "+Q" (Object->Nonatomic)
		: "r" (-1)
		: "cc");
#else
	uint64_t OldValue, TempRegister;
	uint32_t Status;
	__asm__ volatile(
		"1: ldxr %1, %0\n"
		"	add %2, %1, -1\n"
		"	stlxr %w3, %2, %0\n"
		"	cbnz %w3, 1b"
		: "+Q" (Object->Nonatomic), "=&r" (OldValue), "=&r" (TempRegister), "=&r" (Status)
		: 
		: "cc"
	);
#endif
	return Object->Nonatomic;
}

static uint64_t AK_Atomic_Decrement_U64_Seq_Cst(ak_atomic_u64* Object) {
	return AK_Atomic_Decrement_U64_Release(Object);
}

#elif defined(AK_ATOMIC_COMPILER_GCC) && defined(AK_ATOMIC_CPU_ARM)
AK_ATOMIC__COMPILE_TIME_ASSERT(AK_ATOMIC_PTR_SIZE == 4);


#elif defined(AK_ATOMIC_COMPILER_GCC) && defined(AK_ATOMIC_CPU_X86)

#error "Not Implemented!"

#else
#error "Not Implemented!"
#endif

/*Atomic functions with memory order parameters*/
AKATOMICDEF uint8_t AK_Atomic_Load_U8(const ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Load_U8_Acquire(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Load_U8_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Load_U8_Relaxed(Object);
		} break;
	}

	return Result;
}

AKATOMICDEF void AK_Atomic_Store_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder) {
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			AK_Atomic_Store_U8_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			AK_Atomic_Store_U8_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			AK_Atomic_Store_U8_Relaxed(Object, Value);
		} break;
	}
}

AKATOMICDEF uint8_t AK_Atomic_Exchange_U8(ak_atomic_u8* Object, uint8_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Exchange_U8_Acquire(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Exchange_U8_Release(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Exchange_U8_Acq_Rel(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Exchange_U8_Seq_Cst(Object, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Exchange_U8_Relaxed(Object, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Compare_Exchange_Strong_U8(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Compare_Exchange_Strong_U8_Acquire(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Compare_Exchange_Strong_U8_Release(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Compare_Exchange_Strong_U8_Acq_Rel(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Compare_Exchange_Strong_U8_Seq_Cst(Object, OldValue, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Compare_Exchange_Strong_U8_Relaxed(Object, OldValue, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Compare_Exchange_Weak_U8(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Compare_Exchange_Weak_U8_Acquire(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Compare_Exchange_Weak_U8_Release(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Compare_Exchange_Weak_U8_Acq_Rel(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Compare_Exchange_Weak_U8_Seq_Cst(Object, OldValue, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Compare_Exchange_Weak_U8_Relaxed(Object, OldValue, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Fetch_Add_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Add_U8_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Add_U8_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Add_U8_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Add_U8_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Add_U8_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Fetch_Sub_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Sub_U8_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Sub_U8_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Sub_U8_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Sub_U8_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Sub_U8_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Fetch_And_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_And_U8_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_And_U8_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_And_U8_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_And_U8_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_And_U8_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Fetch_Or_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Or_U8_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Or_U8_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Or_U8_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Or_U8_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Or_U8_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Fetch_XOr_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_XOr_U8_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_XOr_U8_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_XOr_U8_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_XOr_U8_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_XOr_U8_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Increment_U8(ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Increment_U8_Release(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Increment_U8_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Increment_U8_Relaxed(Object);
		} break;
	}
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Decrement_U8(ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder) {
	uint8_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Decrement_U8_Release(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Decrement_U8_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Decrement_U8_Relaxed(Object);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Load_U16(const ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Load_U16_Acquire(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Load_U16_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Load_U16_Relaxed(Object);
		} break;
	}

	return Result;
}

AKATOMICDEF void AK_Atomic_Store_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder) {
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			AK_Atomic_Store_U16_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			AK_Atomic_Store_U16_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			AK_Atomic_Store_U16_Relaxed(Object, Value);
		} break;
	}
}

AKATOMICDEF uint16_t AK_Atomic_Exchange_U16(ak_atomic_u16* Object, uint16_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Exchange_U16_Acquire(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Exchange_U16_Release(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Exchange_U16_Acq_Rel(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Exchange_U16_Seq_Cst(Object, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Exchange_U16_Relaxed(Object, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Compare_Exchange_Strong_U16(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Compare_Exchange_Strong_U16_Acquire(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Compare_Exchange_Strong_U16_Release(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Compare_Exchange_Strong_U16_Acq_Rel(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Compare_Exchange_Strong_U16_Seq_Cst(Object, OldValue, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Compare_Exchange_Strong_U16_Relaxed(Object, OldValue, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Compare_Exchange_Weak_U16(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Compare_Exchange_Weak_U16_Acquire(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Compare_Exchange_Weak_U16_Release(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Compare_Exchange_Weak_U16_Acq_Rel(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Compare_Exchange_Weak_U16_Seq_Cst(Object, OldValue, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Compare_Exchange_Weak_U16_Relaxed(Object, OldValue, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Fetch_Add_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Add_U16_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Add_U16_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Add_U16_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Add_U16_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Add_U16_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Fetch_Sub_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Sub_U16_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Sub_U16_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Sub_U16_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Sub_U16_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Sub_U16_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Fetch_And_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_And_U16_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_And_U16_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_And_U16_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_And_U16_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_And_U16_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Fetch_Or_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Or_U16_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Or_U16_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Or_U16_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Or_U16_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Or_U16_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Fetch_XOr_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_XOr_U16_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_XOr_U16_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_XOr_U16_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_XOr_U16_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_XOr_U16_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Increment_U16(ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Increment_U16_Release(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Increment_U16_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Increment_U16_Relaxed(Object);
		} break;
	}
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Decrement_U16(ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder) {
	uint16_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Decrement_U16_Release(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Decrement_U16_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Decrement_U16_Relaxed(Object);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Load_U32(const ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Load_U32_Acquire(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Load_U32_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Load_U32_Relaxed(Object);
		} break;
	}

	return Result;
}

AKATOMICDEF void AK_Atomic_Store_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder) {
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			AK_Atomic_Store_U32_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			AK_Atomic_Store_U32_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			AK_Atomic_Store_U32_Relaxed(Object, Value);
		} break;
	}
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32(ak_atomic_u32* Object, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Exchange_U32_Acquire(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Exchange_U32_Release(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Exchange_U32_Acq_Rel(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Exchange_U32_Seq_Cst(Object, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Exchange_U32_Relaxed(Object, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_Strong_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Compare_Exchange_Strong_U32_Acquire(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Compare_Exchange_Strong_U32_Release(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Compare_Exchange_Strong_U32_Acq_Rel(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Compare_Exchange_Strong_U32_Seq_Cst(Object, OldValue, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Compare_Exchange_Strong_U32_Relaxed(Object, OldValue, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_Weak_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Compare_Exchange_Weak_U32_Acquire(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Compare_Exchange_Weak_U32_Release(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Compare_Exchange_Weak_U32_Acq_Rel(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Compare_Exchange_Weak_U32_Seq_Cst(Object, OldValue, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Compare_Exchange_Weak_U32_Relaxed(Object, OldValue, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Add_U32_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Add_U32_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Add_U32_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Add_U32_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Add_U32_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Sub_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Sub_U32_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Sub_U32_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Sub_U32_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Sub_U32_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Sub_U32_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_And_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_And_U32_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_And_U32_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_And_U32_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_And_U32_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_And_U32_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Or_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Or_U32_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Or_U32_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Or_U32_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Or_U32_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Or_U32_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_XOr_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_XOr_U32_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_XOr_U32_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_XOr_U32_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_XOr_U32_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_XOr_U32_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Increment_U32_Release(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Increment_U32_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Increment_U32_Relaxed(Object);
		} break;
	}
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder) {
	uint32_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Decrement_U32_Release(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Decrement_U32_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Decrement_U32_Relaxed(Object);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64(const ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Load_U64_Acquire(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Load_U64_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Load_U64_Relaxed(Object);
		} break;
	}

	return Result;
}

AKATOMICDEF void AK_Atomic_Store_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder) {
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			AK_Atomic_Store_U64_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			AK_Atomic_Store_U64_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			AK_Atomic_Store_U64_Relaxed(Object, Value);
		} break;
	}
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64(ak_atomic_u64* Object, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Exchange_U64_Acquire(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Exchange_U64_Release(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Exchange_U64_Acq_Rel(Object, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Exchange_U64_Seq_Cst(Object, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Exchange_U64_Relaxed(Object, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_Strong_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Compare_Exchange_Strong_U64_Acquire(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Compare_Exchange_Strong_U64_Release(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Compare_Exchange_Strong_U64_Acq_Rel(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Compare_Exchange_Strong_U64_Seq_Cst(Object, OldValue, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Compare_Exchange_Strong_U64_Relaxed(Object, OldValue, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_Weak_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Compare_Exchange_Weak_U64_Acquire(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Compare_Exchange_Weak_U64_Release(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Compare_Exchange_Weak_U64_Acq_Rel(Object, OldValue, NewValue);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Compare_Exchange_Weak_U64_Seq_Cst(Object, OldValue, NewValue);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Compare_Exchange_Weak_U64_Relaxed(Object, OldValue, NewValue);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Add_U64_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Add_U64_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Add_U64_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Add_U64_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Add_U64_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Sub_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Sub_U64_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Sub_U64_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Sub_U64_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Sub_U64_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Sub_U64_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_And_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_And_U64_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_And_U64_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_And_U64_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_And_U64_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_And_U64_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Or_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_Or_U64_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_Or_U64_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_Or_U64_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_Or_U64_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_Or_U64_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_XOr_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE: {
			Result = AK_Atomic_Fetch_XOr_U64_Acquire(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Fetch_XOr_U64_Release(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: {
			Result = AK_Atomic_Fetch_XOr_U64_Acq_Rel(Object, Value);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Fetch_XOr_U64_Seq_Cst(Object, Value);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Fetch_XOr_U64_Relaxed(Object, Value);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Increment_U64_Release(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Increment_U64_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Increment_U64_Relaxed(Object);
		} break;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder) {
	uint64_t Result;
	switch(MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELEASE: {
			Result = AK_Atomic_Decrement_U64_Release(Object);
		} break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST: {
			Result = AK_Atomic_Decrement_U64_Seq_Cst(Object);
		} break;

		default: {
			AK_ATOMIC_ASSERT(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELAXED);
			Result = AK_Atomic_Decrement_U64_Relaxed(Object);
		} break;
	}
	return Result;
}

/*OS Primtive implementations*/
#if defined(AK_ATOMIC_OS_WIN32) /*Win32*/

/*Win32 Threads*/
static DWORD CALLBACK AK_Win32_Thread_Callback(void* Parameter) {
	ak_thread* Thread = (ak_thread*)Parameter;
	return (DWORD)Thread->Callback(Thread, Thread->UserData);
}

AKATOMICDEF ak_thread* AK_Thread_Create(ak_thread_callback_func* Callback, void* UserData) {
	ak_win32_thread* Thread = (ak_win32_thread*)AK_ATOMIC_MALLOC(sizeof(ak_win32_thread));
	AK_ATOMIC_ASSERT(Thread);
	if (!Thread) return NULL;

	Thread->Base.Callback = Callback;
	Thread->Base.UserData = UserData;
	Thread->Handle = CreateThread(NULL, 0, AK_Win32_Thread_Callback, Thread, 0, &Thread->ID);
 	AK_ATOMIC_ASSERT(Thread->Handle != NULL);
	if (Thread->Handle == NULL) {
		AK_ATOMIC_FREE(Thread);
		return NULL;
	}
	return &Thread->Base;
}

AKATOMICDEF void AK_Thread_Delete(ak_thread* Thread) {
	ak_win32_thread* Win32Thread = (ak_win32_thread*)Thread;
	AK_ATOMIC_ASSERT(Thread);
	

	if (Win32Thread) {
		AK_Thread_Wait(Thread);

		if (Win32Thread->Handle) {
			CloseHandle(Win32Thread->Handle);
		}

		AK_ATOMIC_FREE(Win32Thread);
	}
}

AKATOMICDEF void AK_Thread_Wait(ak_thread* Thread) {
	ak_win32_thread* Win32Thread = (ak_win32_thread *)Thread;
	AK_ATOMIC_ASSERT(Win32Thread && Win32Thread->Handle);
	if (Win32Thread && Win32Thread->Handle) {
		WaitForSingleObject(Win32Thread->Handle, INFINITE);
	}
}

AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread) {
	uint64_t Result = 0;
	ak_win32_thread* Win32Thread = (ak_win32_thread *)Thread;
	AK_ATOMIC_ASSERT(Thread && Win32Thread->Handle);
	if (Win32Thread) {
		Result = Win32Thread->ID;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void) {
	uint64_t Result = GetCurrentThreadId();
	return Result;
}

AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void) {
	SYSTEM_INFO SystemInfo;
	AK_ATOMIC_MEMORY_CLEAR(&SystemInfo, sizeof(SYSTEM_INFO));
	GetSystemInfo(&SystemInfo);
	return SystemInfo.dwNumberOfProcessors;
}

/*Win32 Mutexes*/
AKATOMICDEF int8_t AK_Mutex_Create(ak_mutex* Mutex) {
	InitializeCriticalSection(&Mutex->CriticalSection);
	return ak_atomic_true;
}

AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex) {
	DeleteCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex) {
	EnterCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex) {
	LeaveCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF int8_t AK_Mutex_Try_Lock(ak_mutex* Mutex) {
	return (int8_t)TryEnterCriticalSection(&Mutex->CriticalSection);
}

/*Win32 Semaphores*/
AKATOMICDEF int8_t AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount) {
	Semaphore->Handle = CreateSemaphoreA(NULL, InitialCount, 0x7fffffff, NULL);
	AK_ATOMIC_ASSERT(Semaphore->Handle != NULL);
	return Semaphore->Handle != NULL;
}

AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore) {
	AK_ATOMIC_ASSERT(Semaphore->Handle != NULL);
	if (Semaphore->Handle != NULL) {
		CloseHandle(Semaphore->Handle);
		Semaphore->Handle = NULL;
	}
}

AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore) {
	AK_ATOMIC_ASSERT(Semaphore->Handle != NULL);
	if (Semaphore->Handle != NULL) {
		ReleaseSemaphore(Semaphore->Handle, 1, NULL);
	}
}

AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore) {
	AK_ATOMIC_ASSERT(Semaphore->Handle != NULL);
	if (Semaphore->Handle != NULL) {
		WaitForSingleObject(Semaphore->Handle, INFINITE);
	}
}

AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Increment) {
	AK_ATOMIC_ASSERT(Semaphore->Handle != NULL);
	if (Semaphore->Handle != NULL) {
		ReleaseSemaphore(Semaphore->Handle, Increment, NULL);
	}
}

/*Win32 Condition Variables*/
AKATOMICDEF int8_t AK_Condition_Variable_Create(ak_condition_variable* ConditionVariable) {
    InitializeConditionVariable(&ConditionVariable->ConditionVariable);
    return ak_atomic_true;
}

AKATOMICDEF void AK_Condition_Variable_Delete(ak_condition_variable* ConditionVariable) {
    /*Noop on win32*/
    AK_ATOMIC__UNREFERENCED_PARAMETER(ConditionVariable);
}

AKATOMICDEF void AK_Condition_Variable_Wait(ak_condition_variable* ConditionVariable, ak_mutex* Mutex) {
    SleepConditionVariableCS(&ConditionVariable->ConditionVariable, &Mutex->CriticalSection, INFINITE);
}

AKATOMICDEF void AK_Condition_Variable_Wake_One(ak_condition_variable* ConditionVariable) {
    WakeConditionVariable(&ConditionVariable->ConditionVariable);
}

AKATOMICDEF void AK_Condition_Variable_Wake_All(ak_condition_variable* ConditionVariable) {
    WakeAllConditionVariable(&ConditionVariable->ConditionVariable);
}

/*Win32 Thread Local Storage*/
AKATOMICDEF int8_t AK_TLS_Create(ak_tls* TLS) {
    TLS->Index = TlsAlloc();
    return TLS->Index != TLS_OUT_OF_INDEXES;
}

AKATOMICDEF void AK_TLS_Delete(ak_tls* TLS) {
	AK_ATOMIC_ASSERT(TLS->Index != TLS_OUT_OF_INDEXES);
	if (TLS->Index != TLS_OUT_OF_INDEXES) {
		TlsFree(TLS->Index);
		TLS->Index = TLS_OUT_OF_INDEXES;
	}
}

AKATOMICDEF void* AK_TLS_Get(ak_tls* TLS) {
	AK_ATOMIC_ASSERT(TLS->Index != TLS_OUT_OF_INDEXES);
	if(TLS->Index != TLS_OUT_OF_INDEXES)
		return TlsGetValue(TLS->Index);
	return NULL;
}

AKATOMICDEF void AK_TLS_Set(ak_tls* TLS, void* Data) {
	AK_ATOMIC_ASSERT(TLS->Index != TLS_OUT_OF_INDEXES);
	if(TLS->Index != TLS_OUT_OF_INDEXES)
		TlsSetValue(TLS->Index, Data);
}

/*Win32 High resolution performance counters & timers*/
AKATOMICDEF void AK_Sleep(uint32_t Milliseconds) {
	Sleep(Milliseconds);
}

AKATOMICDEF uint64_t AK_Query_Performance_Counter(void) {
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return (uint64_t)Result.QuadPart;
}

AKATOMICDEF uint64_t AK_Query_Performance_Frequency(void) {
	LARGE_INTEGER Result;
	QueryPerformanceFrequency(&Result);
	return (uint64_t)Result.QuadPart;
}

#elif defined(AK_ATOMIC_OS_POSIX) /*Posix*/

#include <unistd.h>

/*Posix Threads*/
static void* AK_Thread__Internal_Proc(void* Parameter) {
	ak_thread* Thread = (ak_thread*)Parameter;
	return (void*)(size_t)Thread->Callback(Thread, Thread->UserData);
}

AKATOMICDEF ak_thread* AK_Thread_Create(ak_thread_callback_func* Callback, void* UserData) {
	ak_posix_thread* Thread = (ak_posix_thread*)AK_ATOMIC_MALLOC(sizeof(ak_posix_thread));
	AK_ATOMIC_ASSERT(Thread);
	if(!Thread) return NULL;

	Thread->Base.Callback = Callback;
	Thread->Base.UserData = UserData;
	int ErrorCode = pthread_create(&Thread->Thread, NULL, AK_Thread__Internal_Proc, Thread);
	AK_ATOMIC_ASSERT(ErrorCode == 0);
	if(ErrorCode != 0) {
		AK_ATOMIC_FREE(Thread);
		return NULL;
	}

	return &Thread->Base;
}

AKATOMICDEF void AK_Thread_Delete(ak_thread* Thread) {
	ak_posix_thread* PosixThread = (ak_posix_thread*)Thread;
	AK_ATOMIC_ASSERT(PosixThread);

	if(PosixThread) {
		AK_Thread_Wait(Thread);
		AK_ATOMIC_FREE(PosixThread);
	}
}

AKATOMICDEF void AK_Thread_Wait(ak_thread* Thread) {
	ak_posix_thread* PosixThread = (ak_posix_thread*)Thread;
	AK_ATOMIC_ASSERT(PosixThread);

	if(PosixThread && (PosixThread->Thread != 0)) {
		pthread_join(PosixThread->Thread, NULL);
		PosixThread->Thread = 0;
	}
}

AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread) {
	uint64_t Result = 0;
	ak_posix_thread* PosixThread = (ak_posix_thread *)Thread;
	AK_ATOMIC_ASSERT(PosixThread && (PosixThread->Thread != 0));
	if (PosixThread && (PosixThread->Thread != 0)) {
		Result = (uint64_t)PosixThread->Thread;
	}
	return Result;
}

AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void) {
	uint64_t Result = (uint64_t)pthread_self();
	return Result;
}

AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void) {
    uint32_t Result = (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
	return Result;
}

/*Posix Mutexes*/
AKATOMICDEF int8_t AK_Mutex_Create(ak_mutex* Mutex) {
	int8_t ErrorCode = pthread_mutex_init(&Mutex->Mutex, NULL);
	return ErrorCode == 0;
}

AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex) {
	pthread_mutex_destroy(&Mutex->Mutex);
}

AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex) {
	pthread_mutex_lock(&Mutex->Mutex);
}

AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex) {
	pthread_mutex_lock(&Mutex->Mutex);
}

AKATOMICDEF int8_t AK_Mutex_Try_Lock(ak_mutex* Mutex) {
	return pthread_mutex_trylock(&Mutex->Mutex);
}

#ifdef AK_ATOMIC_OS_OSX 
/*OSX Semaphores*/
AKATOMICDEF int8_t AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount) {
	kern_return_t ErrorCode = semaphore_create(mach_task_self(), &Semaphore->Semaphore, SYNC_POLICY_FIFO, InitialCount);
	AK_ATOMIC_ASSERT(ErrorCode == KERN_SUCCESS);
	return ErrorCode == KERN_SUCCESS;
}

AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore) {
	semaphore_destroy(mach_task_self(), Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore) {
	semaphore_signal(Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore) {
	semaphore_wait(Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Addend) {
	while(Addend > 0) {
		semaphore_signal(Semaphore->Semaphore);
		--Addend;
	}
}

#else
#error "Not Implemented!"
#endif

/*Posix Condition Variables*/
AKATOMICDEF int8_t AK_Condition_Variable_Create(ak_condition_variable* ConditionVariable) {
	int ErrorCode = pthread_cond_init(&ConditionVariable->ConditionVariable, NULL);
	AK_ATOMIC_ASSERT(ErrorCode == 0);
	return ErrorCode == 0;
}

AKATOMICDEF void AK_Condition_Variable_Delete(ak_condition_variable* ConditionVariable) {
	pthread_cond_destroy(&ConditionVariable->ConditionVariable);
}

AKATOMICDEF void AK_Condition_Variable_Wait(ak_condition_variable* ConditionVariable, ak_mutex* Mutex) {
	pthread_cond_wait(&ConditionVariable->ConditionVariable, &Mutex->Mutex);
}

AKATOMICDEF void AK_Condition_Variable_Wake_One(ak_condition_variable* ConditionVariable) {
	pthread_cond_signal(&ConditionVariable->ConditionVariable);
}

AKATOMICDEF void AK_Condition_Variable_Wake_All(ak_condition_variable* ConditionVariable) {
	pthread_cond_broadcast(&ConditionVariable->ConditionVariable);
}

/*Posix Thread Local Storage*/
AKATOMICDEF int8_t AK_TLS_Create(ak_tls* TLS) {
	int ErrorCode = pthread_key_create(&TLS->Key, NULL) == 0;
	AK_ATOMIC_ASSERT(ErrorCode == 0);
	return ErrorCode == 0;
} 

AKATOMICDEF void AK_TLS_Delete(ak_tls* TLS) {
	pthread_key_delete(TLS->Key);
}

AKATOMICDEF void* AK_TLS_Get(ak_tls* TLS) {
	return pthread_getspecific(TLS->Key);
}

AKATOMICDEF void AK_TLS_Set(ak_tls* TLS, void* Data) {
	pthread_setspecific(TLS->Key, Data);
}

/*Posix High resolution performance counters & timers*/
AKATOMICDEF void AK_Sleep(uint32_t Milliseconds) {
    struct timespec Time;
    Time.tv_sec = Milliseconds / 1000;
    Time.tv_nsec = (Milliseconds % 1000) * 1000000;
    nanosleep(&Time, NULL);
}

#define AK__NS_PER_SECOND 1000000000
AKATOMICDEF uint64_t AK_Query_Performance_Counter() {
    struct timespec Now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &Now);

    uint64_t Result = Now.tv_sec;
    Result *= AK__NS_PER_SECOND;
    Result += Now.tv_nsec;

    return Result;
}

AKATOMICDEF uint64_t AK_Query_Performance_Frequency() {
    return AK__NS_PER_SECOND;
}

#else
#error "Not Implemented!"
#endif

#ifdef AK_ATOMIC_COMPILER_MSVC
#pragma warning(pop)
#endif

#ifdef AK_ATOMIC_COMPILER_CLANG
#pragma clang diagnostic pop
#endif

#endif
