#ifndef AK_ATOMIC_H
#define AK_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#   define AK_ATOMIC_WIN32_OS
#   define AK_ATOMIC_MSVC_COMPILER
#   if defined(_M_X64)
#       define AK_ATOMIC_PTR_SIZE 8
#   elif defined(_M_IX86)
#       define AK_ATOMIC_PTR_SIZE 4
#   else
#       error "Unrecognized platform!"
#   endif
#elif defined(__GNUC__)
#   define AK_ATOMIC_GCC_COMPILER
#   define AK_ATOMIC_POSIX_OS

#   if defined(__APPLE__)
#       define AK_ATOMIC_OSX_OS
#   endif

#   if defined(__arm__)
#       define AK_ATOMIC_ARM_CPU
#       define AK_ATOMIC_PTR_SIZE 4
#       if defined(__thumb__)
            #define AK_ATOMIC_ARM_THUMB 1
#       endif
#   elif defined(__aarch64__)
#       define AK_ATOMIC_AARCH64_CPU
#       define AK_ATOMIC_PTR_SIZE 8
#   elif defined(__x86_64__)
#       define AK_ATOMIC_X86_64_CPU
#       define AK_ATOMIC_PTR_SIZE 8   
#   elif defined(__i386__)
#       define AK_ATOMIC_X86_64_CPU
#       define AK_ATOMIC_PTR_SIZE 4    
#   else
#   error "Not Implemented"
#   endif
#endif

#ifndef AKATOMICDEF
# ifdef AK_ATOMIC_STATIC
# define AKATOMICDEF static
# else
# define AKATOMICDEF extern
# endif
#endif

#define AK_ATOMIC__STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]
#define AK_ATOMIC__COMPILE_TIME_ASSERT3(X,L) AK_ATOMIC__STATIC_ASSERT(X,static_assertion_at_line_##L)
#define AK_ATOMIC__COMPILE_TIME_ASSERT2(X,L) AK_ATOMIC__COMPILE_TIME_ASSERT3(X,L)
#define AK_ATOMIC__COMPILE_TIME_ASSERT(X)    AK_ATOMIC__COMPILE_TIME_ASSERT2(X,__LINE__)

#if defined(AK_ATOMIC_POSIX_OS)
/*c89 posix won't seem to recognized the time functions
  without this (nanosleep clock_getttime) */
#define _POSIX_C_SOURCE 199309L
#define _DARWIN_C_SOURCE
#endif

/*
If stdint is not included for your platform, define AK_ATOMIC_CUSTOM_TYPES
and define the stdint types:
-uint32_t -> 32 bit unsigned integer
-int32_t -> 32 bit signed integer
-uint64_t -> 64 bit unsigned integer
-int64_t -> 64 bit signed integer
-bool -> 8 bit signed integer
*/

#ifndef AK_ATOMIC_CUSTOM_TYPES
#include <stdint.h>
#include <stdbool.h>
#endif


#if defined(AK_ATOMIC_MSVC_COMPILER)

#include <intrin.h>

typedef struct ak_atomic_u32 {
	uint32_t Nonatomic;
} ak_atomic_u32;

typedef struct ak_atomic_u64 {
	uint64_t Nonatomic;
} ak_atomic_u64;

typedef struct ak_atomic_ptr {
	void* Nonatomic;
} ak_atomic_ptr;

#define AK_Atomic_Compiler_Fence_Acq() _ReadBarrier()
#define AK_Atomic_Compiler_Fence_Rel() _WriteBarrier()
#define AK_Atomic_Compiler_Fence_Seq_Cst() _ReadWriteBarrier()

#define AK_Atomic_Thread_Fence_Acq() _ReadBarrier()
#define AK_Atomic_Thread_Fence_Rel() _WriteBarrier()
#define AK_Atomic_Thread_Fence_Seq_Cst() MemoryBarrier()

#elif defined(AK_ATOMIC_GCC_COMPILER) && (defined(AK_ATOMIC_AARCH64_CPU) || defined(AK_ATOMIC_ARM_CPU))

/*Atomic operators on this architecture need to be aligned properly*/

typedef struct ak_atomic_u32 {
	volatile uint32_t Nonatomic;
} __attribute__((aligned(4))) ak_atomic_u32;

typedef struct ak_atomic_u64 {
	volatile uint64_t Nonatomic;
} __attribute__((aligned(8))) ak_atomic_u64;

typedef struct ak_atomic_ptr {
	void* volatile Nonatomic;
} __attribute__((aligned(AK_ATOMIC_PTR_SIZE))) ak_atomic_ptr;

#define AK_Atomic_Compiler_Fence_Acq() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Compiler_Fence_Rel() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Compiler_Fence_Seq_Cst() __asm__ volatile("" ::: "memory")

#define AK_Atomic_Thread_Fence_Acq() __asm__ volatile("dmb ish" ::: "memory")
#define AK_Atomic_Thread_Fence_Rel() __asm__ volatile("dmb ish" ::: "memory")
#define AK_Atomic_Thread_Fence_Seq_Cst() __asm__ volatile("dmb ish" ::: "memory")

#elif defined(AK_ATOMIC_GCC_COMPILER) && defined(AK_ATOMIC_X86_64_CPU)

typedef struct ak_atomic_u32 {
	volatile uint32_t Nonatomic;
} __attribute__((aligned(4))) ak_atomic_u32;

typedef struct ak_atomic_u64 {
	volatile uint64_t Nonatomic;
} __attribute__((aligned(8))) ak_atomic_u64;

typedef struct ak_atomic_ptr {
	volatile void* Nonatomic;
} __attribute__((aligned(AK_ATOMIC_PTR_SIZE))) ak_atomic_ptr;

#define AK_Atomic_Compiler_Fence_Acq() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Compiler_Fence_Rel() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Compiler_Fence_Seq_Cst() __asm__ volatile("" ::: "memory")

#define AK_Atomic_Thread_Fence_Acq() __asm__ volatile("" ::: "memory")
#define AK_Atomic_Thread_Fence_Rel() __asm__ volatile("" ::: "memory")

#if AK_ATOMIC_PTR_SIZE == 8
#define AK_Atomic_Thread_Fence_Seq_Cst() __asm__ volatile("lock; orl $0, (%%rsp)" ::: "memory")
#else
#define AK_Atomic_Thread_Fence_Seq_Cst() __asm__ volatile("lock; orl $0, (%%esp)" ::: "memory")
#endif

#else
#error "Not Implemented"
#endif

AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u32) == 4);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u64) == 8);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_ptr) == AK_ATOMIC_PTR_SIZE);

/*Compiler specific functions (all other atomics are built ontop of these)*/
AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object);
AKATOMICDEF void     AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value);
AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand);
AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object);
AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object);

AKATOMICDEF uint64_t  AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object);
AKATOMICDEF void      AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value);
AKATOMICDEF uint64_t  AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue);
AKATOMICDEF uint64_t  AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue);
AKATOMICDEF uint64_t  AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand);
AKATOMICDEF uint64_t  AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object);
AKATOMICDEF uint64_t  AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object);

/*Ptr type (is either 32 bit or 64 bit wrappers)*/
AKATOMICDEF void* AK_Atomic_Load_Ptr_Relaxed(const ak_atomic_ptr* Object);
AKATOMICDEF void  AK_Atomic_Store_Ptr_Relaxed(ak_atomic_ptr* Object, void* Value);
AKATOMICDEF void* AK_Atomic_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* NewValue);
AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(ak_atomic_ptr* Object, void** OldValue, void* NewValue);

/*Compare exchange for boolean results*/
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue);
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue);
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue);

typedef enum ak_atomic_memory_order {
    AK_ATOMIC_MEMORY_ORDER_RELAXED,
    AK_ATOMIC_MEMORY_ORDER_ACQUIRE,
    AK_ATOMIC_MEMORY_ORDER_RELEASE,
    AK_ATOMIC_MEMORY_ORDER_ACQ_REL
} ak_atomic_memory_order;

