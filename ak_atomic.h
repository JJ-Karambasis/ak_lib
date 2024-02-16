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
#   if defined(__aarch64__)
#       define AK_ATOMIC_AARCH64_CPU
#       define AK_ATOMIC_PTR_SIZE 8
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

//If stdint is not included for your platform, define AK_ATOMIC_CUSTOM_TYPES
//and define the stdint types:
//uint32_t -> 32 bit unsigned integer
//int32_t -> 32 bit signed integer
//uint64_t -> 64 bit unsigned integer
//int64_t -> 64 bit signed integer
//bool -> 8 bit signed integer
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

#elif defined(AK_ATOMIC_GCC_COMPILER) && defined(AK_ATOMIC_AARCH64_CPU)

//Atomic operators on this architecture need to be aligned properly

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

#define AK_Atomic_Thread_Fence_Acq() __asm__ volatile("dmb ish" ::: "memory")
#define AK_Atomic_Thread_Fence_Rel() __asm__ volatile("dmb ish" ::: "memory")
#define AK_Atomic_Thread_Fence_Seq_Cst() __asm__ volatile("dmb ish" ::: "memory")

#else
#error "Not Implemented"
#endif

AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u32) == 4);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_u64) == 8);
AK_ATOMIC__COMPILE_TIME_ASSERT(sizeof(ak_atomic_ptr) == AK_ATOMIC_PTR_SIZE);

//Compiler specific functions (all other atomics are built ontop of these)
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

//Ptr type (is either 32 bit or 64 bit wrappers)
AKATOMICDEF void* AK_Atomic_Load_Ptr_Relaxed(const ak_atomic_ptr* Object);
AKATOMICDEF void  AK_Atomic_Store_Ptr_Relaxed(ak_atomic_ptr* Object, void* Value);
AKATOMICDEF void* AK_Atomic_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* NewValue);
AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Ptr_Weak_Relaxed(ak_atomic_ptr* Object, void** OldValue, void* NewValue);

//Compare exchange for boolean results
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue);
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue);
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue);

typedef enum ak_atomic_memory_order {
    AK_ATOMIC_MEMORY_ORDER_RELAXED,
    AK_ATOMIC_MEMORY_ORDER_ACQUIRE,
    AK_ATOMIC_MEMORY_ORDER_RELEASE,
    AK_ATOMIC_MEMORY_ORDER_ACQ_REL
} ak_atomic_memory_order;

