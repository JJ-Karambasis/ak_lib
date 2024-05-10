/*We need to compile in c89 to make sure our library can support that 
  version (best language version for porting) but utest.h requires 
  some c99 stdio/stdlib values
*/
#define _ISOC99_SOURCE 
#include <ak_atomic.h>
#include <assert.h>
#include "utest.h"

int32_t AK_Atomic_Test_Random_Between(int32_t Min, int32_t Max) {
    int32_t n = Max - Min + 1;
    int32_t remainder = RAND_MAX % n;
    int32_t x;
    do{
        x = rand();
    } while (x >= RAND_MAX - remainder);
    return Min + x % n;
}

typedef struct {
    ak_atomic_u32 X;
    ak_atomic_u32 Y;
    uint32_t      R1;
    uint32_t      R2;
} ak_atomic_store_load_test_data_u32; 

typedef struct {
    ak_atomic_store_load_test_data_u32* Data;
    uint32_t Index;
} ak_atomic_store_load_test_thread_data_u32;

int32_t AK_Atomic_Store_And_Load_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_store_load_test_thread_data_u32* ThreadData = (ak_atomic_store_load_test_thread_data_u32*)UserData;
    ak_atomic_store_load_test_data_u32* TestData = ThreadData->Data;

    if(ThreadData->Index == 0) {
        while(rand() % 8 != 0) { } /*random delay*/
        AK_Atomic_Store_U32_Relaxed(&TestData->X, 1);
        AK_Atomic_Thread_Fence_Seq_Cst();
        TestData->R1 = AK_Atomic_Load_U32_Relaxed(&TestData->Y);
    } else {
        while(rand() % 8 != 0) { } /*random delay*/
        AK_Atomic_Store_U32_Relaxed(&TestData->Y, 1);
        AK_Atomic_Thread_Fence_Seq_Cst();
        TestData->R2 = AK_Atomic_Load_U32_Relaxed(&TestData->X);
    }

    return 0;
}

/*
For this test we want to make sure loads and stores are done in certain
orders via barriers. Before jobs are ran, 0 is filled in for X and Y 
In the above job we assign
*/
UTEST(AK_Atomic, StoreAndLoad32) {
    const uint32_t Iterations = 1000;

    uint32_t i;
    for(i = 0; i < Iterations; i++) {
        ak_atomic_store_load_test_data_u32 Data = {0};

        ak_atomic_store_load_test_thread_data_u32 ThreadData1 = {0};
        ThreadData1.Data  = &Data;
        ThreadData1.Index = 0;

        ak_atomic_store_load_test_thread_data_u32 ThreadData2 = {0};
        ThreadData2.Data  = &Data;
        ThreadData2.Index = 1;

        ak_thread Thread1, Thread2;
        ASSERT_TRUE(AK_Thread_Create(&Thread1, AK_Atomic_Store_And_Load_Thread_Callback_U32, &ThreadData1));
        ASSERT_TRUE(AK_Thread_Create(&Thread2, AK_Atomic_Store_And_Load_Thread_Callback_U32, &ThreadData2));

        AK_Thread_Wait(&Thread1);
        AK_Thread_Wait(&Thread2);

        ASSERT_FALSE(Data.R1 == 0 && Data.R2 == 0);

        AK_Thread_Delete(&Thread1);
        AK_Thread_Delete(&Thread2);
    }
}

typedef struct {
    ak_atomic_u64 X;
    ak_atomic_u64 Y;
    uint64_t      R1;
    uint64_t      R2;
} ak_atomic_store_load_test_data_u64; 

typedef struct {
    ak_atomic_store_load_test_data_u64* Data;
    uint32_t Index;
} ak_atomic_store_load_test_thread_data_u64;

int32_t AK_Atomic_Store_And_Load_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_store_load_test_thread_data_u64* ThreadData = (ak_atomic_store_load_test_thread_data_u64*)UserData;
    ak_atomic_store_load_test_data_u64* TestData = ThreadData->Data;

    if(ThreadData->Index == 0) {
        while(rand() % 8 != 0) { } /*random delay*/
        AK_Atomic_Store_U64_Relaxed(&TestData->X, 1);
        AK_Atomic_Thread_Fence_Seq_Cst();
        TestData->R1 = AK_Atomic_Load_U64_Relaxed(&TestData->Y);
    } else {
        while(rand() % 8 != 0) { } /*random delay*/
        AK_Atomic_Store_U64_Relaxed(&TestData->Y, 1);
        AK_Atomic_Thread_Fence_Seq_Cst();
        TestData->R2 = AK_Atomic_Load_U64_Relaxed(&TestData->X);
    }

    return 0;
}

/*
For this test we want to make sure loads and stores are done in certain
orders via barriers. Before jobs are ran, 0 is filled in for X and Y 
In the above job we assign
Only case where x86 can reorder instructions is during store then loads
*/
UTEST(AK_Atomic, StoreAndLoad64) {
    const uint32_t Iterations = 1000;

    uint32_t i;
    for(i = 0; i < Iterations; i++) {
        ak_atomic_store_load_test_data_u64 Data = {0};

        ak_atomic_store_load_test_thread_data_u64 ThreadData1 = {0};
        ThreadData1.Data  = &Data;
        ThreadData1.Index = 0;

        ak_atomic_store_load_test_thread_data_u64 ThreadData2 = {0};
        ThreadData2.Data  = &Data;
        ThreadData2.Index = 1;

        ak_thread Thread1, Thread2;
        ASSERT_TRUE(AK_Thread_Create(&Thread1, AK_Atomic_Store_And_Load_Thread_Callback_U64, &ThreadData1));
        ASSERT_TRUE(AK_Thread_Create(&Thread2, AK_Atomic_Store_And_Load_Thread_Callback_U64, &ThreadData2));

        AK_Thread_Wait(&Thread1);
        AK_Thread_Wait(&Thread2);

        ASSERT_FALSE(Data.R1 == 0 && Data.R2 == 0);

        AK_Thread_Delete(&Thread1);
        AK_Thread_Delete(&Thread2);
    }
}

typedef struct {
    ak_atomic_u32 SharedInt;
    bool          Success;
} ak_atomic_load_store_test_data_u32; 

typedef struct {
    ak_atomic_load_store_test_data_u32* Data;
    uint32_t  Index;
    uint32_t* Values;
    uint32_t  Limit;
    uint32_t  Iterations;
} ak_atomic_load_store_test_thread_data_u32;

int32_t AK_Atomic_Load_And_Store_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_load_store_test_thread_data_u32* ThreadData = (ak_atomic_load_store_test_thread_data_u32*)UserData;
    ak_atomic_load_store_test_data_u32* TestData = ThreadData->Data;

    /*
    The processor (even on x86) can delay the stores until after
    the loads without the processor fence. Compiler fence is not enough
    */
    if(ThreadData->Index == 0) {
        uint32_t Index = 0;

        uint32_t i;
        for(i = 0; i < ThreadData->Iterations; i++) {
            Index = (Index*Index+1) % 65521; /*Make it a little random*/
            uint32_t Value = ThreadData->Values[Index & 7];
            AK_Atomic_Store_U32_Relaxed(&TestData->SharedInt, Value);
        }
    } else {
        uint32_t i;
        for(i = 0; i < ThreadData->Iterations; i++) {
            uint32_t Value = AK_Atomic_Load_U32_Relaxed(&TestData->SharedInt);
            if((Value*Value) < ThreadData->Limit) {
                TestData->Success = false;
            }
        }
    }

    return 0;
}

