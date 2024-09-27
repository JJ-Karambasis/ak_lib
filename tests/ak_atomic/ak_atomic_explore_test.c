#define AK_ATOMIC_EXCLUDE_STDINT
#include "ak_atomic_test_header.h"

#include <stdatomic.h>
#include <stdio.h>

int main(void) {
    ak_atomic_u8 A;
    AK_Atomic_Store_U8(&A, 5, AK_ATOMIC_MEMORY_ORDER_RELAXED);

    //uint32_t ATest = AK_Atomic_Increment_U8(&A, AK_ATOMIC_MEMORY_ORDER_RELAXED);
    uint32_t ATest2 = AK_Atomic_Load_U8(&A, AK_ATOMIC_MEMORY_ORDER_RELAXED);
    printf("%d\n", ATest2);

    // atomic_uint B;
    // atomic_store_explicit(&B, 5, memory_order_relaxed);

    // uint32_t BTest = 5;
    // atomic_compare_exchange_strong_explicit(&B, &BTest, 10, memory_order_relaxed, memory_order_relaxed);


    // ak_atomic_u8 Test2;
    // AK_Atomic_Store_U8(&Test2, 5, AK_ATOMIC_MEMORY_ORDER_RELAXED);
    // uint32_t A2 = AK_Atomic_Exchange_U8_Relaxed(&Test2, 20);
    // char B2 = AK_Atomic_Load_U8(&Test2, AK_ATOMIC_MEMORY_ORDER_RELAXED);


	return 0;
}

#ifdef AK_ATOMIC_COMPILER_MSVC
#pragma warning(pop)
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#ifndef AK_ATOMIC_IMPLEMENTATION
#define AK_ATOMIC_IMPLEMENTATION
#include <ak_atomic.h>
#endif