/*Atomic functions with memory order parameters*/
AKATOMICDEF uint32_t AK_Atomic_Load_U32(const ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Exchange_U32(ak_atomic_u32* Object, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_U32_Weak(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_Bool_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_U32_Weak_Explicit(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_Bool_U32_Explicit(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32(ak_atomic_u32* Object, int32_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Increment_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Decrement_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint64_t  AK_Atomic_Load_U64(const ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void      AK_Atomic_Store_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Exchange_U64(ak_atomic_u64* Object, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Compare_Exchange_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_U64_Weak(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_Bool_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_U64_Weak_Explicit(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_Bool_U64_Explicit(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF uint64_t  AK_Atomic_Fetch_Add_U64(ak_atomic_u64* Object, int64_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Increment_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Decrement_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF void* AK_Atomic_Load_Ptr(const ak_atomic_ptr* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void  AK_Atomic_Store_Ptr(ak_atomic_ptr* Object, void* Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void* AK_Atomic_Exchange_Ptr(ak_atomic_ptr* Object, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Ptr_Weak(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Bool_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Ptr_Weak_Explicit(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Bool_Ptr_Explicit(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);


/*Thread primitives*/
typedef struct ak_thread ak_thread;
#define AK_THREAD_CALLBACK_DEFINE(name) int32_t name(ak_thread* Thread, void* UserData)
typedef AK_THREAD_CALLBACK_DEFINE(ak_thread_callback_func);

#define AK_CONDITION_VARIABLE_PREDICATE_DEFINE(name) bool name(void* UserData)
typedef AK_CONDITION_VARIABLE_PREDICATE_DEFINE(ak_condition_variable_predicate_func);

#if defined(AK_ATOMIC_WIN32_OS)
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

struct ak_thread {
    HANDLE                   Handle;
    ak_thread_callback_func* Callback;
    void*                    UserData;
};

typedef struct ak_mutex {
    CRITICAL_SECTION CriticalSection;
} ak_mutex;

typedef struct ak_semaphore {
    HANDLE Handle;
} ak_semaphore;

typedef struct ak_condition_variable {
    CONDITION_VARIABLE Variable;
} ak_condition_variable;

typedef struct ak_tls {
    DWORD Index;
} ak_tls;

#elif defined(AK_ATOMIC_POSIX_OS)
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

struct ak_thread {
    pthread_t                Thread;
    ak_thread_callback_func* Callback;
    void*                    UserData;
};

typedef struct ak_mutex {
    pthread_mutex_t Mutex;
} ak_mutex;

#ifdef AK_ATOMIC_OSX_OS
#include <mach/mach.h>

typedef struct ak_semaphore {
    semaphore_t Semaphore;
} ak_semaphore;

#else
typedef struct ak_semaphore {
    sem_t Semaphore;
} ak_semaphore;
#endif

typedef struct ak_condition_variable {
    pthread_cond_t Variable;
} ak_condition_variable;

typedef struct ak_tls {
    pthread_key_t Key;
} ak_tls;

#else
#error "Not Implemented"
#endif

AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void);
AKATOMICDEF void     AK_Sleep(uint32_t Milliseconds);
AKATOMICDEF uint64_t AK_Query_Performance_Counter(void);
AKATOMICDEF uint64_t AK_Query_Performance_Frequency(void);

AKATOMICDEF bool     AK_Thread_Create(ak_thread* Thread, ak_thread_callback_func* Callback, void* UserData);
AKATOMICDEF void     AK_Thread_Delete(ak_thread* Thread);
AKATOMICDEF void     AK_Thread_Wait(ak_thread* Thread);
AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread);
AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void);

AKATOMICDEF bool AK_Mutex_Create(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex);
AKATOMICDEF bool AK_Mutex_Try_Lock(ak_mutex* Mutex);

AKATOMICDEF bool AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount);
AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore);
AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore);
AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore);
AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Addend);

AKATOMICDEF bool AK_Condition_Variable_Create(ak_condition_variable* ConditionVariable);
AKATOMICDEF void AK_Condition_Variable_Delete(ak_condition_variable* ConditionVariable);
AKATOMICDEF void AK_Condition_Variable_Wait(ak_condition_variable* ConditionVariable, ak_mutex* Mutex, 
                                            ak_condition_variable_predicate_func* PredicateFunc, void* UserData);
AKATOMICDEF void AK_Condition_Variable_Wake_One(ak_condition_variable* ConditionVariable);
AKATOMICDEF void AK_Condition_Variable_Wake_All(ak_condition_variable* ConditionVariable);

AKATOMICDEF bool  AK_TLS_Create(ak_tls* TLS);
AKATOMICDEF void  AK_TLS_Delete(ak_tls* TLS);
AKATOMICDEF void* AK_TLS_Get(ak_tls* TLS);
AKATOMICDEF void  AK_TLS_Set(ak_tls* TLS, void* Data);

typedef struct ak_lw_semaphore {
    ak_semaphore  InternalSem;
    ak_atomic_u32 Count;
    uint32_t      MaxSpinCount;
} ak_lw_semaphore;

AKATOMICDEF bool AK_LW_Semaphore_Create_With_Spin_Count(ak_lw_semaphore* Semaphore, int32_t InitialCount, uint32_t SpinCount);
AKATOMICDEF bool AK_LW_Semaphore_Create(ak_lw_semaphore* Semaphore, int32_t InitialCount);
AKATOMICDEF void AK_LW_Semaphore_Delete(ak_lw_semaphore* Semaphore);
AKATOMICDEF void AK_LW_Semaphore_Increment(ak_lw_semaphore* Semaphore);
AKATOMICDEF void AK_LW_Semaphore_Decrement(ak_lw_semaphore* Semaphore);
AKATOMICDEF void AK_LW_Semaphore_Add(ak_lw_semaphore* Semaphore, int32_t Addend);

#ifndef AK_DISABLE_ASYNC_STACK_INDEX

#define AK_ASYNC_STACK_INDEX32_INVALID ((uint32_t)-1) 

typedef struct {
    void*         AllocUserData;
    uint32_t*     NextIndices;
    ak_atomic_u64 Head;
    uint32_t      Capacity;
    uint32_t      Unused__Padding;
} ak_async_stack_index32;

AKATOMICDEF void     AK_Async_Stack_Index32_Init_Raw(ak_async_stack_index32* StackIndex, uint32_t* IndicesPtr, uint32_t Capacity);
AKATOMICDEF void     AK_Async_Stack_Index32_Push_Sync(ak_async_stack_index32* StackIndex, uint32_t Index);
AKATOMICDEF void     AK_Async_Stack_Index32_Push(ak_async_stack_index32* StackIndex, uint32_t Index);
AKATOMICDEF uint32_t AK_Async_Stack_Index32_Pop(ak_async_stack_index32* StackIndex);
AKATOMICDEF bool     AK_Async_Stack_Index32_Alloc(ak_async_stack_index32* StackIndex, uint32_t Capacity, void* AllocUserData);
AKATOMICDEF void     AK_Async_Stack_Index32_Free(ak_async_stack_index32* StackIndex);

#endif

#ifndef AK_DISABLE_ASYNC_SLOT_MAP

typedef uint64_t ak_slot64;

typedef union {
    ak_slot64 Slot;
    struct {
        uint32_t      Index;
        ak_atomic_u32 Generation;
    } Data;
} ak_slot64__internal;

typedef struct {
    void*                  AllocUserData;
    ak_async_stack_index32 FreeIndices;
    ak_slot64__internal*   Slots; /*Array size is specified by FreeIndices.Capacity*/
} ak_async_slot_map64;

AKATOMICDEF void      AK_Async_Slot_Map64_Init_Raw(ak_async_slot_map64* SlotMap, uint32_t* IndicesPtr, ak_slot64* SlotsPtr, uint32_t Capacity);
AKATOMICDEF bool      AK_Async_Slot_Map64_Is_Allocated(ak_async_slot_map64* SlotMap, ak_slot64 Slot);
AKATOMICDEF ak_slot64 AK_Async_Slot_Map64_Alloc_Slot(ak_async_slot_map64* SlotMap);
AKATOMICDEF void      AK_Async_Slot_Map64_Free_Slot(ak_async_slot_map64* SlotMap, ak_slot64 Slot);
AKATOMICDEF bool      AK_Async_Slot_Map64_Alloc(ak_async_slot_map64* SlotMap, uint32_t Capacity, void* AllocUserData);
AKATOMICDEF void      AK_Async_Slot_Map64_Free(ak_async_slot_map64* SlotMap);

#endif

#ifndef AK_DISABLE_JOB_SYSTEM

#if !defined(AK_JOB_SYSTEM_FAST_USERDATA_SIZE)
#   if AK_ATOMIC_PTR_SIZE == 8
#       define AK_JOB_SYSTEM_FAST_USERDATA_SIZE 35
#   else
#       define AK_JOB_SYSTEM_FAST_USERDATA_SIZE 43
#   endif
#endif

typedef uint64_t ak_job_id;
typedef struct ak_job_system ak_job_system;

typedef enum ak_job_status {
    AK_JOB_STATUS_COMPLETE,
    AK_JOB_STATUS_REQUEUE
} ak_job_status;

#define AK_JOB_CALLBACK_DEFINE(name) ak_job_status name(ak_job_system* JobSystem, ak_job_id JobID, void* JobUserData)
#define AK_JOB_THREAD_BEGIN_DEFINE(name)  void name(ak_job_system* JobSystem, ak_thread* Thread, void* UserData)
#define AK_JOB_THREAD_UPDATE_DEFINE(name) void name(ak_job_system* JobSystem, ak_thread* Thread, void* UserData)
#define AK_JOB_THREAD_END_DEFINE(name)    void name(ak_job_system* JobSystem, ak_thread* Thread, void* UserData)

typedef AK_JOB_CALLBACK_DEFINE(ak_job_callback_func);
typedef AK_JOB_THREAD_BEGIN_DEFINE(ak_job_thread_begin_func);
typedef AK_JOB_THREAD_UPDATE_DEFINE(ak_job_thread_update_func);
typedef AK_JOB_THREAD_END_DEFINE(ak_job_thread_end_func);

typedef enum ak_job_bit_flag {
    AK_JOB_FLAG_NONE,
    AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT = (1 << 0)
} ak_job_bit_flag;
typedef uint32_t ak_job_flags;

typedef struct ak_job_data {
    ak_job_callback_func* JobCallback;
    void*                 Data;
    size_t                DataByteSize;
} ak_job_data;

typedef struct ak_job_thread_callbacks {
    ak_job_thread_begin_func*  JobThreadBegin;
    ak_job_thread_update_func* JobThreadUpdate;
    ak_job_thread_end_func*    JobThreadEnd;
    void*                      UserData;
} ak_job_thread_callbacks;

AKATOMICDEF ak_job_system* AK_Job_System_Create(uint32_t MaxJobCount, uint32_t ThreadCount, const ak_job_thread_callbacks* Callbacks, void* MallocUserData);
AKATOMICDEF void           AK_Job_System_Delete(ak_job_system* JobSystem);
AKATOMICDEF ak_job_id      AK_Job_System_Alloc_Job(ak_job_system* JobSystem, ak_job_data JobData, ak_job_id ParentID, ak_job_flags Flags);
AKATOMICDEF ak_job_id      AK_Job_System_Alloc_Empty_Job(ak_job_system* JobSystem);
AKATOMICDEF void           AK_Job_System_Free_Job(ak_job_system* JobSystem, ak_job_id JobID);
AKATOMICDEF void           AK_Job_System_Add_Job(ak_job_system* JobSystem, ak_job_id JobID);
AKATOMICDEF void           AK_Job_System_Wait_For_Job(ak_job_system* JobSystem, ak_job_id JobID);

#endif

#endif

#ifdef AK_ATOMIC_IMPLEMENTATION

/*Compiler specific functions (all other atomics are built ontop of these)*/

#if defined(AK_ATOMIC_MSVC_COMPILER)

AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
    /*Do a volatile load so that compiler doesn't duplicate loads, which makes
      them nonatomic.*/ 
    return ((volatile ak_atomic_u32*)Object)->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
    ((volatile ak_atomic_u32*)Object)->Nonatomic = Value;
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue) {
    return (uint32_t)_InterlockedExchange((volatile LONG*)Object, NewValue);
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    return _InterlockedCompareExchange((volatile LONG*)Object, NewValue, OldValue);
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue) {
    uint32_t Old = *OldValue;
    uint32_t Previous = (uint32_t)_InterlockedCompareExchange((volatile LONG*)Object, NewValue, Old);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand) {
    return _InterlockedExchangeAdd((volatile LONG*)Object, Operand);
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
    return _InterlockedIncrement((volatile LONG*)Object);
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
    return _InterlockedDecrement((volatile LONG*)Object);
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    /*Do a volatile load so that compiler doesn't duplicate loads, which makes
      them nonatomic.*/ 
    return ((volatile ak_atomic_u64*)Object)->Nonatomic;
#else
    /*Interlocked compare exchange is the most compatibile way to get an atomic 
      64 bit load on 32 bit x86*/
    return AK_Atomic_Compare_Exchange_U64_Relaxed((ak_atomic_u64*)Object, 0, 0);
#endif
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    ((volatile ak_atomic_u64*)Object)->Nonatomic = Value;
#else
    uint64_t Expected = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = _InterlockedCompareExchange64((LONGLONG*)Object, Value, Expected);
        if(Previous == Expected) break;
        Expected = Previous;
    }
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return _InterlockedExchange64((volatile LONGLONG*)Object, NewValue);
#else
    uint64_t Expected = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = _InterlockedCompareExchange64((volatile LONGLONG*)Object, NewValue, Expected);
        if(Previous == Expected) return Previous;
        Expected = Previous;
    }
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    return (uint64_t)_InterlockedCompareExchange64((volatile LONGLONG*)Object, NewValue, OldValue);
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = (uint64_t)_InterlockedCompareExchange64((volatile LONGLONG*)Object, NewValue, Old);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return _InterlockedExchangeAdd64((volatile LONGLONG*)Object, Operand);
#else
    uint64_t Expected = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = _InterlockedCompareExchange64((volatile LONGLONG*)Object, Expected+Operand, Expected);
        if(Previous == Expected) return Previous;
        Expected = Previous;
    }
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return _InterlockedIncrement64((volatile LONGLONG*)Object);
#else
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1)+1;
#endif
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
#if (AK_ATOMIC_PTR_SIZE == 8)
    return _InterlockedDecrement64((volatile LONGLONG*)Object);
#else
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
#endif
}

#elif defined(AK_ATOMIC_GCC_COMPILER) && defined(AK_ATOMIC_AARCH64_CPU)
AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue) {
    uint32_t Status;
    uint32_t Previous;
    __asm__ volatile(
        "1: ldxr %w0, %2\n"
        "   stxr %w1, %w3, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "r"(NewValue)
        : "cc");
    
    return Previous;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    uint32_t Status;
    uint32_t Previous;

    __asm__ volatile(
        "1: ldxr %w0, %2\n"
        "   cmp  %w0, %w3\n"
        "   b.ne 2f\n"
        "   stxr %w1, %w4, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "Ir" (OldValue), "r" (NewValue)
        : "cc");

    return Previous;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue) {
    uint32_t Old = *OldValue;
    uint32_t Previous = (uint32_t)AK_Atomic_Compare_Exchange_U32_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand) {
    uint32_t Status;
    uint32_t Previous, TempAddRegister;

    __asm__ volatile(
        "1: ldxr %w0, %2\n"
        "   add  %w3, %w0, %w4\n"
        "   stxr %w1, %w3, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic), "=&r" (TempAddRegister)
        : "Ir" (Operand)
        : "cc");

    return Previous;
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, 1)+1;
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, -1) - 1;
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    uint32_t Status;
    uint64_t Previous;
    __asm__ volatile(
        "1: ldxr %0, %2\n"
        "   stxr %w1, %3, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "r"(NewValue)
        : "cc");
    
    return Previous;
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    uint32_t Status;
    uint64_t Previous;

    __asm__ volatile(
        "1: ldxr %0, %2\n"
        "   cmp  %0, %3\n"
        "   b.ne 2f\n"
        "   stxr %w1, %4, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "Ir" (OldValue), "r" (NewValue)
        : "cc");

    return Previous;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = (uint64_t)AK_Atomic_Compare_Exchange_U64_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
    uint32_t Status;
    uint64_t Previous, TempAddRegister;

    __asm__ volatile(
        "1: ldxr %0, %2\n"
        "   add  %3, %0, %4\n"
        "   stxr %w1, %3, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic), "=&r" (TempAddRegister)
        : "Ir" (Operand)
        : "cc");

    return Previous;
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1)+1;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
}