UTEST(AK_Atomic, LoadAndStore32) {
    static const uint32_t Iterations = 100000000;
    static uint32_t LoadStoreValues[] = {
        0x37b91364u,
        0x1c5970efu,
        0x536c76bau,
        0x0a10207fu,
        0x71043c77u,
        0x4db84a83u,
        0x27cf0273u,
        0x74a15a69u,
    };
    static const uint32_t Limit = 0xffffff00u;

    ak_atomic_load_store_test_data_u32 TestData = {0};
    TestData.SharedInt.Nonatomic = LoadStoreValues[0];
    TestData.Success = true;

    ak_atomic_load_store_test_thread_data_u32 ThreadData1 = {0};
    ThreadData1.Data = &TestData;
    ThreadData1.Index = 0;
    ThreadData1.Values = LoadStoreValues;
    ThreadData1.Iterations = Iterations;
    ThreadData1.Limit = Limit;

    ak_atomic_load_store_test_thread_data_u32 ThreadData2 = {0};
    ThreadData2.Data = &TestData;
    ThreadData2.Index = 1;
    ThreadData2.Values = LoadStoreValues;
    ThreadData2.Iterations = Iterations;
    ThreadData2.Limit = Limit;

    ak_thread Thread1, Thread2;
    ASSERT_TRUE(AK_Thread_Create(&Thread1, AK_Atomic_Load_And_Store_Thread_Callback_U32, &ThreadData1));
    ASSERT_TRUE(AK_Thread_Create(&Thread2, AK_Atomic_Load_And_Store_Thread_Callback_U32, &ThreadData2));

    AK_Thread_Wait(&Thread1);
    AK_Thread_Wait(&Thread2);

    ASSERT_TRUE(TestData.Success);

    AK_Thread_Delete(&Thread1);
    AK_Thread_Delete(&Thread2);
}

typedef struct {
    ak_atomic_u64 SharedInt;
    bool          Success;
} ak_atomic_load_store_test_data_u64; 

typedef struct {
    ak_atomic_load_store_test_data_u64* Data;
    uint64_t  Index;
    uint64_t* Values;
    uint64_t  Limit;
    uint64_t  Iterations;
} ak_atomic_load_store_test_thread_data_u64;

int32_t AK_Atomic_Load_And_Store_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_load_store_test_thread_data_u64* ThreadData = (ak_atomic_load_store_test_thread_data_u64*)UserData;
    ak_atomic_load_store_test_data_u64* TestData = ThreadData->Data;

    /*
    The processor (even on x86) can delay the stores until after
    the loads without the processor fence. Compiler fence is not enough
    */
    if(ThreadData->Index == 0) {
        uint32_t Index = 0;

        uint32_t i;
        for(i = 0; i < ThreadData->Iterations; i++) {
            Index = (Index*Index+1) % 65521; /*Make it a little random*/
            uint64_t Value = ThreadData->Values[Index & 7];
            AK_Atomic_Store_U64_Relaxed(&TestData->SharedInt, Value);
        }
    } else {
        uint32_t i;
        for(i = 0; i < ThreadData->Iterations; i++) {
            uint64_t Value = AK_Atomic_Load_U64_Relaxed(&TestData->SharedInt);
            if((Value*Value) < ThreadData->Limit) {
                TestData->Success = false;
            }
        }
    }

    return 0;
}

UTEST(AK_Atomic, LoadAndStore64) {
    static const uint32_t Iterations = 100000000;
    static uint64_t LoadStoreValues[] = {
        0x3c116d2362a21633ull,
        0x7747508552ab6bc6ull,
        0x289a0e1528a43422ull,
        0x15e36d0a61d326eaull,
        0x3ccb2e8c0c6224c4ull,
        0x074504c13a1716e1ull,
        0x6c82417a3ad77b24ull,
        0x3124440040454919ull,
    };
    static const uint64_t Limit = 0xffffff0000000000ull;

    ak_atomic_load_store_test_data_u64 TestData = {0};
    TestData.SharedInt.Nonatomic = LoadStoreValues[0];
    TestData.Success = true;

    ak_atomic_load_store_test_thread_data_u64 ThreadData1 = {0};
    ThreadData1.Data = &TestData;
    ThreadData1.Index = 0;
    ThreadData1.Values = LoadStoreValues;
    ThreadData1.Iterations = Iterations;
    ThreadData1.Limit = Limit;

    ak_atomic_load_store_test_thread_data_u64 ThreadData2 = {0};
    ThreadData2.Data = &TestData;
    ThreadData2.Index = 1;
    ThreadData2.Values = LoadStoreValues;
    ThreadData2.Iterations = Iterations;
    ThreadData2.Limit = Limit;

    ak_thread Thread1, Thread2;
    ASSERT_TRUE(AK_Thread_Create(&Thread1, AK_Atomic_Load_And_Store_Thread_Callback_U64, &ThreadData1));
    ASSERT_TRUE(AK_Thread_Create(&Thread2, AK_Atomic_Load_And_Store_Thread_Callback_U64, &ThreadData2));

    AK_Thread_Wait(&Thread1);
    AK_Thread_Wait(&Thread2);

    ASSERT_TRUE(TestData.Success);

    AK_Thread_Delete(&Thread1);
    AK_Thread_Delete(&Thread2);
}

typedef struct ak_atomic_exchange_test_data_u32 {
    uint32_t*     ValuesSeen;
    uint32_t      ValuesPerThread;
    ak_atomic_u32 SharedInt;
} ak_atomic_exchange_test_data_u32;

typedef struct {
    ak_atomic_exchange_test_data_u32* Data;
    uint32_t                          ThreadIndex;
} ak_atomic_exchange_thread_data_u32;

int32_t AK_Test_Compare_U32(const void* A, const void* B) {
    uint32_t AValue = *((const uint32_t*)A);
    uint32_t BValue = *((const uint32_t*)B);
    return (int32_t)BValue - (int32_t)AValue;
}

int32_t AK_Exchange_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_exchange_thread_data_u32* ThreadData = (ak_atomic_exchange_thread_data_u32*)UserData;
    uint32_t Base = ThreadData->Data->ValuesPerThread*ThreadData->ThreadIndex;
    uint32_t i;
    for(i = Base+1; i <= Base+ThreadData->Data->ValuesPerThread; i++) {
        uint32_t OldValue = AK_Atomic_Exchange_U32_Relaxed(&ThreadData->Data->SharedInt, i);
        ThreadData->Data->ValuesSeen[i] = OldValue;
    }

    return 0;
}

