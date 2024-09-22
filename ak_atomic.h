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
#else
	#error "Unrecognized Platform!"
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

#if defined(AK_ATOMIC_COMPILER_MSVC)

/*Warnings for intrin.h*/
#pragma warning(push, 0)
#include <intrin.h>
#pragma warning(pop)

typedef struct {
	uint8_t Nonatomic;
} ak_atomic_u8;

typedef struct {
	uint16_t Nonatomic;
} ak_atomic_u16;

typedef struct {
	uint32_t Nonatomic;
} ak_atomic_u32;

typedef struct {
	uint64_t Nonatomic;
} ak_atomic_u64;

typedef struct {
	void* Nonatomic;
} ak_atomic_ptr;

#define AK_Atomic_Fence_Acquire() _ReadBarrier()
#define AK_Atomic_Fence_Release() _WriteBarrier()
#define AK_Atomic_Fence_Seq_Cst() _ReadWriteBarrier(); MemoryBarrier()

#else
#error "Not Implemented!"
#endif

AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u8)  == 1);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u16) == 2);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u32) == 4);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u64) == 8);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_ptr) == AK_ATOMIC_PTR_SIZE);

/*Compiler specific functions (all other atomics are built ontop of these)*/
AKATOMICDEF uint8_t AK_Atomic_Load_U8_Relaxed(const ak_atomic_u8* Object);
AKATOMICDEF void    AK_Atomic_Store_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value);
AKATOMICDEF uint8_t AK_Atomic_Exchange_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value);
AKATOMICDEF uint8_t AK_Atomic_Compare_Exchange_U8_Relaxed(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue);
AKATOMICDEF int8_t  AK_Atomic_Compare_Exchange_U8_Weak_Relaxed(ak_atomic_u8* Object, uint8_t* OldValue, uint8_t NewValue);
AKATOMICDEF uint8_t AK_Atomic_Fetch_Add_U8_Relaxed(ak_atomic_u8* Object, int8_t Operand);
AKATOMICDEF uint8_t AK_Atomic_Increment_U8_Relaxed(ak_atomic_u8* Object);
AKATOMICDEF uint8_t AK_Atomic_Decrement_U8_Relaxed(ak_atomic_u8* Object); 

AKATOMICDEF uint16_t AK_Atomic_Load_U16_Relaxed(const ak_atomic_u16* Object);
AKATOMICDEF void     AK_Atomic_Store_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value);
AKATOMICDEF uint16_t AK_Atomic_Exchange_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value);
AKATOMICDEF uint16_t AK_Atomic_Compare_Exchange_U16_Relaxed(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U16_Weak_Relaxed(ak_atomic_u16* Object, uint16_t* OldValue, uint16_t NewValue);
AKATOMICDEF uint16_t AK_Atomic_Fetch_Add_U16_Relaxed(ak_atomic_u16* Object, int16_t Operand);
AKATOMICDEF uint16_t AK_Atomic_Increment_U16_Relaxed(ak_atomic_u16* Object);
AKATOMICDEF uint16_t AK_Atomic_Decrement_U16_Relaxed(ak_atomic_u16* Object); 

AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object);
AKATOMICDEF void     AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value);
AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand);
AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object);
AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object); 

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object);
AKATOMICDEF void     AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value);
AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value);
AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue);
AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand);
AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object);
AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object); 

/*Ptr type (is either 32 bit or 64 bit wrappers)*/
AKATOMICDEF void*  AK_Atomic_Load_Ptr_Relaxed(const ak_atomic_ptr* Object);
AKATOMICDEF void   AK_Atomic_Store_Ptr_Relaxed(ak_atomic_ptr* Object, void* Value);
AKATOMICDEF void*  AK_Atomic_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* NewValue);
AKATOMICDEF void*  AK_Atomic_Compare_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(ak_atomic_ptr* Object, void** OldValue, void* NewValue);