#elif defined(AK_ATOMIC_GCC_COMPILER) && defined(AK_ATOMIC_ARM_CPU)

AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue) {
    uint32_t Status;
    uint32_t Previous;
    __asm__ volatile(
        "1: ldrex %0, [%3]\n"
        "   strex %1, %4, [%3]\n"
        "   cmp   %1, #0\n"
        "   bne   1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
        : "r"(Object), "r"(NewValue)
        : "cc");
    
    return Previous;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    size_t Status;
    uint32_t Previous;

    __asm__ volatile(
        "1: ldrex   %0, [%3]\n"
        "   mov     %1, #0\n"
        "   teq     %0, %4\n"
#ifdef AK_ATOMIC_ARM_THUMB
        "   it      eq\n"
#endif
        "   strexeq %1, %5, [%3]\n"
        "   cmp     %1, #0\n"
        "   bne     1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Qo"(Object->Nonatomic)
        : "r"(Object), "r" (OldValue), "r" (NewValue)
        : "cc");

    return Previous;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue) {
    uint32_t Old = *OldValue;
    uint32_t Previous = (uint32_t)AK_Atomic_Compare_Exchange_U32_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand) {
    uint32_t Status;
    uint32_t Previous, TempAddRegister;

    __asm__ volatile(
        "1: ldrex %0, [%4]\n" /*Stores the value of Object into the result*/
        "   mov   %3, %0\n" /*Stores the result into a temp register for addition*/
        "   add   %3, %5\n" /*Adds the temp register and operand and stores it into the temp register*/
        "   strex %1, %3, [%4]\n" /*Copy from the temp register to the final object*/
        "   cmp   %1, #0\n"
        "   bne   1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic), "=&r"(TempAddRegister)
        : "r"(Object), "Ir" (Operand)
        : "cc");

    return Previous;
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, 1)+1;
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, -1) - 1;
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
    uint64_t Result;
    __asm__ volatile(
        "1: ldrexd %0, %H0, [%1]"
        : "=&r"(Result)
        : "r"(Object), "Qo"(Object->Nonatomic)
    );
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
    uint64_t Status;
    __asm__ volatile(
        "1: ldrexd %0, %H0, [%2]\n"
        "   strexd %0, %3, %H3, [%2]\n"
        "   teq    %0, #0\n"
        "   bne    1b"
        : "=&r"(Status), "=Qo" (Object->Nonatomic)
        : "r"(Object), "r"(Value)
        : "cc"
    );
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    uint32_t Status;
    uint64_t Previous;
    __asm__ volatile(
        "1: ldrexd %0, %H0, [%3]\n"
        "   strexd %1, %4, %H4, [%3]\n"
        "   cmp    %1, #0\n"
        "   bne    1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Qo"(Object->Nonatomic)
        : "r"(Object), "r"(NewValue)
        : "cc");
    
    return Previous;
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    size_t Status;
    uint64_t Previous;

    __asm__ volatile(
        "1: ldrexd   %0, %H0, [%3]\n"
        "   mov      %1, #0\n"
        "   teq      %0, %4\n"
#ifdef AK_ATOMIC_ARM_THUMB
        "   it      eq\n"
#endif
        "   teqeq    %H0, %H4\n"
#ifdef AK_ATOMIC_ARM_THUMB
        "   it      eq\n"
#endif
        "   strexdeq %1, %5, %H5, [%3]\n"
        "   cmp      %1, #0\n"
        "   bne      1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Qo"(Object->Nonatomic)
        : "r"(Object), "r" (OldValue), "r" (NewValue)
        : "cc");

    return Previous;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = (uint64_t)AK_Atomic_Compare_Exchange_U64_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
    uint32_t Status;
    uint64_t Previous, TempAddRegister;

    __asm__ volatile(
        "1: ldrexd %0,  %H0, [%4]\n" /*Stores the value of Object into the result*/
        "   adds   %3,  %0,  %5\n"
        "   adc    %H3, %H0, %H5\n"
        "   strexd %1,  %3, %H3, [%4]\n" /*Copy from the temp register to the final object*/
        "   teq    %1,  #0\n"
        "   bne    1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Qo"(Object->Nonatomic), "=&r"(TempAddRegister)
        : "r"(Object), "r" (Operand)
        : "cc");

    return Previous;
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1)+1;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
}

#elif defined(AK_ATOMIC_GCC_COMPILER) && defined(AK_ATOMIC_X86_64_CPU)

AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U32_Relaxed(ak_atomic_u32* Object, uint32_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t NewValue) {
    /*
    "=r"(previous) chooses any general register, makes that %0, and outputs
    this register to previous after the
    block.
    "+m"(object->nonatomic) is the memory address that is read/written. This
    becomes %1.
    "0"(operand) puts operand into the same register as %0 before the block.
    volatile is required. Otherwise, if the return value (previous) is unused,
    the asm block
    No lock prefix is necessary for XCHG.
    */
    uint32_t Result;
    __asm__ volatile(
        "xchgl %0, %1"
        : "=r"(Result), "+m"(Object->Nonatomic)
        : "0"(NewValue)
    );
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    /*
    "=a"(previous) means the asm block outputs EAX to previous, because CMPXCHG
    puts the old value in EAX.
    "+m"(object->nonatomic) is the memory address that is read/written. This
    becomes %1.
    "q"(desired) puts desired into any of EBX, ECX or EDX before the block.
    This becomes %2.
    "0"(expected) puts expected in the same register as "=a"(previous), which
    is EAX, before the block.*/
    uint32_t Result;
    __asm__ volatile(
        "lock; cmpxchgl %2, %1"
        : "=a"(Result), "+m"(Object->Nonatomic)
        : "q"(NewValue), "0"(OldValue) 
    );
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue) {
    uint32_t Old = *OldValue;
    uint32_t Previous = (uint32_t)AK_Atomic_Compare_Exchange_U32_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32_Relaxed(ak_atomic_u32* Object, int32_t Operand) {
    /*
    See AK_Atomic_Exchange_U32_Relaxed for register constraint explanations
    Lock prefix is necessary of xaddl
    */
   uint32_t Result;
   __asm__ volatile(
        "lock; xaddl %0, %1"
        : "=r"(Result), "+m"(Object->Nonatomic)
        : "0"(Operand)
   );
   return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, 1)+1;    
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32_Relaxed(ak_atomic_u32* Object) {
    return AK_Atomic_Fetch_Add_U32_Relaxed(Object, -1)-1;    
}

#if AK_ATOMIC_PTR_SIZE == 8
AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
    return Object->Nonatomic;
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t Value) {
    Object->Nonatomic = Value;
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    uint64_t Result;
    __asm__ volatile(
        "xchgq %0, %1"
        : "=r"(Result), "+m"(Object->Nonatomic)
        : "0"(NewValue)
    );
    return Result;
}

AKATOMICDEF uint64_t  AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    uint64_t Result;
    __asm__ volatile(
        "lock; cmpxchgq %2, %1"
        : "=a"(Result), "+m"(Object->Nonatomic)
        : "q"(NewValue), "0"(OldValue)
    );
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = AK_Atomic_Compare_Exchange_U64_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
    uint64_t Result;
    __asm__ volatile(
        "lock; xaddq %0, %1"
        : "=r"(Result), "+m"(Object->Nonatomic)
        : "0"(Operand)
    );
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1) + 1;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
}

#else