UTEST(AK_Atomic, Exchange32) {
    static const uint32_t ValuesPerThread = 5000000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();
    uint32_t NumValues = (ValuesPerThread*NumThreads)+1;
    uint32_t* ValuesSeen = malloc(sizeof(uint32_t)*NumValues);
    ak_atomic_exchange_thread_data_u32* ThreadsData = malloc(sizeof(ak_atomic_exchange_thread_data_u32)*NumThreads);
    memset(ValuesSeen, 0, sizeof(uint32_t)*NumValues);

    ak_atomic_exchange_test_data_u32 TestData = {0};
    TestData.ValuesSeen = ValuesSeen;
    TestData.ValuesPerThread = ValuesPerThread;

    ak_thread* Threads = malloc(sizeof(ak_thread)*NumThreads);
    
    uint32_t ThreadIndex;
    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        ak_atomic_exchange_thread_data_u32* ThreadData = ThreadsData + ThreadIndex;
        ThreadData->Data = &TestData;
        ThreadData->ThreadIndex = ThreadIndex;
        AK_Thread_Create(&Threads[ThreadIndex], AK_Exchange_Thread_Callback_U32, ThreadData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ValuesSeen[0] = TestData.SharedInt.Nonatomic;
    qsort(ValuesSeen, NumValues, sizeof(uint32_t), AK_Test_Compare_U32);
    uint32_t* Expected = (uint32_t*)malloc(sizeof(uint32_t)*NumValues);
    uint32_t i;
    for(i = 0; i < NumValues; i++) {
        Expected[i] = i;
    }
    qsort(Expected, NumValues, sizeof(uint32_t), AK_Test_Compare_U32);
    bool Success = memcmp(ValuesSeen, Expected, sizeof(uint32_t)*NumValues) == 0;

    ASSERT_TRUE(Success);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
    }

    free(ThreadsData);
    free(Expected);
    free(Threads);
    free(ValuesSeen);
}

typedef struct ak_atomic_exchange_test_data_u64 {
    uint64_t*     ValuesSeen;
    uint64_t      ValuesPerThread;
    ak_atomic_u64 SharedInt;
} ak_atomic_exchange_test_data_u64;

typedef struct {
    ak_atomic_exchange_test_data_u64* Data;
    uint64_t                          ThreadIndex;
} ak_atomic_exchange_thread_data_u64;

int32_t AK_Test_Compare_U64(const void* A, const void* B) {
    uint64_t AValue = *((const uint64_t*)A);
    uint64_t BValue = *((const uint64_t*)B);
    return (int32_t)BValue-(int32_t)AValue;
}

int32_t AK_Exchange_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_exchange_thread_data_u64* ThreadData = (ak_atomic_exchange_thread_data_u64*)UserData;
    uint64_t Base = ThreadData->Data->ValuesPerThread*ThreadData->ThreadIndex;
    uint64_t i;
    for(i = Base+1; i <= Base+ThreadData->Data->ValuesPerThread; i++) {
        uint64_t OldValue = AK_Atomic_Exchange_U64_Relaxed(&ThreadData->Data->SharedInt, i);
        ThreadData->Data->ValuesSeen[i] = OldValue;
    }

    return 0;
}

UTEST(AK_Atomic, Exchange64) {
    static const uint64_t ValuesPerThread = 5000000;
    uint64_t NumThreads = AK_Get_Processor_Thread_Count();
    uint64_t NumValues = (ValuesPerThread*NumThreads)+1;
    uint64_t* ValuesSeen = malloc(sizeof(uint64_t)*NumValues);
    ak_atomic_exchange_thread_data_u64* ThreadsData = malloc(sizeof(ak_atomic_exchange_thread_data_u64)*NumThreads);
    memset(ValuesSeen, 0, sizeof(uint64_t)*NumValues);

    ak_atomic_exchange_test_data_u64 TestData = {0};
    TestData.ValuesSeen = ValuesSeen;
    TestData.ValuesPerThread = ValuesPerThread;

    ak_thread* Threads = malloc(sizeof(ak_thread)*NumThreads);
    uint64_t ThreadIndex;
    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        ak_atomic_exchange_thread_data_u64* ThreadData = ThreadsData + ThreadIndex;
        ThreadData->Data = &TestData;
        ThreadData->ThreadIndex = ThreadIndex;
        AK_Thread_Create(&Threads[ThreadIndex], AK_Exchange_Thread_Callback_U64, ThreadData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ValuesSeen[0] = TestData.SharedInt.Nonatomic;
    qsort(ValuesSeen, NumValues, sizeof(uint64_t), AK_Test_Compare_U64);
    uint64_t* Expected = (uint64_t*)malloc(sizeof(uint64_t)*NumValues);
    uint64_t i;
    for(i = 0; i < NumValues; i++) {
        Expected[i] = i;
    }
    qsort(Expected, NumValues, sizeof(uint64_t), AK_Test_Compare_U64);
    bool Success = memcmp(ValuesSeen, Expected, sizeof(uint64_t)*NumValues) == 0;

    ASSERT_TRUE(Success);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
    }

    free(ThreadsData);
    free(Expected);
    free(Threads);
    free(ValuesSeen);
}

typedef struct ak_atomic_compare_exchange_thread_data_u32 {
    uint32_t Iterations;
    ak_atomic_u32 Flag;
    uint32_t SharedValue;
    uint32_t Padding;
} ak_atomic_compare_exchange_thread_data_u32;

int32_t AK_Compare_Exchange_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_compare_exchange_thread_data_u32* ThreadData = (ak_atomic_compare_exchange_thread_data_u32*)UserData;
    uint32_t Count = 0;
    while(Count < ThreadData->Iterations) {
        if(AK_Atomic_Compare_Exchange_Bool_U32(&ThreadData->Flag, 0, 1, AK_ATOMIC_MEMORY_ORDER_ACQ_REL)) {
            ThreadData->SharedValue++;
            Count++;
            AK_Atomic_Store_U32(&ThreadData->Flag, 0, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        }
    }
    return 0;
}

UTEST(AK_Atomic, CompareExchange32) {
    static const uint32_t Iterations = 5000000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();
    ak_atomic_compare_exchange_thread_data_u32 ThreadData = {0};
    ThreadData.Iterations = Iterations;
    
    ak_thread* Threads = (ak_thread*)malloc(sizeof(ak_thread)*NumThreads);
    memset(Threads, 0, sizeof(ak_thread)*NumThreads);

    uint32_t i;
    for(i = 0; i < NumThreads; i++) {
        AK_Thread_Create(&Threads[i], AK_Compare_Exchange_Thread_Callback_U32, &ThreadData);
    }

    for(i = 0; i < NumThreads; i++) {
        AK_Thread_Wait(&Threads[i]);
    }

    ASSERT_TRUE(ThreadData.SharedValue == Iterations*NumThreads);

    for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(&Threads[i]);
    }

    free(Threads);
}

typedef struct {
    uint32_t Iterations;
    ak_atomic_u64 Flag;
    uint64_t SharedValue;
} ak_atomic_compare_exchange_thread_data_u64;