//Atomic functions with memory order parameters
AKATOMICDEF uint32_t AK_Atomic_Load_U32(const ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void     AK_Atomic_Store_U32(ak_atomic_u32* Object, uint32_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Exchange_U32(ak_atomic_u32* Object, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Compare_Exchange_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_U32_Weak(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF bool     AK_Atomic_Compare_Exchange_Bool_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Fetch_Add_U32(ak_atomic_u32* Object, int32_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Increment_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint32_t AK_Atomic_Decrement_U32(ak_atomic_u32* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF uint64_t  AK_Atomic_Load_U64(const ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void      AK_Atomic_Store_U64(ak_atomic_u64* Object, uint64_t Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Exchange_U64(ak_atomic_u64* Object, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Compare_Exchange_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_U64_Weak(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF bool      AK_Atomic_Compare_Exchange_Bool_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Fetch_Add_U64(ak_atomic_u64* Object, int64_t Operand, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Increment_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF uint64_t  AK_Atomic_Decrement_U64(ak_atomic_u64* Object, ak_atomic_memory_order MemoryOrder);

AKATOMICDEF void* AK_Atomic_Load_Ptr(const ak_atomic_ptr* Object, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void  AK_Atomic_Store_Ptr(ak_atomic_ptr* Object, void* Value, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void* AK_Atomic_Exchange_Ptr(ak_atomic_ptr* Object, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF void* AK_Atomic_Compare_Exchange_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Ptr_Weak(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure);
AKATOMICDEF bool  AK_Atomic_Compare_Exchange_Bool_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder);

//Thread primitives
typedef struct ak_thread ak_thread;
typedef int32_t ak_thread_callback(ak_thread* Thread, void* UserData);

#if defined(AK_ATOMIC_WIN32_OS)
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

struct ak_thread {
    HANDLE              Handle;
    ak_thread_callback* Callback;
    void*               UserData;
};

typedef struct ak_mutex {
    CRITICAL_SECTION CriticalSection;
} ak_mutex;

#elif defined(AK_ATOMIC_POSIX_OS)

#include <pthread.h>

struct ak_thread {
    pthread_t Thread;
    ak_thread_callback* Callback;
    void*               UserData;
};

typedef struct ak_mutex {
    pthread_mutexattr_t Attribute;
    pthread_mutex_t Mutex;
} ak_mutex;

#else
#error "Not Implemented"
#endif

AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void);
AKATOMICDEF void     AK_Sleep(uint32_t Milliseconds);

AKATOMICDEF bool     AK_Thread_Create(ak_thread* Thread, ak_thread_callback* Callback, void* UserData);
AKATOMICDEF void     AK_Thread_Delete(ak_thread* Thread);
AKATOMICDEF void     AK_Thread_Wait(ak_thread* Thread);
AKATOMICDEF uint64_t AK_Thread_Get_ID(ak_thread* Thread);
AKATOMICDEF uint64_t AK_Thread_Get_Current_ID(void);

AKATOMICDEF bool AK_Mutex_Create(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex);
AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex);
AKATOMICDEF bool AK_Mutex_Try_Lock(ak_mutex* Mutex);

#endif

#ifdef AK_ATOMIC_IMPLEMENTATION

//Compiler specific functions (all other atomics are built ontop of these)

#if defined(AK_ATOMIC_MSVC_COMPILER)

AKATOMICDEF uint32_t AK_Atomic_Load_U32_Relaxed(const ak_atomic_u32* Object) {
    //Do a volatile load so that compiler doesn't duplicate loads, which makes
    //them nonatomic. 
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
    //Do a volatile load so that compiler doesn't duplicate loads, which makes
    //them nonatomic. 
    return ((volatile ak_atomic_u64*)Object)->Nonatomic;
#else
    //Interlocked compare exchange is the most compatibile way to get an atomic 
    //64 bit load on 32 bit x86
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
        "   cmp  %w1, #0\n"
        "   b.ne 1b\n"
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
    uint32_t Previous;

    __asm__ volatile(
        "1: ldxr %w0, %2\n"
        "   add  %w0, %w0, %w3\n"
        "   stxr %w1, %w0, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
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
        "   cmp  %w1, #0\n"
        "   b.ne 1b\n"
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
    uint64_t Previous;

    __asm__ volatile(
        "1: ldxr %0, %2\n"
        "   add  %0, %0, %3\n"
        "   stxr %w1, %0, %2\n"
        "   cbnz %w1, 1b\n"
        "2:"
        : "=&r" (Previous), "=&r" (Status), "+Q"(Object->Nonatomic)
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

#else
#   error "Not Implemented"
#endif

//Ptr type (is either 32 bit or 64 bit wrappers)
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

//Compare exchange for boolean results
AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue) {
    return AK_Atomic_Compare_Exchange_U32_Relaxed(Object, OldValue, NewValue) == OldValue;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue) {
    return AK_Atomic_Compare_Exchange_U64_Relaxed(Object, OldValue, NewValue) == OldValue;
}

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(ak_atomic_ptr* Object, void* OldValue, void* NewValue) {
    return AK_Atomic_Compare_Exchange_Ptr_Relaxed(Object, OldValue, NewValue) == OldValue;
}

//Atomic functions with memory order parameters
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

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U32_Weak(ak_atomic_u32* Object, uint32_t* OldValue, uint32_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
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

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U32(ak_atomic_u32* Object, uint32_t OldValue, uint32_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
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

AKATOMICDEF bool AK_Atomic_Compare_Exchange_U64_Weak(ak_atomic_u64* Object, uint64_t* OldValue, uint64_t NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
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

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_U64(ak_atomic_u64* Object, uint64_t OldValue, uint64_t NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
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

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Ptr_Weak(ak_atomic_ptr* Object, void** OldValue, void* NewValue, ak_atomic_memory_order Success, ak_atomic_memory_order Failure) {
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

AKATOMICDEF bool AK_Atomic_Compare_Exchange_Bool_Ptr(ak_atomic_ptr* Object, void* OldValue, void* NewValue, ak_atomic_memory_order MemoryOrder) {
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_RELEASE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL) 
        AK_Atomic_Thread_Fence_Rel();
    bool Result = AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(Object, OldValue, NewValue);
    if(MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQUIRE || MemoryOrder == AK_ATOMIC_MEMORY_ORDER_ACQ_REL)
        AK_Atomic_Thread_Fence_Acq();
    return Result;
}

//OS Thread primitives
#if defined(AK_ATOMIC_WIN32_OS)
AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void) {
    SYSTEM_INFO SystemInfo = {0};
    GetSystemInfo(&SystemInfo);
    return SystemInfo.dwNumberOfProcessors;
}

AKATOMICDEF void AK_Sleep(uint32_t Milliseconds) {
    Sleep(Milliseconds);
}

static DWORD WINAPI AK_Thread__Internal_Proc(LPVOID Parameter) {
    ak_thread* Thread = (ak_thread*)Parameter;
    return Thread->Callback(Thread, Thread->UserData);
}

AKATOMICDEF bool AK_Thread_Create(ak_thread* Thread, ak_thread_callback* Callback, void* UserData) {
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
    EnterCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex) {
    LeaveCriticalSection(&Mutex->CriticalSection);
}

AKATOMICDEF bool AK_Mutex_Try_Lock(ak_mutex* Mutex) {
    return TryEnterCriticalSection(&Mutex->CriticalSection);
}
#elif defined(AK_ATOMIC_POSIX_OS)

AKATOMICDEF uint32_t AK_Get_Processor_Thread_Count(void) {
    return (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
}

AKATOMICDEF void AK_Sleep(uint32_t Milliseconds) {
    sleep(Milliseconds);
}

static void* AK_Thread__Internal_Proc(void* Parameter) {
    ak_thread* Thread = (ak_thread*)Parameter;
    return (void*)(size_t)Thread->Callback(Thread, Thread->UserData);
}

AKATOMICDEF bool AK_Thread_Create(ak_thread* Thread, ak_thread_callback* Callback, void* UserData) {
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
    pthread_mutexattr_init(&Mutex->Attribute);
    pthread_mutexattr_settype(&Mutex->Attribute, PTHREAD_MUTEX_NORMAL);
    return pthread_mutex_init(&Mutex->Mutex, &Mutex->Attribute) == 0;
}

AKATOMICDEF void AK_Mutex_Delete(ak_mutex* Mutex) {
    pthread_mutexattr_destroy(&Mutex->Attribute);
    pthread_mutex_destroy(&Mutex->Mutex);
}

AKATOMICDEF void AK_Mutex_Unlock(ak_mutex* Mutex) {
    pthread_mutex_lock(&Mutex->Mutex);
}

AKATOMICDEF void AK_Mutex_Lock(ak_mutex* Mutex) {
    pthread_mutex_unlock(&Mutex->Mutex);
}

AKATOMICDEF bool AK_Mutex_Try_Lock(ak_mutex* Mutex) {
    return pthread_mutex_trylock(&Mutex->Mutex);
}
#else
#error "Not Implemented"
#endif

#endif