AKATOMICDEF uint64_t AK_Atomic_Load_U64_Relaxed(const ak_atomic_u64* Object) {
    /* 
    On 32-bit x86, the most compatible way to get an atomic 64-bit load is with
    cmpxchg8b.
    "=&A"(previous) outputs EAX:EDX to previous after the block, while telling
    the compiler that these registers are clobbered before %1 is used, so don't 
    use EAX or EDX for %1.
    "m"(object->nonatomic) loads object's address into a register, which
    becomes %1, before the block.*/
    uint64_t Result;
    __asm__ volatile(
        "movl %%ebx, %%eax\n"
        "movl %%ecx, %%edx\n"
        "lock; cmpxchg8b %1"
        : "=&A"(Result)
        : "m"(Object->Nonatomic)
    );
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    /*
    On 32-bit x86, the most compatible way to get an atomic 64-bit store is
    with cmpxchg8b.
    Essentially, we perform turf_compareExchange64Relaxed(object, object->nonatomic, desired)
    in a loop until it returns the previous value.
    According to the Linux kernel (atomic64_cx8_32.S), we don't need the
    "lock;" prefix on cmpxchg8b since aligned 64-bit writes are already atomic 
    on 586 and newer.
    "=m"(object->nonatomic) loads object's address into a register, which
    becomes %0, before the block, and tells the compiler the variable at 
    address will be modified by the block.
    "b" and "c" move desired to ECX:EBX before the block.
    "A"(expected) loads the previous value of object->nonatomic into EAX:EDX
    before the block. */
    uint64_t OldValue = Object->Nonatomic;
    __asm__ volatile(
        "1: cmpxchg8b %0\n"
        "   jne 1b"
        : "=m"(Object->Nonatomic)
        : "b"((uint32_t)NewValue), "c"((uint32_t)(NewValue >> 32)), "A"(OldValue)
    );
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t NewValue) {
    uint64_t OldValue = Object->Nonatomic;
    for(;;) {
        uint64_t Previous = AK_Atomic_Compare_Exchange_U64_Relaxed(Object, OldValue, NewValue);
        if(Previous == OldValue)
            return OldValue;
        OldValue = Previous; 
    }
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    /*
    cmpxchg8b is the only way to do 64-bit RMW operations on 32-bit x86.
    "=A"(previous) outputs EAX:EDX to previous after the block.
    "+m"(object->nonatomic) is the memory address that is read/written. This
    becomes %1.
    "b" and "c" move desired to ECX:EBX before the block.
    "0"(expected) puts expected in the same registers as "=a"(previous), which
    are EAX:EDX, before the block.*/
    uint64_t Result;
    __asm__ volatile(
        "lock; cmpxchg8b %1"
        : "=A"(Result), "+m"(Object->Nonatomic)
        : "b"((uint32_t)NewValue), "c"((uint32_t)(NewValue >> 32)), "0"(OldValue)
    );
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue) {
    uint64_t Old = *OldValue;
    uint64_t Previous = AK_Atomic_Compare_Exchange_U64_Relaxed(Object, Old, NewValue);
    bool Result = (Previous == Old);
    if(!Result) *OldValue = Previous;
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64_Relaxed(ak_atomic_u64* Object, int64_t Operand) {
    for(;;) {
        uint64_t OldValue = Object->Nonatomic;
        if(AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(Object, OldValue, OldValue+Operand))
            return OldValue;
    }
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, 1) + 1;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64_Relaxed(ak_atomic_u64* Object) {
    return AK_Atomic_Fetch_Add_U64_Relaxed(Object, -1) - 1;
}
#endif

#else
#   error "Not Implemented"
#endif

/*Ptr type (is either 32 bit or 64 bit wrappers)*/
#if (AK_ATOMIC_PTR_SIZE == 8)
AKATOMICDEF void* AK_Atomic_Load_Ptr_Relaxed(const ak_atomic_ptr* Object) {
    return (void*)AK_Atomic_Load_U64_Relaxed((const ak_atomic_u64*)Object);
}

AKATOMICDEF void AK_Atomic_Store_Ptr_Relaxed(ak_atomic_ptr* Object, void* Value) {
    AK_Atomic_Store_U64_Relaxed((ak_atomic_u64*)Object, (uint64_t)Value);
}
 
AKATOMICDEF void* AK_Atomic_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* NewValue) {
    return (void*)AK_Atomic_Exchange_U64_Relaxed((ak_atomic_u64*)Object, (uint64_t)NewValue);
}

AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue) {
    return (void*)AK_Atomic_Compare_Exchange_U64_Relaxed((ak_atomic_u64*)Object, (uint64_t)OldValue, (uint64_t)NewValue);
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(ak_atomic_ptr* Object, void** OldValue, void* NewValue) {
    return AK_Atomic_Compare_Exchange_U64_Weak_Relaxed((ak_atomic_u64*)Object, (uint64_t*)OldValue, (uint64_t)NewValue);
}
#else
AKATOMICDEF void* AK_Atomic_Load_Ptr_Relaxed(const ak_atomic_ptr* Object) {
    return (void*)AK_Atomic_Load_U32_Relaxed((const ak_atomic_u32*)Object);
}

AKATOMICDEF void AK_Atomic_Store_Ptr_Relaxed(ak_atomic_ptr* Object, void* Value) {
    AK_Atomic_Store_U32_Relaxed((ak_atomic_u32*)Object, (uint32_t)Value);
}
 
AKATOMICDEF void* AK_Atomic_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* NewValue) {
    return (void*)AK_Atomic_Exchange_U32_Relaxed((ak_atomic_u32*)Object, (uint32_t)NewValue);
}

AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue) {
    return (void*)AK_Atomic_Compare_Exchange_U32_Relaxed((ak_atomic_u32*)Object, (uint32_t)OldValue, (uint32_t)NewValue);
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(ak_atomic_ptr* Object, void** OldValue, void* NewValue) {
    return AK_Atomic_Compare_Exchange_U32_Weak_Relaxed((ak_atomic_u32*)Object, (uint32_t*)OldValue, (uint32_t)NewValue);
}
#endif

/*Compare exchange for boolean results*/
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    return AK_Atomic_Compare_Exchange_U32_Relaxed(Object, OldValue, NewValue) == OldValue;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    return AK_Atomic_Compare_Exchange_U64_Relaxed(Object, OldValue, NewValue) == OldValue;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue) {
    return AK_Atomic_Compare_Exchange_Ptr_Relaxed(Object, OldValue, NewValue) == OldValue;
}

/*Atomic functions with memory order parameters*/
AKATOMICDEF uint32_t AK_Atomic_Load_U32(const ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder) {
    uint32_t Result = AK_Atomic_Load_U32_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    AK_Atomic_Store_U32_Relaxed(Object, Value);
}