int32_t AK_Compare_Exchange_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_compare_exchange_thread_data_u64* ThreadData = (ak_atomic_compare_exchange_thread_data_u64*)UserData;
    uint32_t Count = 0;
    while(Count < ThreadData->Iterations) {
        if(AK_Atomic_Compare_Exchange_Bool_U64(&ThreadData->Flag, 0, 1, AK_ATOMIC_MEMORY_ORDER_ACQ_REL)) {
            ThreadData->SharedValue++;
            Count++;
            AK_Atomic_Store_U64(&ThreadData->Flag, 0, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        }
    }
    return 0;
}

UTEST(AK_Atomic, CompareExchange64) {
    static const uint32_t Iterations = 5000000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();
    ak_atomic_compare_exchange_thread_data_u64 ThreadData = {0};
    ThreadData.Iterations = Iterations;

    ak_thread* Threads = (ak_thread*)malloc(sizeof(ak_thread)*NumThreads);
    memset(Threads, 0, sizeof(ak_thread)*NumThreads);

    uint32_t i;
    for(i = 0; i < NumThreads; i++) {
        AK_Thread_Create(&Threads[i], AK_Compare_Exchange_Thread_Callback_U64, &ThreadData);
    }

    for(i = 0; i < NumThreads; i++) {
        AK_Thread_Wait(&Threads[i]);
    }

    ASSERT_TRUE(ThreadData.SharedValue == Iterations*NumThreads);

    for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(&Threads[i]);
    }

    free(Threads);
}

typedef struct {
    uint32_t      Iterations;
    ak_atomic_u32 SharedInt;
} ak_atomic_increment_test_data_u32;

int32_t AK_Increment_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u32* TestData = (ak_atomic_increment_test_data_u32*)UserData;
    uint32_t i = 0;
    for(i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Increment_U32_Relaxed(&TestData->SharedInt);
    }
    return 0;
}

int32_t AK_Decrement_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u32* TestData = (ak_atomic_increment_test_data_u32*)UserData;
    uint32_t i;
    for(i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Decrement_U32_Relaxed(&TestData->SharedInt);
    }
    return 0;
}

int32_t AK_Fetch_Add_Increment_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u32* TestData = (ak_atomic_increment_test_data_u32*)UserData;
    uint32_t i;
    for(i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Fetch_Add_U32_Relaxed(&TestData->SharedInt, 1);
    }
    return 0;
}

int32_t AK_Fetch_Add_Decrement_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u32* TestData = (ak_atomic_increment_test_data_u32*)UserData;
    uint32_t i;
    for(i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Fetch_Add_U32_Relaxed(&TestData->SharedInt, -1);
    }
    return 0;
}

UTEST(AK_Atomic, Increment32) {
    const uint32_t Iterations = 5000000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();

    ak_atomic_increment_test_data_u32 TestData = {0};
    TestData.Iterations = Iterations;

    ak_thread* Threads = (ak_thread*)malloc(sizeof(ak_thread)*NumThreads);
    uint32_t ThreadIndex;
    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Create(&Threads[ThreadIndex], AK_Increment_Thread_Callback_U32, &TestData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == Iterations*NumThreads);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Decrement_Thread_Callback_U32, &TestData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == 0);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Fetch_Add_Increment_Thread_Callback_U32, &TestData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == Iterations*NumThreads);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Fetch_Add_Decrement_Thread_Callback_U32, &TestData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == 0);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
    }

    free(Threads);
}

typedef struct {
    uint32_t      Iterations;
    ak_atomic_u64 SharedInt;
} ak_atomic_increment_test_data_u64;

int32_t AK_Increment_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u64* TestData = (ak_atomic_increment_test_data_u64*)UserData;
    uint32_t i;
    for(i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Increment_U64_Relaxed(&TestData->SharedInt);
    }
    return 0;
}

int32_t AK_Decrement_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u64* TestData = (ak_atomic_increment_test_data_u64*)UserData;
    uint32_t i;
    for(i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Decrement_U64_Relaxed(&TestData->SharedInt);
    }
    return 0;
}

int32_t AK_Fetch_Add_Increment_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u64* TestData = (ak_atomic_increment_test_data_u64*)UserData;
    uint32_t i;
    for(i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Fetch_Add_U64_Relaxed(&TestData->SharedInt, 1);
    }
    return 0;
}

int32_t AK_Fetch_Add_Decrement_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u64* TestData = (ak_atomic_increment_test_data_u64*)UserData;
    uint32_t i;
    for(i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Fetch_Add_U64_Relaxed(&TestData->SharedInt, -1);
    }
    return 0;
}

UTEST(AK_Atomic, Increment64) {
    const uint32_t Iterations = 5000000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();

    ak_atomic_increment_test_data_u64 TestData = {0};
    TestData.Iterations = Iterations;

    ak_thread* Threads = (ak_thread*)malloc(sizeof(ak_thread)*NumThreads);
    uint64_t ThreadIndex;
    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Create(&Threads[ThreadIndex], AK_Increment_Thread_Callback_U64, &TestData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == Iterations*NumThreads);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Decrement_Thread_Callback_U64, &TestData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == 0);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Fetch_Add_Increment_Thread_Callback_U64, &TestData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == Iterations*NumThreads);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Fetch_Add_Decrement_Thread_Callback_U64, &TestData);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == 0);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
    }

    free(Threads);
}

typedef struct ak_atomic_compare_exchange_node ak_atomic_compare_exchange_node;

struct ak_atomic_compare_exchange_node {
    ak_atomic_compare_exchange_node* Next;   
};

typedef struct {
    ak_atomic_compare_exchange_node* Nodes;
    ak_atomic_ptr* ListHead;
    uint32_t Iterations;
} ak_atomic_compare_exchange_thread_data;

int32_t AK_Compare_Exchange_Thread_Callback_Ptr(ak_thread* Thread, void* UserData) {
    ak_atomic_compare_exchange_thread_data* ThreadData = (ak_atomic_compare_exchange_thread_data*)UserData;
    ak_atomic_compare_exchange_node* Nodes = ThreadData->Nodes;
    ak_atomic_ptr* ListHead = ThreadData->ListHead;

    uint32_t i; 
    for(i = 0; i < ThreadData->Iterations; i++) {
        ak_atomic_compare_exchange_node* Insert = Nodes + i;
        ak_atomic_compare_exchange_node* Head;
        do {
            Head = (ak_atomic_compare_exchange_node*)AK_Atomic_Load_Ptr_Relaxed(ListHead);
            Insert->Next = Head;
        } while(!AK_Atomic_Compare_Exchange_Bool_Ptr_Relaxed(ListHead, Head, Insert));
    }

    return 0;
}