/*Compare exchange for boolean results*/
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue);

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
AKATOMICDEF uint8_t AK_Atomic_Compare_Exchange_U8(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t  AK_Atomic_Compare_Exchange_U8_Weak(ak_atomic_u8* Object, uint8_t* OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t  AK_Atomic_Compare_Exchange_Bool_U8(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t  AK_Atomic_Compare_Exchange_U8_Weak_Explicit(ak_atomic_u8* Object, uint8_t* OldValue, uint8_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t  AK_Atomic_Compare_Exchange_Bool_U8_Explicit(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint8_t AK_Atomic_Fetch_Add_U8(ak_atomic_u8* Object, int8_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Increment_U8(ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Decrement_U8(ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint16_t AK_Atomic_Load_U16(const ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Exchange_U16(ak_atomic_u16* Object, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Compare_Exchange_U16(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U16_Weak(ak_atomic_u16* Object, uint16_t* OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_Bool_U16(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U16_Weak_Explicit(ak_atomic_u16* Object, uint16_t* OldValue, uint16_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_Bool_U16_Explicit(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint16_t AK_Atomic_Fetch_Add_U16(ak_atomic_u16* Object, int16_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Increment_U16(ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Decrement_U16(ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint32_t AK_Atomic_Load_U32(const ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Exchange_U32(ak_atomic_u32* Object, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U32_Weak(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_Bool_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U32_Weak_Explicit(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_Bool_U32_Explicit(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32(ak_atomic_u32* Object, int32_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Increment_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Decrement_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint64_t  AK_Atomic_Load_U64(const ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t    AK_Atomic_Store_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Exchange_U64(ak_atomic_u64* Object, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Compare_Exchange_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t    AK_Atomic_Compare_Exchange_U64_Weak(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t    AK_Atomic_Compare_Exchange_Bool_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t    AK_Atomic_Compare_Exchange_U64_Weak_Explicit(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t    AK_Atomic_Compare_Exchange_Bool_U64_Explicit(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint64_t  AK_Atomic_Fetch_Add_U64(ak_atomic_u64* Object, int64_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Increment_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Decrement_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF void*  AK_Atomic_Load_Ptr(const ak_atomic_ptr* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void   AK_Atomic_Store_Ptr(ak_atomic_ptr* Object, void* Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void*  AK_Atomic_Exchange_Ptr(ak_atomic_ptr* Object, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void*  AK_Atomic_Compare_Exchange_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Ptr_Weak(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Bool_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Ptr_Weak_Explicit(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Bool_Ptr_Explicit(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);

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
#pragma warning(disable : 4061 4711)
#endif

/*Custom allocators*/
#if !defined(AK_ATOMIC_MALLOC) || !defined(AK_ATOMIC_FREE)
#include <stdlib.h>
#define AK_ATOMIC_MALLOC(size) malloc(size)
#define AK_ATOMIC_FREE(memory) free(memory)
#endif

#if !defined(AK_ATOMIC_MEMSET)
#include <string.h>
#define AK_ATOMIC_MEMSET(dst, value, size) memset(dst, value, size)
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

/*Atomic functions with memory order parameters*/
AKATOMICDEF uint8_t AK_Atomic_Load_U8(const ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder) {
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELAXED: return AK_Atomic_Load_U8_Relaxed(Object);
		
		case AK_ATOMIC_MEMORY_ORDER_ACQUIRE:
		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL: 
			AK_Atomic_Fence_Acquire();
			return AK_Atomic_Load_U8_Relaxed(Object);

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST:
			AK_Atomic_Fence_Seq_Cst();
			return AK_Atomic_Load_U8_Relaxed(Object);

		default:
			AK_ATOMIC_ASSERT(!"Invalid param");
			return AK_Atomic_Load_U8_Relaxed(Object);
	}
}

AKATOMICDEF void AK_Atomic_Store_U8(ak_atomic_u8* Object, uint8_t Value, ak_atomic_memory_order MemoryOrder) {
	switch (MemoryOrder) {
		case AK_ATOMIC_MEMORY_ORDER_RELAXED: 
			AK_Atomic_Store_U8_Relaxed(Object, Value);
			break;

		case AK_ATOMIC_MEMORY_ORDER_RELEASE:
		case AK_ATOMIC_MEMORY_ORDER_ACQ_REL:
			AK_Atomic_Store_U8_Relaxed(Object, Value);
			AK_Atomic_Fence_Release();
			break;

		case AK_ATOMIC_MEMORY_ORDER_SEQ_CST:
			AK_Atomic_Store_U8_Relaxed(Object, Value);
			AK_Atomic_Fence_Seq_Cst();
			break;

		default:
			AK_ATOMIC_ASSERT(!"Invalid param");
			AK_Atomic_Store_U8_Relaxed(Object, Value);
			break;
	}
}

AKATOMICDEF uint8_t AK_Atomic_Exchange_U8(ak_atomic_u8* Object, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Compare_Exchange_U8(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t  AK_Atomic_Compare_Exchange_U8_Weak(ak_atomic_u8* Object, uint8_t* OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t  AK_Atomic_Compare_Exchange_Bool_U8(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t  AK_Atomic_Compare_Exchange_U8_Weak_Explicit(ak_atomic_u8* Object, uint8_t* OldValue, uint8_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t  AK_Atomic_Compare_Exchange_Bool_U8_Explicit(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint8_t AK_Atomic_Fetch_Add_U8(ak_atomic_u8* Object, int8_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Increment_U8(ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint8_t AK_Atomic_Decrement_U8(ak_atomic_u8* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint16_t AK_Atomic_Load_U16(const ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U16(ak_atomic_u16* Object, uint16_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Exchange_U16(ak_atomic_u16* Object, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Compare_Exchange_U16(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U16_Weak(ak_atomic_u16* Object, uint16_t* OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_Bool_U16(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U16_Weak_Explicit(ak_atomic_u16* Object, uint16_t* OldValue, uint16_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_Bool_U16_Explicit(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint16_t AK_Atomic_Fetch_Add_U16(ak_atomic_u16* Object, int16_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Increment_U16(ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint16_t AK_Atomic_Decrement_U16(ak_atomic_u16* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint32_t AK_Atomic_Load_U32(const ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Exchange_U32(ak_atomic_u32* Object, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U32_Weak(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_Bool_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_U32_Weak_Explicit(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t   AK_Atomic_Compare_Exchange_Bool_U32_Explicit(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32(ak_atomic_u32* Object, int32_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Increment_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Decrement_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint64_t  AK_Atomic_Load_U64(const ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t    AK_Atomic_Store_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Exchange_U64(ak_atomic_u64* Object, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Compare_Exchange_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t    AK_Atomic_Compare_Exchange_U64_Weak(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t    AK_Atomic_Compare_Exchange_Bool_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t    AK_Atomic_Compare_Exchange_U64_Weak_Explicit(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t    AK_Atomic_Compare_Exchange_Bool_U64_Explicit(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint64_t  AK_Atomic_Fetch_Add_U64(ak_atomic_u64* Object, int64_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Increment_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Decrement_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF void*  AK_Atomic_Load_Ptr(const ak_atomic_ptr* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void   AK_Atomic_Store_Ptr(ak_atomic_ptr* Object, void* Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void*  AK_Atomic_Exchange_Ptr(ak_atomic_ptr* Object, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void*  AK_Atomic_Compare_Exchange_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Ptr_Weak(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Bool_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Ptr_Weak_Explicit(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_Bool_Ptr_Explicit(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);

/*CPU and compiler specific architecture (all other atomics are built ontop of these)*/
#if defined(AK_ATOMIC_COMPILER_MSVC)

AKATOMICDEF uint8_t AK_Atomic_Load_U8_Relaxed(const ak_atomic_u8* Object) {
	/*Cast to a volatile type so that the compiler doesn't duplicate loads, which makes it nonatomic.*/
	return ((volatile ak_atomic_u8 *)Object)->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value) {
	((volatile ak_atomic_u8 *)Object)->Nonatomic = Value;
}

AKATOMICDEF uint8_t AK_Atomic_Exchange_U8_Relaxed(ak_atomic_u8* Object, uint8_t Value) {
	return (uint8_t)_InterlockedExchange8((volatile char*)Object, (char)Value);
}

AKATOMICDEF uint8_t AK_Atomic_Compare_Exchange_U8_Relaxed(ak_atomic_u8* Object, uint8_t OldValue, uint8_t NewValue) {
	return (uint8_t)_InterlockedCompareExchange8((volatile char*)Object, (char)NewValue, (char)OldValue);
}

AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_U8_Weak_Relaxed(ak_atomic_u8* Object, uint8_t* OldValue, uint8_t NewValue) {
	uint8_t Expected = *OldValue;
	uint8_t Previous = (uint8_t)_InterlockedCompareExchange8((volatile char*)Object, (char)NewValue, (char)Expected);
	int8_t Result = Previous == Expected;
	if (!Result) *OldValue = Previous;
	return Result;
}

AKATOMICDEF uint8_t AK_Atomic_Fetch_Add_U8_Relaxed(ak_atomic_u8* Object, int8_t Operand) {
	return (uint8_t)_InterlockedExchangeAdd8((volatile char*)Object, Operand);
}

AKATOMICDEF uint8_t AK_Atomic_Increment_U8_Relaxed(ak_atomic_u8* Object) {
	return (uint8_t)(AK_Atomic_Fetch_Add_U8_Relaxed(Object, 1) + 1);
}

AKATOMICDEF uint8_t AK_Atomic_Decrement_U8_Relaxed(ak_atomic_u8* Object) {
	return (uint8_t)(AK_Atomic_Fetch_Add_U8_Relaxed(Object, -1) - 1);
}

AKATOMICDEF uint16_t AK_Atomic_Load_U16_Relaxed(const ak_atomic_u16* Object) {
	/*Cast to a volatile type so that the compiler doesn't duplicate loads, which makes it nonatomic.*/
	return ((volatile ak_atomic_u16 *)Object)->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value) {
	((volatile ak_atomic_u16 *)Object)->Nonatomic = Value;
}

AKATOMICDEF uint16_t AK_Atomic_Exchange_U16_Relaxed(ak_atomic_u16* Object, uint16_t Value) {
	return (uint16_t)_InterlockedExchange16((volatile short *)Object, (short)Value);
}

AKATOMICDEF uint16_t AK_Atomic_Compare_Exchange_U16_Relaxed(ak_atomic_u16* Object, uint16_t OldValue, uint16_t NewValue) {
	return (uint16_t)_InterlockedCompareExchange16((volatile short *)Object, (short)NewValue, (short)OldValue);
}

AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_U16_Weak_Relaxed(ak_atomic_u16* Object, uint16_t* OldValue, uint16_t NewValue) {
	uint16_t Expected = *OldValue;
	uint16_t Previous = (uint16_t)_InterlockedCompareExchange16((volatile short*)Object, (short)NewValue, (short)Expected);
	int8_t Result = Previous == Expected;
	if (!Result) *OldValue = Previous;
	return Result;
}

AKATOMICDEF uint16_t AK_Atomic_Fetch_Add_U16_Relaxed(ak_atomic_u16* Object, int16_t Operand) {
	return (uint16_t)_InterlockedExchange16((volatile short *)Object, Operand);
}

AKATOMICDEF uint16_t AK_Atomic_Increment_U16_Relaxed(ak_atomic_u16* Object) {
	return (uint16_t)_InterlockedIncrement16((volatile short *)Object);
}

AKATOMICDEF uint16_t AK_Atomic_Decrement_U16_Relaxed(ak_atomic_u16* Object) {
	return (uint16_t)_InterlockedDecrement16((volatile short *)Object);
}

AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
	/*Cast to a volatile type so that the compiler doesn't duplicate loads, which makes it nonatomic.*/
	return ((volatile ak_atomic_u32 *)Object)->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
	((volatile ak_atomic_u32 *)Object)->Nonatomic = Value;
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
	return (uint32_t)_InterlockedExchange((volatile long *)Object, (long)Value);
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
	return (uint32_t)_InterlockedCompareExchange((volatile long *)Object, (long)NewValue, (long)OldValue);
}

AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue) {
	uint32_t Expected = *OldValue;
	uint32_t Previous = (uint32_t)_InterlockedCompareExchange((volatile long*)Object, (long)NewValue, (long)Expected);
	int8_t Result = Previous == Expected;
	if (!Result) *OldValue = Previous;
	return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand) {
	return (uint32_t)_InterlockedExchangeAdd((volatile long*)Object, (long)Operand);
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
	return (uint32_t)_InterlockedIncrement((volatile long *)Object);
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
	return (uint32_t)_InterlockedDecrement((volatile long *)Object);
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
#if (AK_ATOMIC_PTR_SIZE == 8)
	/*Cast to a volatile type so that the compiler doesn't duplicate loads, which makes it nonatomic.*/
	return ((volatile ak_atomic_u64 *)Object)->Nonatomic;
#else
	/*Interlocked compare exchange is the most compatibile way to get an atomic 
	64 bit load on 32 bit x86*/
	return (uint64_t)AK_Atomic_Compare_Exchange_U64_Relaxed((ak_atomic_u64 *)Object, 0, 0);
#endif
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
#if (AK_ATOMIC_PTR_SIZE == 8)
	((volatile ak_atomic_u64 *)Object)->Nonatomic = Value;
#else
	/*Interlocked compare exchange is the most compatible way to get an atomic 
	64 bit store on 32 bit x86*/
	uint64_t Expected = Object->Nonatomic;
	for(;;) {
		uint64_t Previous = (uint64_t)_InterlockedCompareExchange64((volatile __int64 *)Object, (__int64)Value, (__int64)Expected);
		if (Previous != Expected) break;
		Expected = Previous;
	}
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return (uint64_t)_InterlockedExchange64((volatile __int64*)Object, (__int64)Value);
#else
    uint64_t Expected = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = (uint64_t)_InterlockedCompareExchange64((volatile __int64*)Object, (__int64)Value, (__int64)Expected);
        if(Previous == Expected) return Previous;
        Expected = Previous;
    }
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
	return (uint64_t)_InterlockedCompareExchange64((volatile __int64*)Object, (__int64)NewValue, (__int64)OldValue);
}

AKATOMICDEF int8_t AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = (uint64_t)_InterlockedCompareExchange64((volatile __int64*)Object, (__int64)NewValue, (__int64)Old);
    int8_t Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return (uint64_t)_InterlockedExchangeAdd64((volatile __int64*)Object, (__int64)Operand);
#else
    uint64_t Expected = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = (uint64_t)_InterlockedCompareExchange64((volatile __int64*)Object, (__int64)(Expected+Operand), (__int64)Expected);
        if(Previous == Expected) return Previous;
        Expected = Previous;
    }
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return (uint64_t)_InterlockedIncrement64((volatile __int64*)Object);
#else
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1)+1;
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return (uint64_t)_InterlockedDecrement64((volatile __int64*)Object);
#else
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
#endif
}

#else
#error "Not Implemented!"
#endif

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
	AK_ATOMIC_ASSERT(Thread);
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
	AK_ATOMIC_MEMSET(&SystemInfo, 0, sizeof(SYSTEM_INFO));
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

#else
#error "Not Implemented!"
#endif

#ifdef AK_ATOMIC_COMPILER_MSVC
#pragma warning(pop)
#endif

#endif