AKATOMICDEF uint32_t AK_Atomic_Exchange_U32(ak_atomic_u32* Object, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Exchange_U32_Relaxed(Object, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Compare_Exchange_U32_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak_Explicit(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_U32_Weak_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32_Explicit(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32(ak_atomic_u32* Object, int32_t Operand, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Fetch_Add_U32_Relaxed(Object, Operand);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Increment_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Increment_U32_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint32_t AK_Atomic_Decrement_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint32_t Result = AK_Atomic_Decrement_U32_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Load_U64(const ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder) {
    uint64_t Result = AK_Atomic_Load_U64_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    AK_Atomic_Store_U64_Relaxed(Object, Value);
}

AKATOMICDEF uint64_t AK_Atomic_Exchange_U64(ak_atomic_u64* Object, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Exchange_U64_Relaxed(Object, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Compare_Exchange_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Compare_Exchange_U64_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
} 

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak_Explicit(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_U64_Weak_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64_Explicit(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Fetch_Add_U64(ak_atomic_u64* Object, int64_t Operand, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Fetch_Add_U64_Relaxed(Object, Operand);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Increment_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Increment_U64_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF uint64_t AK_Atomic_Decrement_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    uint64_t Result = AK_Atomic_Decrement_U64_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void* AK_Atomic_Load_Ptr(const ak_atomic_ptr* Object, ak_atomic_memory_order MemoryOrder) {
    void* Result = AK_Atomic_Load_Ptr_Relaxed(Object);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void AK_Atomic_Store_Ptr(ak_atomic_ptr* Object, void* Value, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    AK_Atomic_Store_Ptr_Relaxed(Object, Value);
}

AKATOMICDEF void* AK_Atomic_Exchange_Ptr(ak_atomic_ptr* Object, void* NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    void* Result = AK_Atomic_Exchange_Ptr_Relaxed(Object, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    void* Result = AK_Atomic_Compare_Exchange_Ptr_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Ptr_Weak(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Ptr_Weak_Explicit(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr_Explicit(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
    if((Success == AK_ATOMIC_MEMORY_ORDER_RELEASE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) ||
       (Failure == AK_ATOMIC_MEMORY_ORDER_RELEASE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL))
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(Object, OldValue, NewValue);
    if(Result) {
        if(Success == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Success == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    } else {
        if(Failure == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || Failure == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
            AK_Atomic_Thread_Fence_Acq();
    }
    return Result;
}

/*OS Thread primitives*/
#if defined(AK_ATOMIC_WIN32_OS)
AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void) {
    SYSTEM_INFO SystemInfo = {0};
    GetSystemInfo(&SystemInfo);
    return SystemInfo.dwNumberOfProcessors;
}

AKATOMICDEF void AK_Sleep(uint32_t Milliseconds) {
    Sleep(Milliseconds);
}

AKATOMICDEF uint64_t AK_Query_Performance_Counter(void) {
    uint64_t Result;
    QueryPerformanceCounter((LARGE_INTEGER*)&Result);
    return Result;
}

AKATOMICDEF uint64_t AK_Query_Performance_Frequency(void) {
    uint64_t Result;
    QueryPerformanceFrequency((LARGE_INTEGER*)&Result);
    return Result;  
}

static DWORD WINAPI AK_Thread__Internal_Proc(LPVOID Parameter) {
    ak_thread* Thread = (ak_thread*)Parameter;
    return Thread->Callback(Thread, Thread->UserData);
}

AKATOMICDEF bool AK_Thread_Create(ak_thread* Thread, ak_thread_callback_func* Callback, void* UserData) {
    Thread->Callback = Callback;
    Thread->UserData = UserData;
    Thread->Handle = CreateThread(NULL, 0, AK_Thread__Internal_Proc, Thread, 0, NULL);
    return Thread->Handle != NULL;
}

AKATOMICDEF void AK_Thread_Delete(ak_thread* Thread) {
    AK_Thread_Wait(Thread);
    if(Thread->Handle != NULL) {
        CloseHandle(Thread->Handle);
        Thread->Handle = NULL;
        Thread->Callback = NULL;
        Thread->UserData = NULL;
    }
}
AKATOMICDEF void AK_Thread_Wait(ak_thread* Thread) {
    if(Thread->Handle != NULL) {
        WaitForSingleObject(Thread->Handle, INFINITE);
    }
}

AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread) {
    if(Thread->Handle != NULL) {
        return GetThreadId(Thread->Handle);
    }
    return 0;
}

AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void) {
    return GetCurrentThreadId();
}

AKATOMICDEF bool AK_Mutex_Create(ak_mutex* Mutex) {
    InitializeCriticalSection(&Mutex->CriticalSection);
    return true;
}

AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex) {
    DeleteCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex) {
    LeaveCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex) {
    EnterCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF bool AK_Mutex_Try_Lock(ak_mutex* Mutex) {
    return TryEnterCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF bool AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount) {
    Semaphore->Handle = CreateSemaphoreA(NULL, InitialCount, 0x7fffffff, NULL);
    return Semaphore->Handle != NULL;
}

AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore) {
    CloseHandle(Semaphore->Handle);
}

AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore) {
    ReleaseSemaphore(Semaphore->Handle, 1, NULL);
}

AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore) {
    WaitForSingleObject(Semaphore->Handle, INFINITE);
}

AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Addend) {
    ReleaseSemaphore(Semaphore->Handle, Addend, NULL);
}

AKATOMICDEF bool AK_Condition_Variable_Create(ak_condition_variable* ConditionVariable) {
    InitializeConditionVariable(&ConditionVariable->Variable);
    return true;
}

AKATOMICDEF void AK_Condition_Variable_Delete(ak_condition_variable* ConditionVariable) {
    /*Noop on win32*/
}

AKATOMICDEF void AK_Condition_Variable_Wait(ak_condition_variable* ConditionVariable, ak_mutex* Mutex, 
                                            ak_condition_variable_predicate_func* PredicateFunc, void* UserData) {
    AK_Mutex_Lock(Mutex);
    while(!PredicateFunc(UserData)) {
        SleepConditionVariableCS(&ConditionVariable->Variable, &Mutex->CriticalSection, INFINITE);
    }
    AK_Mutex_Unlock(Mutex);
}

AKATOMICDEF void AK_Condition_Variable_Wake_One(ak_condition_variable* ConditionVariable) {
    WakeConditionVariable(&ConditionVariable->Variable);
}

AKATOMICDEF void AK_Condition_Variable_Wake_All(ak_condition_variable* ConditionVariable) {
    WakeAllConditionVariable(&ConditionVariable->Variable);
}

AKATOMICDEF bool AK_TLS_Create(ak_tls* TLS) {
    TLS->Index = TlsAlloc();
    return TLS->Index != TLS_OUT_OF_INDEXES;
}

AKATOMICDEF void AK_TLS_Delete(ak_tls* TLS) {
    TlsFree(TLS->Index);
}

AKATOMICDEF void* AK_TLS_Get(ak_tls* TLS) {
    return TlsGetValue(TLS->Index);
}
AKATOMICDEF void AK_TLS_Set(ak_tls* TLS, void* Data) {
    TlsSetValue(TLS->Index, Data);
}

#elif defined(AK_ATOMIC_POSIX_OS)

AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void) {
    return (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
}

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

static void* AK_Thread__Internal_Proc(void* Parameter) {
    ak_thread* Thread = (ak_thread*)Parameter;
    return (void*)(size_t)Thread->Callback(Thread, Thread->UserData);
}

AKATOMICDEF bool AK_Thread_Create(ak_thread* Thread, ak_thread_callback_func* Callback, void* UserData) {
    Thread->Callback = Callback;
    Thread->UserData = UserData;
    return pthread_create(&Thread->Thread, NULL, AK_Thread__Internal_Proc, Thread) == 0;
}

AKATOMICDEF void AK_Thread_Delete(ak_thread* Thread) {
    AK_Thread_Wait(Thread);
    Thread->Thread = 0;
    Thread->Callback = NULL;
    Thread->UserData = NULL;
}
AKATOMICDEF void AK_Thread_Wait(ak_thread* Thread) {
    if(Thread->Thread != 0) {
        pthread_join(Thread->Thread, NULL);
        Thread->Thread = 0;
    }
}

AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread) {
    if(Thread->Thread != 0) {
        return (uint64_t)Thread->Thread;
    }
    return 0;
}

AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void) {
    return (uint64_t)pthread_self();
}

AKATOMICDEF bool AK_Mutex_Create(ak_mutex* Mutex) {
    return pthread_mutex_init(&Mutex->Mutex, NULL) == 0;
}

AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex) {
    pthread_mutex_destroy(&Mutex->Mutex);
}

AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex) {
    pthread_mutex_unlock(&Mutex->Mutex);
}

AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex) {
    pthread_mutex_lock(&Mutex->Mutex);
}

AKATOMICDEF bool AK_Mutex_Try_Lock(ak_mutex* Mutex) {
    return pthread_mutex_trylock(&Mutex->Mutex);
}
#ifdef AK_ATOMIC_OSX_OS
AKATOMICDEF bool AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount) {
    return semaphore_create(mach_task_self(), &Semaphore->Semaphore, SYNC_POLICY_FIFO, 0) == KERN_SUCCESS;
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
    int32_t i;
    for(i = 0; i < Addend; i++)
        semaphore_signal(Semaphore->Semaphore);
}
#else
AKATOMICDEF bool AK_Semaphore_Create(ak_semaphore* Semaphore, int32_t InitialCount) {
    return sem_init(&Semaphore->Semaphore, 0, InitialCount) == 0;
}

AKATOMICDEF void AK_Semaphore_Delete(ak_semaphore* Semaphore) {
    sem_destroy(&Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Increment(ak_semaphore* Semaphore) {
    sem_post(&Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Decrement(ak_semaphore* Semaphore) {
    sem_wait(&Semaphore->Semaphore);
}

AKATOMICDEF void AK_Semaphore_Add(ak_semaphore* Semaphore, int32_t Addend) {
    int32_t i;
    for(i = 0; i < Addend; i++)
        sem_post(&Semaphore->Semaphore);
}
#endif

AKATOMICDEF bool AK_Condition_Variable_Create(ak_condition_variable* ConditionVariable) {
    return pthread_cond_init(&ConditionVariable->Variable, NULL) == 0;
}

AKATOMICDEF void AK_Condition_Variable_Delete(ak_condition_variable* ConditionVariable) {
    pthread_cond_destroy(&ConditionVariable->Variable);
}

AKATOMICDEF void AK_Condition_Variable_Wait(ak_condition_variable* ConditionVariable, ak_mutex* Mutex, 
                                            ak_condition_variable_predicate_func* PredicateFunc, void* UserData) {
    AK_Mutex_Lock(Mutex);
    while(!PredicateFunc(UserData)) {
        pthread_cond_wait(&ConditionVariable->Variable, &Mutex->Mutex);
    }
    AK_Mutex_Unlock(Mutex);
}

AKATOMICDEF void AK_Condition_Variable_Wake_One(ak_condition_variable* ConditionVariable) {
    pthread_cond_signal(&ConditionVariable->Variable);
}

AKATOMICDEF void AK_Condition_Variable_Wake_All(ak_condition_variable* ConditionVariable) {
    pthread_cond_broadcast(&ConditionVariable->Variable);
}

AKATOMICDEF bool AK_TLS_Create(ak_tls* TLS) {
    return pthread_key_create(&TLS->Key, NULL) == 0;
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

#else
#error "Not Implemented"
#endif

static bool AK_LW_Semaphore__Internal_Try(ak_lw_semaphore* Semaphore) {
    int32_t OldCount = (int32_t)AK_Atomic_Load_U32_Relaxed(&Semaphore->Count);
    while(OldCount > 0) {
        if(AK_Atomic_Compare_Exchange_U32_Weak_Explicit(&Semaphore->Count, (uint32_t*)&OldCount, OldCount-1, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            return true;
        }
    }
    return false;
}

static void AK_LW_Semaphore__Internal_Partial_Spin_Wait(ak_lw_semaphore* Semaphore) {
    int32_t OldCount;
    uint32_t SpinCount = 0;
    while(SpinCount++ < Semaphore->MaxSpinCount) {
        OldCount = (int32_t)AK_Atomic_Load_U32_Relaxed(&Semaphore->Count);
        if((OldCount > 0) && AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&Semaphore->Count, OldCount, OldCount-1, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED))
            return;
        AK_Atomic_Compiler_Fence_Acq(); /*Prevent compiler from collapsing the loop*/
    }
    OldCount = (int32_t)AK_Atomic_Fetch_Add_U32(&Semaphore->Count, -1, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    if(OldCount > 0) return;   
    AK_Semaphore_Decrement(&Semaphore->InternalSem);
    return;
}

AKATOMICDEF bool AK_LW_Semaphore_Create_With_Spin_Count(ak_lw_semaphore* Semaphore, int32_t InitialCount, uint32_t SpinCount) {
    AK_Atomic_Store_U32_Relaxed(&Semaphore->Count, InitialCount);
    Semaphore->MaxSpinCount = SpinCount;
    return AK_Semaphore_Create(&Semaphore->InternalSem, 0);
}

AKATOMICDEF bool AK_LW_Semaphore_Create(ak_lw_semaphore* Semaphore, int32_t InitialCount) {
    return AK_LW_Semaphore_Create_With_Spin_Count(Semaphore, InitialCount, 10000);
}

AKATOMICDEF void AK_LW_Semaphore_Delete(ak_lw_semaphore* Semaphore) {
    AK_Semaphore_Delete(&Semaphore->InternalSem);
}

AKATOMICDEF void AK_LW_Semaphore_Decrement(ak_lw_semaphore* Semaphore) {
    if(!AK_LW_Semaphore__Internal_Try(Semaphore)) {
        AK_LW_Semaphore__Internal_Partial_Spin_Wait(Semaphore);
    }
}

AKATOMICDEF void AK_LW_Semaphore_Increment(ak_lw_semaphore* Semaphore) {
    AK_LW_Semaphore_Add(Semaphore, 1);
}

AKATOMICDEF void AK_LW_Semaphore_Add(ak_lw_semaphore* Semaphore, int32_t Addend) {
    int32_t OldCount = (int32_t)AK_Atomic_Fetch_Add_U32(&Semaphore->Count, Addend, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    int32_t ToRelease = -OldCount < Addend ? -OldCount : Addend;
    if(ToRelease > 0) {
        AK_Semaphore_Add(&Semaphore->InternalSem, ToRelease);
    }
}

#ifndef AK_DISABLE_ASYNC_STACK_INDEX

#ifndef AK_ASYNC_STACK_INDEX_NO_STDIO
#include <stdio.h>
#endif

#if !defined(AK_ASYNC_STACK_INDEX_MALLOC)
#define AK_ASYNC_STACK_INDEX_MALLOC(size, user_data) ((void)(user_data), malloc(size))
#define AK_ASYNC_STACK_INDEX_FREE(ptr, user_data) ((void)(user_data), free(ptr))
#endif

#if !defined(AK_ASYNC_STACK_INDEX_MEMSET)
#define AK_ASYNC_STACK_INDEX_MEMSET(mem, index, size) memset(mem, index, size)
#endif

#if !defined(AK_ASYNC_STACK_INDEX_ASSERT)
#include <assert.h>
#define AK_ASYNC_STACK_INDEX_ASSERT(cond) assert(cond)
#endif

typedef union {
    uint64_t ID;
    struct {
        uint32_t Index;
        uint32_t Key;
    } KeyIndex;
} ak_async_stack_index32__key;

static ak_async_stack_index32__key AK_Async_Stack_Index32__Make_Key(uint32_t Index, uint32_t Key) {
    ak_async_stack_index32__key Result;
    Result.KeyIndex.Index = Index;
    Result.KeyIndex.Key = Key;
    return Result;
}

AKATOMICDEF void AK_Async_Stack_Index32_Init_Raw(ak_async_stack_index32* StackIndex, uint32_t* IndicesPtr, uint32_t Capacity) {
    /*Make sure atomic is properly aligned*/
    AK_ASYNC_STACK_INDEX_ASSERT((((size_t)&StackIndex->Head) % 8) == 0);
    StackIndex->NextIndices = IndicesPtr;
    StackIndex->Capacity = Capacity;
    ak_async_stack_index32__key* HeadKey = (ak_async_stack_index32__key*)&StackIndex->Head.Nonatomic; 
    HeadKey->KeyIndex.Index = AK_ASYNC_STACK_INDEX32_INVALID;
    HeadKey->KeyIndex.Key = 0;
}

AKATOMICDEF void AK_Async_Stack_Index32_Push_Sync(ak_async_stack_index32* StackIndex, uint32_t Index) {
    ak_async_stack_index32__key* CurrentTop = (ak_async_stack_index32__key*)&StackIndex->Head.Nonatomic;
    uint32_t Current = CurrentTop->KeyIndex.Index;
    StackIndex->NextIndices[Index] = Current;
    CurrentTop->KeyIndex.Index = Index;
}

AKATOMICDEF void AK_Async_Stack_Index32_Push(ak_async_stack_index32* StackIndex, uint32_t Index) {
    for(;;) {
        ak_async_stack_index32__key CurrentTop = { AK_Atomic_Load_U64_Relaxed(&StackIndex->Head)};
        uint32_t Current = CurrentTop.KeyIndex.Index;
        StackIndex->NextIndices[Index] = Current;
        ak_async_stack_index32__key NewTop = AK_Async_Stack_Index32__Make_Key(Index, CurrentTop.KeyIndex.Key+1); /*Increment key to avoid ABA problem*/
        /*Add job index to the freelist atomically*/
        if(AK_Atomic_Compare_Exchange_Bool_U64_Explicit(&StackIndex->Head, CurrentTop.ID, NewTop.ID, AK_ATOMIC_MEMORY_ORDER_RELEASE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            return;
        }
    }
}

AKATOMICDEF uint32_t AK_Async_Stack_Index32_Pop(ak_async_stack_index32* StackIndex) {
    for(;;) {
        ak_async_stack_index32__key CurrentTop = { AK_Atomic_Load_U64_Relaxed(&StackIndex->Head)};

        uint32_t Index = CurrentTop.KeyIndex.Index;
        if(Index == AK_ASYNC_STACK_INDEX32_INVALID) {
            /*No more jobs avaiable*/
            return AK_ASYNC_STACK_INDEX32_INVALID;
        }
        AK_ASYNC_STACK_INDEX_ASSERT(Index < StackIndex->Capacity); /*Overflow*/
        uint32_t Next = StackIndex->NextIndices[Index];
        ak_async_stack_index32__key NewTop = AK_Async_Stack_Index32__Make_Key(Next, CurrentTop.KeyIndex.Key+1); /*Increment key to avoid ABA problem*/
        /*Atomically update the job freelist*/
        if(AK_Atomic_Compare_Exchange_Bool_U64_Explicit(&StackIndex->Head, CurrentTop.ID, NewTop.ID, AK_ATOMIC_MEMORY_ORDER_ACQUIRE, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            return Index;
        }
    }
}

AKATOMICDEF bool AK_Async_Stack_Index32_Alloc(ak_async_stack_index32* StackIndex, uint32_t Capacity, void* AllocUserData) {
    size_t AllocationSize = Capacity*sizeof(uint32_t);
    void* Memory = AK_ASYNC_STACK_INDEX_MALLOC(AllocationSize, AllocUserData);
    if(!Memory) return false;

    AK_ASYNC_STACK_INDEX_MEMSET(Memory, 0, AllocationSize);
    AK_Async_Stack_Index32_Init_Raw(StackIndex, (uint32_t*)Memory, Capacity);
    StackIndex->AllocUserData = AllocUserData;
    return true;
}

AKATOMICDEF void AK_Async_Stack_Index32_Free(ak_async_stack_index32* StackIndex) {
    if(StackIndex->NextIndices) {
        AK_ASYNC_STACK_INDEX_FREE(StackIndex->NextIndices, StackIndex->AllocUserData);
    }
    AK_ASYNC_STACK_INDEX_MEMSET(StackIndex, 0, sizeof(ak_async_stack_index32));
}

#endif

#ifndef AK_DISABLE_ASYNC_SLOT_MAP

#ifndef AK_ASYNC_SLOT_MAP_NO_STDIO
#include <stdio.h>
#endif

#if !defined(AK_ASYNC_SLOT_MAP_MALLOC)
#define AK_ASYNC_SLOT_MAP_MALLOC(size, user_data) ((void)(user_data), malloc(size))
#define AK_ASYNC_SLOT_MAP_FREE(ptr, user_data) ((void)(user_data), free(ptr))
#endif

#if !defined(AK_ASYNC_SLOT_MAP_MEMSET)
#define AK_ASYNC_SLOT_MAP_MEMSET(mem, index, size) memset(mem, index, size)
#endif

#if !defined(AK_ASYNC_SLOT_MAP_ASSERT)
#include <assert.h>
#define AK_ASYNC_SLOT_MAP_ASSERT(cond) assert(cond)
#endif

AKATOMICDEF void AK_Async_Slot_Map64_Init_Raw(ak_async_slot_map64* SlotMap, uint32_t* IndicesPtr, ak_slot64* SlotsPtr, uint32_t Capacity) {
    AK_Async_Stack_Index32_Init_Raw(&SlotMap->FreeIndices, IndicesPtr, Capacity);
    SlotMap->Slots = (ak_slot64__internal*)SlotsPtr;

    uint32_t i; 
    for(i = 0; i < Capacity; i++) {
        /*Add all the entries to the freelist since every job is free to begin with. 
        This can be done synchronously without worrying about the stack key and aba problem */
        AK_Async_Stack_Index32_Push_Sync(&SlotMap->FreeIndices, i);

        SlotMap->Slots[i].Data.Index = i;
        SlotMap->Slots[i].Data.Generation.Nonatomic = 1;
    }
}

AKATOMICDEF bool AK_Async_Slot_Map64_Is_Allocated(ak_async_slot_map64* SlotMap, ak_slot64 Slot) {
    ak_slot64__internal SlotID = {Slot};
    AK_ASYNC_SLOT_MAP_ASSERT(SlotID.Data.Index < SlotMap->FreeIndices.Capacity); /*Overflow!*/
    return AK_Atomic_Load_U32_Relaxed(&SlotMap->Slots[SlotID.Data.Index].Data.Generation) == SlotID.Data.Generation.Nonatomic;
}

AKATOMICDEF ak_slot64 AK_Async_Slot_Map64_Alloc_Slot(ak_async_slot_map64* SlotMap) {
    uint32_t Index = AK_Async_Stack_Index32_Pop(&SlotMap->FreeIndices);
    if(Index == AK_ASYNC_STACK_INDEX32_INVALID) return 0;

    AK_ASYNC_SLOT_MAP_ASSERT(Index < SlotMap->FreeIndices.Capacity); /*Overflow*/
    ak_slot64__internal Result = SlotMap->Slots[Index];
    AK_ASYNC_SLOT_MAP_ASSERT(Index == Result.Data.Index); /*Index corruption!*/
    return Result.Slot;
}

AKATOMICDEF void AK_Async_Slot_Map64_Free_Slot(ak_async_slot_map64* SlotMap, ak_slot64 Slot) {
    ak_slot64__internal SlotID = {Slot};
    uint32_t NextGenerationIndex = SlotID.Data.Generation.Nonatomic+1;
    if(NextGenerationIndex == 0) NextGenerationIndex = 1; /*Generation cannot be 0*/
    AK_ASYNC_SLOT_MAP_ASSERT(SlotID.Data.Index < SlotMap->FreeIndices.Capacity); /*Overflow*/
    ak_slot64__internal* SlotMapSlotID = SlotMap->Slots + SlotID.Data.Index;
    AK_ASYNC_SLOT_MAP_ASSERT(SlotMapSlotID->Data.Index == SlotID.Data.Index);
    if(AK_Atomic_Compare_Exchange_Bool_U32(&SlotMapSlotID->Data.Generation, SlotID.Data.Generation.Nonatomic, NextGenerationIndex, AK_ATOMIC_MEMORY_ORDER_ACQ_REL)) {
        AK_Async_Stack_Index32_Push(&SlotMap->FreeIndices, SlotID.Data.Index);
    } else {
        AK_ASYNC_SLOT_MAP_ASSERT(false); /*Tried to delete an invalid slot*/
    }
}
AKATOMICDEF bool AK_Async_Slot_Map64_Alloc(ak_async_slot_map64* SlotMap, uint32_t Capacity, void* AllocUserData) {
    size_t AllocationSize = (sizeof(uint32_t)+sizeof(ak_slot64))*Capacity;
    void* Memory = AK_ASYNC_SLOT_MAP_MALLOC(AllocationSize, AllocUserData);
    if(!Memory) return false;

    uint32_t* IndicesPtr = (uint32_t*)Memory;
    ak_slot64* SlotsPtr = (ak_slot64*)(IndicesPtr+Capacity);
    AK_Async_Slot_Map64_Init_Raw(SlotMap, IndicesPtr, SlotsPtr, Capacity);
    SlotMap->AllocUserData = AllocUserData;
    return true;
}

AKATOMICDEF void AK_Async_Slot_Map64_Free(ak_async_slot_map64* SlotMap) {
    if(SlotMap->Slots) {
        AK_ASYNC_SLOT_MAP_FREE(SlotMap->FreeIndices.NextIndices, SlotMap->AllocUserData);
    }
    AK_ASYNC_SLOT_MAP_MEMSET(SlotMap, 0, sizeof(ak_async_slot_map64));
}

#endif

#ifndef AK_DISABLE_JOB_SYSTEM

#ifndef AK_JOB_SYSTEM_NO_STDIO
#include <stdio.h>
#endif

#if !defined(AK_JOB_SYSTEM_MALLOC)
#define AK_JOB_SYSTEM_MALLOC(size, user_data) ((void)(user_data), malloc(size))
#define AK_JOB_SYSTEM_FREE(ptr, user_data) ((void)(user_data), free(ptr))
#endif

#if !defined(AK_JOB_SYSTEM_MEMSET)
#define AK_JOB_SYSTEM_MEMSET(mem, index, size) memset(mem, index, size)
#endif

#if !defined(AK_JOB_SYSTEM_MEMCPY)
#define AK_JOB_SYSTEM_MEMCPY(mem, index, size) memcpy(mem, index, size)
#endif

#if !defined(AK_JOB_SYSTEM_ASSERT)
#include <assert.h>
#define AK_JOB_SYSTEM_ASSERT(cond) assert(cond)
#endif

typedef struct ak__job ak__job;

typedef struct ak_job_data__internal {
    void*   SlowData;
    uint8_t FastData[24];
} ak_job_data__internal;

enum {
    AK__JOB_INFO_FLAG_NONE,
    AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT = (1 << 0)
};

#define AK__INVALID_JOB_INDEX ((uint32_t)-1)
struct ak__job {
    uint32_t              Index;
    ak_atomic_u32         Generation;
    ak_job_callback_func* JobCallback;
    ak__job*              ParentJob;
    ak_atomic_u32         PendingJobs;
    uint8_t               UserData[AK_JOB_SYSTEM_FAST_USERDATA_SIZE];
    uint8_t               JobInfoFlags;
};

/*Highly recommend job size to not go past 64 bytes. Comment out this static 
  assertion if you need more space*/
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak__job) == 64);

typedef struct ak__job_thread ak__job_thread;

struct ak__job_thread {
    ak_thread     Thread;
    uint64_t      ThreadID;
    ak_atomic_u32 IsRunning;
};

typedef struct ak__job_queue {
    ak_atomic_u32  BottomIndex;
    ak_atomic_u32  TopIndex;
    ak__job**      Queue;
    uint64_t       ThreadID;
    ak_job_system* JobSystem;
    ak__job_thread JobThread;
} ak__job_queue;

typedef struct {
    ak_mutex        Lock;
    void*           MallocUserData;
    uint32_t        Capacity;
    uint32_t        Count;
    ak__job_queue** Queues;
} ak__job_queue_dynamic_array;

static void AK__Job_Queue_Dynamic_Array_Init(ak__job_queue_dynamic_array* Array, uint32_t Capacity, void* MallocUserData) {
    AK_Mutex_Create(&Array->Lock);
    Array->MallocUserData = MallocUserData;
    Array->Capacity = Capacity;
    Array->Count = 0;
    Array->Queues = AK_JOB_SYSTEM_MALLOC(Capacity*sizeof(ak__job_queue*), MallocUserData);
}

static void AK__Job_Queue_Dynamic_Array_Release(ak__job_queue_dynamic_array* Array) {
    if(Array) {
        AK_JOB_SYSTEM_FREE(Array->Queues, Array->MallocUserData);
        AK_Mutex_Delete(&Array->Lock);
    }
}

static void AK__Job_Queue_Dynamic_Array_Push(ak__job_queue_dynamic_array* Array, ak__job_queue* Queue) {
    /*This is a simple mutex to add threads to the thread array. Creating a thread is 
      already very expensive and called very infrequently that we can have basic lock 
      sync code*/
    AK_Mutex_Lock(&Array->Lock);
    if(Array->Count == Array->Capacity) {
        uint32_t NewCapacity = Array->Capacity*2;
        ak__job_queue** NewQueues = AK_JOB_SYSTEM_MALLOC(NewCapacity*sizeof(ak__job_queue*), Array->MallocUserData);
        AK_JOB_SYSTEM_ASSERT(NewQueues);
        AK_JOB_SYSTEM_MEMCPY(NewQueues, Array->Queues, Array->Capacity*sizeof(ak__job_queue*));
        AK_JOB_SYSTEM_FREE(Array->Queues, Array->MallocUserData);
        Array->Capacity = NewCapacity;
        Array->Queues = NewQueues;
    }
    Array->Queues[Array->Count++] = Queue;
    AK_Mutex_Unlock(&Array->Lock);
}

struct ak_job_system {
    /*System information*/
    void*                 MallocUserData;
    uint32_t              MaxJobCount;
    ak_tls                TLS;

    /*Job information*/
    ak_async_stack_index32 FreeJobIndices;
    ak__job*               Jobs; /*Array of max job count*/
    ak_lw_semaphore        JobSemaphore;

    /*Threads and queues*/
    ak_job_thread_callbacks      ThreadCallbacks;
    ak__job_queue_dynamic_array  QueueArray;    
    uint32_t                     ThreadCount;
};

ak__job_queue* AK_Job_System__Create_Queue(ak_job_system* JobSystem) {
    size_t AllocationSize = sizeof(ak__job_queue)+(sizeof(ak__job*)*JobSystem->MaxJobCount);
    ak__job_queue* JobQueue = (ak__job_queue*)AK_JOB_SYSTEM_MALLOC(AllocationSize, JobSystem->MallocUserData);
    AK_JOB_SYSTEM_ASSERT(JobQueue);
    if(!JobQueue) return NULL;
    AK_JOB_SYSTEM_MEMSET(JobQueue, 0, AllocationSize);
    JobQueue->Queue = (ak__job**)(JobQueue+1);
    AK__Job_Queue_Dynamic_Array_Push(&JobSystem->QueueArray, JobQueue);
    JobQueue->JobSystem = JobSystem;
    JobQueue->JobThread.ThreadID = (uint64_t)-1;
    return JobQueue;
}

void AK_Job_System__Delete_Queue(ak_job_system* JobSystem, ak__job_queue* JobQueue) {
    if(JobQueue->JobThread.ThreadID != (uint64_t)-1) {
        AK_Thread_Delete(&JobQueue->JobThread.Thread);
    }
    AK_JOB_SYSTEM_FREE(JobQueue, JobSystem->MallocUserData);
}

static AK_THREAD_CALLBACK_DEFINE(AK_Job_System__Thread_Callback);
bool AK_Job_System__Create_Thread(ak_job_system* JobSystem, ak__job_queue* JobQueue) {
    ak__job_thread* Thread = &JobQueue->JobThread;
    JobQueue->JobThread.IsRunning.Nonatomic = true;
    bool Result = AK_Thread_Create(&Thread->Thread, AK_Job_System__Thread_Callback, JobQueue);
    AK_JOB_SYSTEM_ASSERT(Result);
    if(Result) {
        JobQueue->JobThread.ThreadID = AK_Thread_Get_ID(&Thread->Thread);
    }
    JobSystem->ThreadCount++;
    return Result;
}

static ak__job_queue* AK_Job_System__Get_Local_Queue(ak_job_system* JobSystem) {
    ak__job_queue* JobQueue = (ak__job_queue*)AK_TLS_Get(&JobSystem->TLS);
    if(!JobQueue) {
        uint64_t ThreadID = AK_Thread_Get_Current_ID();
        uint32_t i;
        for(i = 0; i < JobSystem->QueueArray.Count; i++) {
            if(JobSystem->QueueArray.Queues[i]->ThreadID == ThreadID) {
                JobQueue = JobSystem->QueueArray.Queues[i];
                break;
            }
        }

        if(!JobQueue) {
            JobQueue = AK_Job_System__Create_Queue(JobSystem);
            AK_JOB_SYSTEM_ASSERT(JobQueue);
            JobQueue->JobThread.ThreadID = ThreadID;
        }


        if(JobQueue) AK_TLS_Set(&JobSystem->TLS, JobQueue);
    }
    return JobQueue;
}

static ak__job* AK_Job_System__Steal_Job(ak_job_system* JobSystem, ak__job_queue* JobQueue) {
    int32_t Top = (int32_t)AK_Atomic_Load_U32(&JobQueue->TopIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    /*Bottom needs to be read after top so an acquire barrier is sufficient (LoadLoad situation)*/
    int32_t Bottom = (int32_t)AK_Atomic_Load_U32(&JobQueue->BottomIndex, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);

    ak__job* Result = NULL;
    if(Top < Bottom) {
        Result = JobQueue->Queue[Top % JobSystem->MaxJobCount];
        if(!AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&JobQueue->TopIndex, Top, Top+1, AK_ATOMIC_MEMORY_ORDER_ACQ_REL, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
            Result = NULL;
        }
    } 
    return Result;
}

static ak__job* AK_Job_System__Pop_Job(ak_job_system* JobSystem, ak__job_queue* JobQueue) {
    int32_t Bottom = ((int32_t)AK_Atomic_Load_U32_Relaxed(&JobQueue->BottomIndex))-1;
    AK_Atomic_Store_U32_Relaxed(&JobQueue->BottomIndex, Bottom);
    /*We need to make sure Top is read before bottom thus this is a StoreLoad situation
      and needs an explicit memory barrier to handle this case */
    AK_Atomic_Thread_Fence_Seq_Cst();
    int32_t Top = (int32_t)AK_Atomic_Load_U32_Relaxed(&JobQueue->TopIndex);

    ak__job* Result = NULL;
    if(Top <= Bottom) {
        Result = JobQueue->Queue[Bottom % JobSystem->MaxJobCount];
        if(Top == Bottom) {
            if(!AK_Atomic_Compare_Exchange_Bool_U32_Explicit(&JobQueue->TopIndex, Top, Top+1, AK_ATOMIC_MEMORY_ORDER_ACQ_REL, AK_ATOMIC_MEMORY_ORDER_RELAXED)) {
                Result = NULL;
            }
            AK_Atomic_Store_U32_Relaxed(&JobQueue->BottomIndex, Bottom+1);
        }
    } else {
        AK_Atomic_Store_U32_Relaxed(&JobQueue->BottomIndex, Bottom+1);
    }
    return Result;
}

static void AK_Job_System__Push_Job(ak_job_system* JobSystem, ak__job_queue* JobQueue, ak__job* Job) {
    int32_t Bottom = (int32_t)AK_Atomic_Load_U32_Relaxed(&JobQueue->BottomIndex);
    JobQueue->Queue[Bottom % JobSystem->MaxJobCount] = Job;
    AK_Atomic_Store_U32(&JobQueue->BottomIndex, Bottom+1, AK_ATOMIC_MEMORY_ORDER_RELEASE);
}

static size_t AK_Job_System__Get_Queue_Size(ak__job_queue* JobQueue) {
    return AK_Atomic_Load_U32_Relaxed(&JobQueue->BottomIndex)-AK_Atomic_Load_U32_Relaxed(&JobQueue->TopIndex);
}

static ak__job_queue* AK_Job_System__Get_Largest_Job_Queue(ak_job_system* JobSystem, ak__job_queue* CurrentQueue) {
    size_t BestSize = 0;
    ak__job_queue* BestQueue = NULL;
    
    uint32_t i;
    for(i = 0; i < JobSystem->QueueArray.Count; i++) {
        ak__job_queue* Queue = JobSystem->QueueArray.Queues[i];
        if(Queue != CurrentQueue) {
            size_t QueueSize = AK_Job_System__Get_Queue_Size(Queue); 
            if(QueueSize > BestSize) {
                BestSize = QueueSize;
                BestQueue = Queue;
            }
        }
    }

    return BestQueue;
}

static void AK__Job_Queue_Add_Job(ak_job_system* JobSystem, ak__job_queue* JobQueue, ak__job* Job) {
    AK_Job_System__Push_Job(JobSystem, JobQueue, Job);
    AK_LW_Semaphore_Increment(&JobSystem->JobSemaphore);
}

static ak__job* AK_Job_System__Get_Next_Job(ak_job_system* JobSystem, ak__job_queue* JobQueue) {
    ak__job* Job = AK_Job_System__Pop_Job(JobSystem, JobQueue);

    if(!Job) {
        ak__job_queue* StolenJobQueue = AK_Job_System__Get_Largest_Job_Queue(JobSystem, JobQueue);
        if(StolenJobQueue) {
            Job = AK_Job_System__Steal_Job(JobSystem, StolenJobQueue);
        }
    }
    return Job;   
}

static ak_job_id AK__Job_Make_ID(ak__job* Job) {
    ak_job_id JobID = (uint64_t)(Job->Index) | ((uint64_t)AK_Atomic_Load_U32_Relaxed(&Job->Generation) << 32);
    return JobID;
}

static void AK_Job_System__Allocate_Data(ak__job* Job, void* Bytes, size_t ByteSize, void* AllocUserData) {
    void* Data;
    if(ByteSize <= sizeof(Job->UserData)) {
        Data = &Job->UserData;
        Job->JobInfoFlags &= ~AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT;
    } else {
        void* SlowData = AK_JOB_SYSTEM_MALLOC(ByteSize, AllocUserData);
        AK_JOB_SYSTEM_ASSERT(SlowData);

        /*Copy pointer address to user data*/
        AK_JOB_SYSTEM_MEMCPY(Job->UserData, &SlowData, sizeof(void*));
        Data = SlowData;
        Job->JobInfoFlags |= AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT;
    }
    AK_JOB_SYSTEM_MEMCPY(Data, Bytes, ByteSize);
}

#define AK_Job_System__Get_User_Data_Heap(user_data) (void*)(*(size_t*)(user_data))

static void AK_Job_System__Free_Data(ak__job* Job, void* AllocUserData) {
    if(Job->JobInfoFlags & AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT) {
        void* Ptr = AK_Job_System__Get_User_Data_Heap(Job->UserData);
        AK_JOB_SYSTEM_FREE(Ptr, AllocUserData);
    }
}

static void AK_Job_System__Finish_Job(ak_job_system* JobSystem, ak__job* Job) {
    if(AK_Atomic_Decrement_U32_Relaxed(&Job->PendingJobs) == 0) {

        if(Job->ParentJob) {
            AK_Job_System__Finish_Job(JobSystem, Job->ParentJob);
        }

        AK_Job_System__Free_Data(Job, JobSystem->MallocUserData);
        ak_job_id JobID = AK__Job_Make_ID(Job);
        AK_Job_System_Free_Job(JobSystem, JobID);
    }
}

static bool AK_Job_System__Process_Next_Job(ak_job_system* JobSystem, ak__job_queue* JobQueue) {
    ak__job* Job = AK_Job_System__Get_Next_Job(JobSystem, JobQueue);
    if(Job) {
        ak_job_id JobID = AK__Job_Make_ID(Job);
        ak_job_status JobStatus = AK_JOB_STATUS_COMPLETE;
        if(Job->JobCallback) {
            void* UserData = 
                ((Job->JobInfoFlags & AK__JOB_INFO_FLAG_HEAP_ALLOCATED_BIT) ? 
                  AK_Job_System__Get_User_Data_Heap(Job->UserData) : 
                  (void*)Job->UserData);
            JobStatus = Job->JobCallback(JobSystem, JobID, UserData);
        }
        switch(JobStatus) {
            case AK_JOB_STATUS_COMPLETE: {
                AK_Job_System__Finish_Job(JobSystem, Job);
            } break;

            case AK_JOB_STATUS_REQUEUE: {
                AK__Job_Queue_Add_Job(JobSystem, JobQueue, Job);
            } break;
        }
        return true;
    }
    return false;
}

static void AK_Job_Queue__Thread_Run(ak__job_queue* JobQueue) {
    ak_job_system* JobSystem = JobQueue->JobSystem;
    ak_job_thread_callbacks* ThreadCallbacks = &JobSystem->ThreadCallbacks;
    ak__job_thread* JobThread = &JobQueue->JobThread;

    if(ThreadCallbacks->JobThreadBegin)
        ThreadCallbacks->JobThreadBegin(JobSystem, &JobThread->Thread, ThreadCallbacks->UserData);
    
    if(ThreadCallbacks->JobThreadUpdate)
            ThreadCallbacks->JobThreadUpdate(JobSystem, &JobThread->Thread, ThreadCallbacks->UserData);

    while(AK_Atomic_Load_U32_Relaxed(&JobThread->IsRunning)) {
        if(!AK_Job_System__Process_Next_Job(JobSystem, JobQueue)) {
            AK_LW_Semaphore_Decrement(&JobSystem->JobSemaphore);
        }

        if(ThreadCallbacks->JobThreadUpdate)
            ThreadCallbacks->JobThreadUpdate(JobSystem, &JobThread->Thread, ThreadCallbacks->UserData);
    }
    

    if(ThreadCallbacks->JobThreadEnd)
        ThreadCallbacks->JobThreadEnd(JobSystem, &JobThread->Thread, ThreadCallbacks->UserData);
}

static ak__job* AK_Job_System__Get_Job(ak_job_system* JobSystem, ak_job_id JobID) {
    uint32_t Index = (uint32_t)JobID;
    uint32_t Generation = (uint32_t)(JobID >> 32);
    AK_JOB_SYSTEM_ASSERT(Index < JobSystem->MaxJobCount);
    ak__job* Job = JobSystem->Jobs + Index;
    if(AK_Atomic_Load_U32_Relaxed(&Job->Generation) == Generation) {
        return Job;
    }
    return NULL;
}

static AK_THREAD_CALLBACK_DEFINE(AK_Job_System__Thread_Callback) {
    ak__job_queue* JobQueue = (ak__job_queue*)UserData;
    AK_Job_Queue__Thread_Run(JobQueue);
    return 0;
}

AKATOMICDEF ak_job_system* AK_Job_System_Create(uint32_t MaxJobCount, uint32_t ThreadCount, const ak_job_thread_callbacks* ThreadCallbacks, void* MallocUserData) {
    /*Get the allocation size. Want to avoid many small allocations so batch them together*/
    size_t AllocationSize = sizeof(ak_job_system); /*Space for the job system*/ 
    AllocationSize += MaxJobCount*sizeof(uint32_t); /*Space for the job free indices*/
    AllocationSize += MaxJobCount*sizeof(ak__job); /*Space for the jobs*/

    /*Allocate and clear all the memory*/
    ak_job_system* JobSystem = (ak_job_system*)AK_JOB_SYSTEM_MALLOC(AllocationSize, MallocUserData);
    AK_JOB_SYSTEM_ASSERT(JobSystem);

    if(!JobSystem) return NULL;

    AK_JOB_SYSTEM_MEMSET(JobSystem, 0, AllocationSize);

    /*System information*/
    JobSystem->MallocUserData    = MallocUserData;
    JobSystem->MaxJobCount       = MaxJobCount;
    AK_TLS_Create(&JobSystem->TLS);
    AK_LW_Semaphore_Create(&JobSystem->JobSemaphore, 0);

    /*Job information*/
    uint32_t* FreeJobIndices = (uint32_t*)(JobSystem+1);
    AK_Async_Stack_Index32_Init_Raw(&JobSystem->FreeJobIndices, FreeJobIndices, MaxJobCount);
    JobSystem->Jobs = (ak__job*)(FreeJobIndices+MaxJobCount);

    uint32_t i;
    for(i = 0; i < MaxJobCount; i++) {
        /*Add all the entries to the freelist since every job is free to begin with. 
          This can be done synchronously without worrying about the stack key and aba problem */
        AK_Async_Stack_Index32_Push_Sync(&JobSystem->FreeJobIndices, i);

        /*Set index and generation for the job. Generation of 0 is not valid*/
        JobSystem->Jobs[i].Index = i;
        JobSystem->Jobs[i].Generation.Nonatomic = 1;
        AK_JOB_SYSTEM_ASSERT((((size_t)&JobSystem->Jobs[i].PendingJobs) % 4) == 0);
        AK_JOB_SYSTEM_ASSERT((((size_t)&JobSystem->Jobs[i].Generation) % 4) == 0);
    }
    
    /*Thread information*/
    if(ThreadCallbacks) {
        JobSystem->ThreadCallbacks = *ThreadCallbacks;
    }

    AK__Job_Queue_Dynamic_Array_Init(&JobSystem->QueueArray, ThreadCount, JobSystem->MallocUserData);
    for(i = 0; i < ThreadCount; i++) {
        ak__job_queue* JobQueue = AK_Job_System__Create_Queue(JobSystem);
        AK_JOB_SYSTEM_ASSERT(JobQueue);
        bool CreationResult = AK_Job_System__Create_Thread(JobSystem, JobQueue);
        AK_JOB_SYSTEM_ASSERT(CreationResult);
    }

    /*Validate that our memory is correct*/

    return JobSystem;
}

AKATOMICDEF void AK_Job_System_Delete(ak_job_system* JobSystem) {
        
    uint32_t i;
    for(i = 0; i < JobSystem->QueueArray.Count; i++) {
        ak__job_queue* JobQueue = JobSystem->QueueArray.Queues[i];
        if(JobQueue->JobThread.ThreadID != (uint64_t)-1) {
            ak__job_thread* Thread = &JobQueue->JobThread;
            AK_Atomic_Store_U32(&Thread->IsRunning, false, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        }
    }
    
    /*Set all the threads status to IsRunning false before we wake the condition variable*/
    AK_LW_Semaphore_Add(&JobSystem->JobSemaphore, JobSystem->ThreadCount);

    for(i = 0; i < JobSystem->QueueArray.Count; i++) {
        AK_Job_System__Delete_Queue(JobSystem, JobSystem->QueueArray.Queues[i]);
    }
    AK__Job_Queue_Dynamic_Array_Release(&JobSystem->QueueArray);
    AK_LW_Semaphore_Delete(&JobSystem->JobSemaphore);
    AK_TLS_Delete(&JobSystem->TLS);
    AK_JOB_SYSTEM_FREE(JobSystem, JobSystem->MallocUserData);
}

AKATOMICDEF ak_job_id AK_Job_System_Alloc_Job(ak_job_system* JobSystem, ak_job_data JobData, ak_job_id ParentID, ak_job_flags Flags) {
    /*First need to find a free job index atomically*/
    uint32_t FreeIndex = AK_Async_Stack_Index32_Pop(&JobSystem->FreeJobIndices);
    if(FreeIndex == AK_ASYNC_STACK_INDEX32_INVALID) {
        /*No more jobs avaiable*/
        return 0;
    }

    ak__job* Job = JobSystem->Jobs+FreeIndex;
    AK_JOB_SYSTEM_ASSERT(Job->Index == FreeIndex); /*Invalid indices!*/

    Job->JobCallback = JobData.JobCallback;
    AK_Job_System__Allocate_Data(Job, JobData.Data, JobData.DataByteSize, JobSystem->MallocUserData);
    Job->ParentJob = AK_Job_System__Get_Job(JobSystem, ParentID);

    /*Pending jobs should be 0 for the job*/    
    AK_JOB_SYSTEM_ASSERT(AK_Atomic_Load_U32(&Job->PendingJobs, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 0);
    AK_Atomic_Increment_U32(&Job->PendingJobs, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    if(Job->ParentJob) {
        AK_Atomic_Increment_U32(&Job->ParentJob->PendingJobs, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
    }

    ak_job_id JobID = AK__Job_Make_ID(Job);
    if(Flags & AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT) {
        AK_Job_System_Add_Job(JobSystem, JobID);
    }
    return JobID;
}

AKATOMICDEF ak_job_id AK_Job_System_Alloc_Empty_Job(ak_job_system* JobSystem) {
    ak_job_data JobData = {0};
    return AK_Job_System_Alloc_Job(JobSystem, JobData, 0, 0);
}

AKATOMICDEF void AK_Job_System_Free_Job(ak_job_system* JobSystem, ak_job_id JobID) {
    uint32_t Index = (uint32_t)(JobID);
    AK_JOB_SYSTEM_ASSERT(Index < JobSystem->MaxJobCount); /*Overflow*/
    ak__job* Job = JobSystem->Jobs + Index;
    uint32_t Generation = (uint32_t)(JobID >> 32);
    uint32_t NextGenerationIndex = Generation+1;
    if(NextGenerationIndex == 0) NextGenerationIndex = 1;
    AK_Atomic_Store_U32(&Job->Generation, NextGenerationIndex, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    AK_Atomic_Thread_Fence_Seq_Cst();
    AK_Async_Stack_Index32_Push(&JobSystem->FreeJobIndices, Index);
}

AKATOMICDEF void AK_Job_System_Add_Job(ak_job_system* JobSystem, ak_job_id JobID) {
    ak__job_queue* JobQueue = AK_Job_System__Get_Local_Queue(JobSystem);
    ak__job* Job = AK_Job_System__Get_Job(JobSystem, JobID);
    if(Job) {
        AK__Job_Queue_Add_Job(JobSystem, JobQueue, Job);
    }
}

AKATOMICDEF void AK_Job_System_Wait_For_Job(ak_job_system* JobSystem, ak_job_id JobID) {
    while(AK_Job_System__Get_Job(JobSystem, JobID)) {
        ak__job_queue* JobQueue = AK_Job_System__Get_Local_Queue(JobSystem);
        AK_Job_System__Process_Next_Job(JobSystem, JobQueue);
    }
}

#endif

#endif