UTEST(AK_Atomic, CompareExchangePtr) {
    static const uint32_t Iterations = 5000000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();

    ak_atomic_compare_exchange_node* Nodes = (ak_atomic_compare_exchange_node*)malloc(sizeof(ak_atomic_compare_exchange_node)*NumThreads*Iterations);
    ak_atomic_compare_exchange_thread_data* ThreadData = (ak_atomic_compare_exchange_thread_data*)malloc(sizeof(ak_atomic_compare_exchange_thread_data)*NumThreads); 
    ak_thread* Threads = (ak_thread*)malloc(sizeof(ak_thread)*NumThreads);
    memset(Nodes, 0, sizeof(ak_atomic_compare_exchange_node)*NumThreads*Iterations);
    memset(ThreadData, 0, sizeof(ak_atomic_compare_exchange_thread_data)*NumThreads);
    memset(Threads, 0, sizeof(ak_thread)*NumThreads);

    ak_atomic_ptr ListHead = {0};
    uint32_t ThreadIndex;
    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        ThreadData[ThreadIndex].Nodes = Nodes + ThreadIndex*Iterations;
        ThreadData[ThreadIndex].ListHead = &ListHead;
        ThreadData[ThreadIndex].Iterations = Iterations;

        AK_Thread_Create(&Threads[ThreadIndex], AK_Compare_Exchange_Thread_Callback_Ptr, &ThreadData[ThreadIndex]);
    }

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    uint32_t Count = 0;
    ak_atomic_compare_exchange_node* Node = (ak_atomic_compare_exchange_node*)ListHead.Nonatomic;
    while(Node) {
        ListHead.Nonatomic = Node->Next;
        Count++;
        Node = (ak_atomic_compare_exchange_node*)ListHead.Nonatomic;
    }
    ASSERT_EQ(Count, Iterations*NumThreads);

    for(ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
    }

    free(Threads);
    free(ThreadData);
    free(Nodes);
}

typedef struct {
    ak_rw_lock    RWLock;
    int32_t*      Shared;
    int32_t       SharedLength;
    uint32_t      Iterations;
    ak_atomic_u32 Success;
} ak_rw_lock_test_data;

int32_t AK_RW_Lock_Test_Thread(ak_thread* Thread, void* UserData) {
    ak_rw_lock_test_data* TestData = (ak_rw_lock_test_data*)UserData;

    uint32_t i;
    for(i = 0; i < TestData->Iterations; i++) {

        if(AK_Atomic_Test_Random_Between(0, 3) == 0) {
            int32_t Value = rand();

            AK_RW_Lock_Writer(&TestData->RWLock);
            int32_t j;
            for(j = TestData->SharedLength-1; j >= 0; j--) {
                TestData->Shared[j] = Value--;
            }
            AK_RW_Unlock_Writer(&TestData->RWLock);
        } else {
            bool Ok = true;
            {
                AK_RW_Lock_Reader(&TestData->RWLock);
                int32_t Value = TestData->Shared[0];

                int32_t j;
                for(j = 1; j < TestData->SharedLength; j++) {
                    Ok = Ok && (++Value == TestData->Shared[j]);
                }
                AK_RW_Unlock_Reader(&TestData->RWLock);
            }
            if(!Ok) {
                AK_Atomic_Store_U32_Relaxed(&TestData->Success, false);
            }
        }
    }

    return 0;
}

UTEST(AK_RW_Lock, Test) {
    static const uint32_t Iterations = 5000000;
    static const int32_t SharedArrayLength = 32;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();

    ak_rw_lock_test_data TestData = {0};
    TestData.Iterations = Iterations;
    TestData.Shared = malloc(sizeof(int32_t)*SharedArrayLength);
    TestData.SharedLength = SharedArrayLength;
    AK_RW_Lock_Create(&TestData.RWLock);
    
    int32_t si;
    for(si = 0; si < SharedArrayLength; si++) {
        TestData.Shared[si] = si;
    }
    AK_Atomic_Store_U32_Relaxed(&TestData.Success, true);

    ak_thread* Threads = (ak_thread*)malloc(NumThreads*sizeof(ak_thread));
    uint32_t i;
    for(i = 0; i < NumThreads; i++) {
        AK_Thread_Create(&Threads[i], AK_RW_Lock_Test_Thread, &TestData);
    }

    for(i = 0; i < NumThreads; i++) {
        AK_Thread_Wait(&Threads[i]);
    }

    for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(&Threads[i]);
    }

    free(Threads);
    ASSERT_TRUE(TestData.Success.Nonatomic);
    free(TestData.Shared);
    AK_RW_Lock_Delete(&TestData.RWLock);
}

#ifndef AK_DISABLE_ASYNC_SLOT_MAP

typedef struct {
    ak_slot64     SlotID;
    uint64_t      AllocateThreadID;
    uint64_t      FreeThreadID;
    ak_atomic_u32 Allocated;
    bool          Freed;
} ak_slot_map_thread_test_entry;

typedef struct {
    ak_async_slot_map64*           SlotMap;
    uint32_t                       Count;
    ak_slot_map_thread_test_entry* Entries;
} ak_slot_map_thread_test_data;

int32_t AK_Async_Slot_Map_Free_Thread_Callback(ak_thread* Thread, void* UserData) {
    ak_slot_map_thread_test_data* ThreadData = (ak_slot_map_thread_test_data*)UserData;
    ak_async_slot_map64* SlotMap = ThreadData->SlotMap;

    bool IsDone = false;
    while(!IsDone) {
        IsDone = true;
        uint32_t Index;
        for(Index = 0; Index < ThreadData->Count; Index++) {
            ak_slot_map_thread_test_entry* Entry = ThreadData->Entries + Index;
            if(AK_Atomic_Load_U32(&Entry->Allocated, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) && !Entry->Freed) {
                AK_Async_Slot_Map64_Free_Slot(SlotMap, Entry->SlotID);
                Entry->FreeThreadID = AK_Thread_Get_Current_ID();
                Entry->Freed = true;
            }

            if(!Entry->Freed) {
                IsDone = false;
            }
        }
    }

    return 0;
}

int32_t AK_Async_Slot_Map_Allocate_Thread_Callback(ak_thread* Thread, void* UserData) {
    ak_slot_map_thread_test_data* ThreadData = (ak_slot_map_thread_test_data*)UserData;
    ak_async_slot_map64* SlotMap = ThreadData->SlotMap;

    uint32_t Index;
    for(Index = 0; Index < ThreadData->Count; Index++) {
        ak_slot_map_thread_test_entry* Entry = ThreadData->Entries + Index;
        Entry->SlotID = AK_Async_Slot_Map64_Alloc_Slot(SlotMap);
        Entry->AllocateThreadID = AK_Thread_Get_Current_ID();
        AK_Atomic_Store_U32(&Entry->Allocated, true, AK_ATOMIC_MEMORY_ORDER_RELEASE);
    }
    return 0;
}

UTEST(AK_Async_Slot_Map64, Test) {
    const uint32_t NumWorkerThreads = AK_Get_Processor_Thread_Count();
    const uint32_t Batch = 1000000;

    ak_async_slot_map64 SlotMap;
    AK_Async_Slot_Map64_Alloc(&SlotMap, NumWorkerThreads*Batch, NULL);

    ak_slot_map_thread_test_entry* ThreadEntries = (ak_slot_map_thread_test_entry*)malloc((NumWorkerThreads*Batch)*sizeof(ak_slot_map_thread_test_entry));
    memset(ThreadEntries, 0, (NumWorkerThreads*Batch)*sizeof(ak_slot_map_thread_test_entry));
    ak_slot_map_thread_test_data* TestData = (ak_slot_map_thread_test_data*)malloc(sizeof(ak_slot_map_thread_test_data)*NumWorkerThreads);
    ak_thread* AllocateThreads = (ak_thread*)malloc(sizeof(ak_thread)*NumWorkerThreads);
    ak_thread* FreeThreads = (ak_thread*)malloc(sizeof(ak_thread)*NumWorkerThreads);

    uint32_t ThreadIndex;
    for(ThreadIndex = 0; ThreadIndex < NumWorkerThreads; ThreadIndex++) {
        ak_slot_map_thread_test_data* ThreadData = TestData+ThreadIndex;

        ThreadData->SlotMap = &SlotMap;
        ThreadData->Count   = Batch;
        ThreadData->Entries = ThreadEntries + ThreadIndex*Batch;

        AK_Thread_Create(&FreeThreads[ThreadIndex], AK_Async_Slot_Map_Free_Thread_Callback, ThreadData);
        AK_Thread_Create(&AllocateThreads[ThreadIndex], AK_Async_Slot_Map_Allocate_Thread_Callback, ThreadData);    
    }

    for(ThreadIndex = 0; ThreadIndex < NumWorkerThreads; ThreadIndex++) {
        AK_Thread_Delete(&FreeThreads[ThreadIndex]);
        AK_Thread_Delete(&AllocateThreads[ThreadIndex]);
    }

    uint32_t i;
    for(i = 0; i < NumWorkerThreads*Batch; i++) {
        ASSERT_TRUE(ThreadEntries[i].Allocated.Nonatomic);
        ASSERT_TRUE(ThreadEntries[i].Freed);
    }

    free(FreeThreads);
    free(AllocateThreads);
    free(TestData);
    free(ThreadEntries);
}

#endif

#ifndef AK_DISABLE_JOB_SYSTEM

typedef struct {
    uint8_t Size;
    uint8_t Data[AK_JOB_SYSTEM_FAST_USERDATA_SIZE-1];
} ak_job_system_job_user_data_small_data;

typedef struct {
    uint32_t Size;
    uint32_t Data[AK_JOB_SYSTEM_FAST_USERDATA_SIZE*10];
} ak_job_system_job_user_data_large_data;

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Job_System_Small_User_Data_Test_Callback) {
    ak_job_system_job_user_data_small_data* JobData = (ak_job_system_job_user_data_small_data*)JobUserData;
    assert(JobData->Size == AK_JOB_SYSTEM_FAST_USERDATA_SIZE);
    uint8_t i;
    for(i = 0; i < AK_JOB_SYSTEM_FAST_USERDATA_SIZE-1; i++) {
        assert(JobData->Data[i] == i);
    }
}

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Job_System_Large_User_Data_Test_Callback) {
    ak_job_system_job_user_data_large_data* JobData = (ak_job_system_job_user_data_large_data*)JobUserData;
    assert(JobData->Size == AK_JOB_SYSTEM_FAST_USERDATA_SIZE*10);
    uint32_t i;
    for(i = 0; i < AK_JOB_SYSTEM_FAST_USERDATA_SIZE*10; i++) {
        assert(JobData->Data[i] == i);
    }
}

typedef struct {
    ak_atomic_u64 AllocatedAmount;
    ak_atomic_u64 FreedAmount;
} ak_job_system_allocator_info; 

UTEST(AK_Job_System, UserData) {
    const uint32_t Iterations = 100000;

    ak_job_system_allocator_info AllocatorInfo = {0};
    ak_job_system* JobSystem = AK_Job_System_Create((Iterations*2)+1, AK_Get_Processor_Thread_Count(), 0, NULL, &AllocatorInfo);

    ak_job_id RootJob = AK_Job_System_Alloc_Empty_Job(JobSystem, AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
    uint32_t i;
    for(i = 0; i < Iterations; i++) {
        /*Make sure the job properly copies the data from the stack*/
        ak_job_system_job_user_data_small_data SmallData = {0};
        SmallData.Size = AK_JOB_SYSTEM_FAST_USERDATA_SIZE;
        
        uint8_t k;
        for(k = 0; k < AK_JOB_SYSTEM_FAST_USERDATA_SIZE-1; k++) {
            SmallData.Data[k] = k;
        }

        ak_job_system_job_user_data_large_data LargeData = {0};
        LargeData.Size = AK_JOB_SYSTEM_FAST_USERDATA_SIZE*10;

        uint32_t j;
        for(j = 0; j < AK_JOB_SYSTEM_FAST_USERDATA_SIZE*10; j++) {
            LargeData.Data[j] = j;
        }

        ak_job_system_data SmallJobData;
        SmallJobData.JobCallback = AK_Job_System_Small_User_Data_Test_Callback;
        SmallJobData.Data = &SmallData;
        SmallJobData.DataByteSize = sizeof(SmallData);
        ak_job_id JobID = AK_Job_System_Alloc_Job(JobSystem, SmallJobData, 
                                                  RootJob, AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT|AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
        
        ak_job_system_data LargeJobData;
        LargeJobData.JobCallback = AK_Job_System_Large_User_Data_Test_Callback;
        LargeJobData.Data = &LargeData;
        LargeJobData.DataByteSize = sizeof(LargeData);
        JobID = AK_Job_System_Alloc_Job(JobSystem, LargeJobData, 
                                        RootJob, AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT|AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
        ASSERT_TRUE(JobID != 0);
    }

    AK_Job_System_Add_Job(JobSystem, RootJob);
    AK_Job_System_Wait_For_Job(JobSystem, RootJob);
    AK_Job_System_Delete(JobSystem);
    ASSERT_TRUE(AllocatorInfo.AllocatedAmount.Nonatomic == AllocatorInfo.FreedAmount.Nonatomic);
}

typedef struct {
    uint32_t  Count;
    uint32_t* CounterTable;
} ak_job_system_thread_data;

typedef struct {
    ak_job_system_thread_data* ThreadData;
    uint32_t                   Index;
} ak_job_system_job_data;

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Job_System_Test_Callback) {
    ak_job_system_job_data* JobData = (ak_job_system_job_data*)JobUserData;
    assert(JobData->Index < JobData->ThreadData->Count); /*overflow occurred*/
    JobData->ThreadData->CounterTable[JobData->Index]++;
}

UTEST(AK_Job_System, MainTest) {
    const uint32_t ThreadCount = AK_Get_Processor_Thread_Count();
    const uint32_t MaxJobCount = (1000000);

    ak_job_system_allocator_info AllocatorInfo = {0};

    ak_job_system_thread_data ThreadData = {0};
    ThreadData.Count = MaxJobCount;
    ThreadData.CounterTable = (uint32_t*)malloc(sizeof(uint32_t)*MaxJobCount);
    memset(ThreadData.CounterTable, 0, sizeof(uint32_t)*MaxJobCount);

    ak_job_system* JobSystem = AK_Job_System_Create(MaxJobCount+1, ThreadCount, 0, NULL, &AllocatorInfo);
    ASSERT_TRUE(JobSystem);

    uint64_t StartTime = AK_Query_Performance_Counter();

    ak_job_id RootJob = AK_Job_System_Alloc_Empty_Job(JobSystem, AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
    uint32_t i;
    for(i = 0; i < MaxJobCount; i++) {
        ak_job_system_job_data JobData;
        JobData.ThreadData = &ThreadData;
        JobData.Index = i;

        ak_job_system_data JobCallbackData;
        JobCallbackData.JobCallback = AK_Job_System_Test_Callback;
        JobCallbackData.Data = &JobData;
        JobCallbackData.DataByteSize = sizeof(ak_job_system_job_data);

        ak_job_id JobID = AK_Job_System_Alloc_Job(JobSystem, JobCallbackData, 
                                                  RootJob, AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT|AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
        ASSERT_TRUE(JobID != 0);
    }

    AK_Job_System_Add_Job(JobSystem, RootJob);
    AK_Job_System_Wait_For_Job(JobSystem, RootJob);
    printf("Time: %fms\n", ((double)(AK_Query_Performance_Counter()-StartTime)/(double)AK_Query_Performance_Frequency())*1000.0);
    AK_Job_System_Delete(JobSystem);

    for(i = 0; i < MaxJobCount; i++) {
        /*Each index should've only been operated on once*/
        ASSERT_EQ(ThreadData.CounterTable[i], 1);
    }

    free(ThreadData.CounterTable);
    ASSERT_TRUE(AllocatorInfo.AllocatedAmount.Nonatomic == AllocatorInfo.FreedAmount.Nonatomic);
}

typedef struct {
    int32_t A;
    int32_t B;
    int32_t C;
    int32_t D;
    int32_t E;
    int32_t F;
} ak_job_system_dependency_test_data;

typedef struct ak_job_system_dependency_data_wrapper {
    ak_job_system_dependency_test_data* DependencyData;
} ak_job_system_dependency_data_wrapper;

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Async_Job_Dependency_A) {
    ak_job_system_dependency_test_data* DependencyData = ((ak_job_system_dependency_data_wrapper*)JobUserData)->DependencyData;
    assert(!DependencyData->A);
    assert(!DependencyData->C);
    assert(!DependencyData->F);
    AK_Atomic_Thread_Fence_Rel();
    DependencyData->A = true;
}

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Async_Job_Dependency_B) {
    ak_job_system_dependency_test_data* DependencyData = ((ak_job_system_dependency_data_wrapper*)JobUserData)->DependencyData;
    assert(!DependencyData->B);
    assert(!DependencyData->C);
    assert(!DependencyData->F);
    AK_Atomic_Thread_Fence_Rel();
    DependencyData->B = true;
}

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Async_Job_Dependency_C) {
    ak_job_system_dependency_test_data* DependencyData = ((ak_job_system_dependency_data_wrapper*)JobUserData)->DependencyData;
    assert(!DependencyData->C);
    assert(!DependencyData->F);
    AK_Atomic_Thread_Fence_Rel();
    DependencyData->C = true;
}

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Async_Job_Dependency_D) {
    ak_job_system_dependency_test_data* DependencyData = ((ak_job_system_dependency_data_wrapper*)JobUserData)->DependencyData;
    assert(!DependencyData->D);
    assert(!DependencyData->E);
    assert(!DependencyData->F);
    AK_Atomic_Thread_Fence_Rel();
    DependencyData->D = true;
}

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Async_Job_Dependency_E) {
    ak_job_system_dependency_test_data* DependencyData = ((ak_job_system_dependency_data_wrapper*)JobUserData)->DependencyData;
    assert(!DependencyData->E);
    assert(!DependencyData->F);
    AK_Atomic_Thread_Fence_Rel();
    DependencyData->E = true;
}

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Async_Job_Dependency_F) {
    ak_job_system_dependency_test_data* DependencyData = ((ak_job_system_dependency_data_wrapper*)JobUserData)->DependencyData;
    assert(DependencyData->A);
    assert(DependencyData->B);
    assert(DependencyData->D);
    assert(DependencyData->E);
    assert(!DependencyData->F);
    AK_Atomic_Thread_Fence_Rel();
    DependencyData->F = true;
}

UTEST(AK_Job_System, DependencyTest) {
    const uint32_t Iterations = 100000;

    ak_job_system_allocator_info AllocatorInfo = {0};

    ak_job_system* JobSystem = AK_Job_System_Create((6*Iterations)+1, AK_Get_Processor_Thread_Count(), (5*Iterations), NULL, &AllocatorInfo);

    ak_job_system_dependency_test_data* DependencyDataArray = (ak_job_system_dependency_test_data*)malloc(Iterations*sizeof(ak_job_system_dependency_test_data));
    memset(DependencyDataArray, 0, Iterations*sizeof(ak_job_system_dependency_test_data));

    ak_job_id RootJob = AK_Job_System_Alloc_Empty_Job(JobSystem, 0);

    uint32_t i;
    for(i = 0; i < Iterations; i++) {
        ak_job_system_dependency_data_wrapper Wrapper;
        Wrapper.DependencyData = &DependencyDataArray[i];

        ak_job_system_data JobDataA;
        JobDataA.JobCallback = AK_Async_Job_Dependency_A;
        JobDataA.Data = &Wrapper;
        JobDataA.DataByteSize = sizeof(Wrapper);

        ak_job_system_data JobDataB;
        JobDataB.JobCallback = AK_Async_Job_Dependency_B;
        JobDataB.Data = &Wrapper;
        JobDataB.DataByteSize = sizeof(Wrapper);

        ak_job_system_data JobDataC;
        JobDataC.JobCallback = AK_Async_Job_Dependency_C;
        JobDataC.Data = &Wrapper;
        JobDataC.DataByteSize = sizeof(Wrapper);

        ak_job_system_data JobDataD;
        JobDataD.JobCallback = AK_Async_Job_Dependency_D;
        JobDataD.Data = &Wrapper;
        JobDataD.DataByteSize = sizeof(Wrapper);

        ak_job_system_data JobDataE;
        JobDataE.JobCallback = AK_Async_Job_Dependency_E;
        JobDataE.Data = &Wrapper;
        JobDataE.DataByteSize = sizeof(Wrapper);

        ak_job_system_data JobDataF;
        JobDataF.JobCallback = AK_Async_Job_Dependency_F;
        JobDataF.Data = &Wrapper;
        JobDataF.DataByteSize = sizeof(Wrapper);

        ak_job_id JobA = AK_Job_System_Alloc_Job(JobSystem, JobDataA, RootJob, AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
        ak_job_id JobB = AK_Job_System_Alloc_Job(JobSystem, JobDataB, RootJob, AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
        ak_job_id JobC = AK_Job_System_Alloc_Job(JobSystem, JobDataC, RootJob, AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
        ak_job_id JobD = AK_Job_System_Alloc_Job(JobSystem, JobDataD, RootJob, AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
        ak_job_id JobE = AK_Job_System_Alloc_Job(JobSystem, JobDataE, RootJob, AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
        ak_job_id JobF = AK_Job_System_Alloc_Job(JobSystem, JobDataF, RootJob, AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
        
        AK_Job_System_Add_Dependency(JobSystem, JobA, JobC);
        AK_Job_System_Add_Dependency(JobSystem, JobB, JobC);
        AK_Job_System_Add_Dependency(JobSystem, JobD, JobE);
        AK_Job_System_Add_Dependency(JobSystem, JobC, JobF);
        AK_Job_System_Add_Dependency(JobSystem, JobE, JobF);
        AK_Job_System_Add_Job(JobSystem, JobA);
        AK_Job_System_Add_Job(JobSystem, JobB);
        AK_Job_System_Add_Job(JobSystem, JobD);
    }
    AK_Job_System_Add_Job(JobSystem, RootJob);
    AK_Job_System_Wait_For_Job(JobSystem, RootJob);
    AK_Job_System_Free_Job(JobSystem, RootJob);

    for(i = 0; i < Iterations; i++) {
        ak_job_system_dependency_test_data* DependencyData = DependencyDataArray+i;
        ASSERT_TRUE(DependencyData->A);
        ASSERT_TRUE(DependencyData->B);
        ASSERT_TRUE(DependencyData->C);
        ASSERT_TRUE(DependencyData->D);
        ASSERT_TRUE(DependencyData->E);
        ASSERT_TRUE(DependencyData->F);
    }

    free(DependencyDataArray);
    AK_Job_System_Delete(JobSystem);
    ASSERT_TRUE(AllocatorInfo.AllocatedAmount.Nonatomic == AllocatorInfo.FreedAmount.Nonatomic);
}

AK_JOB_SYSTEM_CALLBACK_DEFINE(AK_Job_System_Sleep_Test_Callback) {
    uint64_t StartTime = AK_Query_Performance_Counter();
    AK_Sleep(10000);
    printf("Time: %fms\n", ((double)(AK_Query_Performance_Counter()-StartTime)/(double)AK_Query_Performance_Frequency())*1000.0);
}

UTEST(AK_Job_System, SleepTest) {
    ak_job_system_allocator_info AllocatorInfo = {0};

    ak_job_system* JobSystem = AK_Job_System_Create(1, AK_Get_Processor_Thread_Count(), 0, NULL, &AllocatorInfo);
    ASSERT_TRUE(JobSystem);

    ak_job_system_data JobData = {0};
    JobData.JobCallback = AK_Job_System_Sleep_Test_Callback;
    ak_job_id SleepID = AK_Job_System_Alloc_Job(JobSystem, JobData, 0, AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT|AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
    ASSERT_TRUE(SleepID != 0);

    AK_Job_System_Wait_For_Job(JobSystem, SleepID);
    AK_Job_System_Delete(JobSystem);

    ASSERT_TRUE(AllocatorInfo.AllocatedAmount.Nonatomic == AllocatorInfo.FreedAmount.Nonatomic);
}

typedef struct {
    ak_atomic_u32 ThreadsStarted;
    ak_atomic_u32 ThreadsUpdated;
    ak_atomic_u32 ThreadsEnded;
} ak_job_thread_callback_data;

AK_JOB_THREAD_BEGIN_DEFINE(AK_Job_System_Thread_Begin_Test) {
    ak_job_thread_callback_data* ThreadCallbackData = (ak_job_thread_callback_data*)UserData;
    AK_Atomic_Increment_U32_Relaxed(&ThreadCallbackData->ThreadsStarted);
}

AK_JOB_THREAD_UPDATE_DEFINE(AK_Job_System_Thread_Update_Test) {
    ak_job_thread_callback_data* ThreadCallbackData = (ak_job_thread_callback_data*)UserData;
    AK_Atomic_Increment_U32_Relaxed(&ThreadCallbackData->ThreadsUpdated);
}

AK_JOB_THREAD_END_DEFINE(AK_Job_System_Thread_End_Test) {
    ak_job_thread_callback_data* ThreadCallbackData = (ak_job_thread_callback_data*)UserData;
    AK_Atomic_Increment_U32_Relaxed(&ThreadCallbackData->ThreadsEnded);
}

UTEST(AK_Job_System, ThreadCallbacks) {
    const uint32_t ThreadCount = 8;

    ak_job_thread_callback_data ThreadCallbackData = {0};
    ak_job_system_allocator_info AllocatorInfo = {0};

    ak_job_thread_callbacks ThreadCallbacks = {0};
    ThreadCallbacks.JobThreadBegin  = AK_Job_System_Thread_Begin_Test;
    ThreadCallbacks.JobThreadEnd    = AK_Job_System_Thread_End_Test;
    ThreadCallbacks.JobThreadUpdate = AK_Job_System_Thread_Update_Test;
    ThreadCallbacks.UserData = &ThreadCallbackData;

    ak_job_system* JobSystem = AK_Job_System_Create(1, ThreadCount, 0, &ThreadCallbacks, &AllocatorInfo);
    
    ak_job_system_data JobData = {0};
    ak_job_id JobID = AK_Job_System_Alloc_Job(JobSystem, JobData, 0, AK_JOB_FLAG_QUEUE_IMMEDIATELY_BIT|AK_JOB_FLAG_FREE_WHEN_DONE_BIT);
    AK_Job_System_Wait_For_Job(JobSystem, JobID);
    AK_Job_System_Delete(JobSystem);

    ASSERT_EQ(ThreadCallbackData.ThreadsStarted.Nonatomic, ThreadCount);
    ASSERT_EQ(ThreadCallbackData.ThreadsEnded.Nonatomic, ThreadCount);
    ASSERT_GE(ThreadCallbackData.ThreadsUpdated.Nonatomic, 1);

    ASSERT_TRUE(AllocatorInfo.AllocatedAmount.Nonatomic == AllocatorInfo.FreedAmount.Nonatomic);
}

#endif

#if !defined(ANDROID_BUILD)
UTEST_MAIN()
#endif

void* AK_Job_System_Test_Allocate_Memory(size_t Size, void* UserData) {
    if(UserData) {
        ak_job_system_allocator_info* AllocatorInfo = (ak_job_system_allocator_info*)UserData;
        size_t* Memory = malloc(Size+sizeof(size_t));
        *Memory = Size;
        AK_Atomic_Fetch_Add_U64_Relaxed(&AllocatorInfo->AllocatedAmount, Size);
        return Memory+1;
    } else {
        return malloc(Size);
    }
}

void AK_Job_System_Test_Free_Memory(void* Memory, void* UserData) {
    if(UserData) {
        ak_job_system_allocator_info* AllocatorInfo = (ak_job_system_allocator_info*)UserData;
        size_t* Ptr = ((size_t*)Memory)-1;
        size_t Size = *Ptr;
        free(Ptr);
        AK_Atomic_Fetch_Add_U64_Relaxed(&AllocatorInfo->FreedAmount, Size);

    } else {
        free(Memory);
    }
}

#define AK_JOB_MALLOC(size, user_data) AK_Job_System_Test_Allocate_Memory(size, user_data)
#define AK_JOB_FREE(memory, user_data) AK_Job_System_Test_Free_Memory(memory, user_data)
#define AK_ATOMIC_IMPLEMENTATION
#include <ak_atomic.h>