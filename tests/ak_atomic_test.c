#include <ak_atomic.h>
#include "utest.h"

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
        while(rand() % 8 != 0) { } //random delay
        AK_Atomic_Store_U32_Relaxed(&TestData->X, 1);
        AK_Atomic_Thread_Fence_Seq_Cst();
        TestData->R1 = AK_Atomic_Load_U32_Relaxed(&TestData->Y);
    } else {
        while(rand() % 8 != 0) { } //random delay
        AK_Atomic_Store_U32_Relaxed(&TestData->Y, 1);
        AK_Atomic_Thread_Fence_Seq_Cst();
        TestData->R2 = AK_Atomic_Load_U32_Relaxed(&TestData->X);
    }

    return 0;
}

//For this test we want to make sure loads and stores are done in certain
//orders via barriers. Before jobs are ran, 0 is filled in for X and Y 
//In the above job we assign
UTEST(AK_Atomic, StoreAndLoad32) {
    const uint32_t Iterations = 1000;

    for(uint32_t i = 0; i < Iterations; i++) {
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
        while(rand() % 8 != 0) { } //random delay
        AK_Atomic_Store_U64_Relaxed(&TestData->X, 1);
        AK_Atomic_Thread_Fence_Seq_Cst();
        TestData->R1 = AK_Atomic_Load_U64_Relaxed(&TestData->Y);
    } else {
        while(rand() % 8 != 0) { } //random delay
        AK_Atomic_Store_U64_Relaxed(&TestData->Y, 1);
        AK_Atomic_Thread_Fence_Seq_Cst();
        TestData->R2 = AK_Atomic_Load_U64_Relaxed(&TestData->X);
    }

    return 0;
}

//For this test we want to make sure loads and stores are done in certain
//orders via barriers. Before jobs are ran, 0 is filled in for X and Y 
//In the above job we assign
UTEST(AK_Atomic, StoreAndLoad64) {
    const uint32_t Iterations = 1000;

    for(uint32_t i = 0; i < Iterations; i++) {
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

    //The processor (even on x86) can delay the stores until after
    //the loads without the processor fence. Compiler fence is not enough
    if(ThreadData->Index == 0) {
        uint32_t Index = 0;
        for(uint32_t i = 0; i < ThreadData->Iterations; i++) {
            Index = (Index*Index+1) % 65521; //Make it a little random
            uint32_t Value = ThreadData->Values[Index & 7];
            //Compiler_Fence_Release(); //Prevent compiler from hosting the store out of the loop
            AK_Atomic_Store_U32_Relaxed(&TestData->SharedInt, Value);
        }
    } else {
        for(uint32_t i = 0; i < ThreadData->Iterations; i++) {
            //Compiler_Fence_Acquire(); //Prevent compiler from hosting the load out of the loop
            uint32_t Value = AK_Atomic_Load_U32_Relaxed(&TestData->SharedInt);
            if((Value*Value) < ThreadData->Limit) {
                TestData->Success = false;
            }
        }
    }

    return 0;
}

UTEST(AK_Atomic, LoadAndStore32) {
    static const uint32_t Iterations = 10000000;
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

    ak_atomic_load_store_test_thread_data_u32 ThreadData2 = {0};
    ThreadData2.Data = &TestData;
    ThreadData2.Index = 1;

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

    //The processor (even on x86) can delay the stores until after
    //the loads without the processor fence. Compiler fence is not enough
    if(ThreadData->Index == 0) {
        uint32_t Index = 0;
        for(uint32_t i = 0; i < ThreadData->Iterations; i++) {
            Index = (Index*Index+1) % 65521; //Make it a little random
            uint64_t Value = ThreadData->Values[Index & 7];
            //Compiler_Fence_Release(); //Prevent compiler from hosting the store out of the loop
            AK_Atomic_Store_U64_Relaxed(&TestData->SharedInt, Value);
        }
    } else {
        for(uint32_t i = 0; i < ThreadData->Iterations; i++) {
            //Compiler_Fence_Acquire(); //Prevent compiler from hosting the load out of the loop
            uint64_t Value = AK_Atomic_Load_U64_Relaxed(&TestData->SharedInt);
            if((Value*Value) < ThreadData->Limit) {
                TestData->Success = false;
            }
        }
    }

    return 0;
}

UTEST(AK_Atomic, LoadAndStore64) {
    static const uint32_t Iterations = 10000000;
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

    ak_atomic_load_store_test_thread_data_u64 ThreadData2 = {0};
    ThreadData2.Data = &TestData;
    ThreadData2.Index = 1;

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
    for(uint32_t i = Base+1; i <= Base+ThreadData->Data->ValuesPerThread; i++) {
        uint32_t OldValue = AK_Atomic_Exchange_U32_Relaxed(&ThreadData->Data->SharedInt, i);
        ThreadData->Data->ValuesSeen[i] = OldValue;
    }

    return 0;
}

UTEST(AK_Atomic, Exchange32) {
    static const uint32_t ValuesPerThread = 100000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();
    uint32_t NumValues = (ValuesPerThread*NumThreads)+1;
    uint32_t* ValuesSeen = malloc(sizeof(uint32_t)*NumValues);
    ak_atomic_exchange_thread_data_u32* ThreadsData = malloc(sizeof(ak_atomic_exchange_thread_data_u32)*NumThreads);
    memset(ValuesSeen, 0, sizeof(uint32_t)*NumValues);

    ak_atomic_exchange_test_data_u32 TestData = {0};
    TestData.ValuesSeen = ValuesSeen;
    TestData.ValuesPerThread = ValuesPerThread;

    ak_thread* Threads = malloc(sizeof(ak_thread)*NumThreads);
    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        ak_atomic_exchange_thread_data_u32* ThreadData = ThreadsData + ThreadIndex;
        ThreadData->Data = &TestData;
        ThreadData->ThreadIndex = ThreadIndex;
        AK_Thread_Create(&Threads[ThreadIndex], AK_Exchange_Thread_Callback_U32, ThreadData);
    }

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ValuesSeen[0] = TestData.SharedInt.Nonatomic;
    qsort(ValuesSeen, NumValues, sizeof(uint32_t), AK_Test_Compare_U32);
    uint32_t* Expected = (uint32_t*)malloc(sizeof(uint32_t)*NumValues);
    for(uint32_t i = 0; i < NumValues; i++) {
        Expected[i] = i;
    }
    qsort(Expected, NumValues, sizeof(uint32_t), AK_Test_Compare_U32);
    bool Success = memcmp(ValuesSeen, Expected, sizeof(uint32_t)*NumValues) == 0;

    ASSERT_TRUE(Success);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
    }

    free(ThreadsData);
    free(Expected);
    free(Threads);
    free(ValuesSeen);
}

typedef struct {
    uint32_t Iterations;
    ak_atomic_u32 Flag;
    uint32_t SharedValue;
} ak_atomic_compare_exchange_thread_data_u32;

int32_t AK_Compare_Exchange_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_compare_exchange_thread_data_u32* ThreadData = (ak_atomic_compare_exchange_thread_data_u32*)UserData;
    uint32_t Count = 0;
    while(Count < ThreadData->Iterations) {
        if(AK_Atomic_Compare_Exchange_Bool_U32_Relaxed(&ThreadData->Flag, 0, 1)) {
            ThreadData->SharedValue++;
            AK_Atomic_Store_U32_Relaxed(&ThreadData->Flag, 0);
            Count++;
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
    for(uint32_t i = 0; i < NumThreads; i++) {
        AK_Thread_Create(&Threads[i], AK_Compare_Exchange_Thread_Callback_U32, &ThreadData);
    }

    for(uint32_t i = 0; i < NumThreads; i++) {
        AK_Thread_Wait(&Threads[i]);
    }

    ASSERT_TRUE(ThreadData.SharedValue == Iterations*NumThreads);

    for(uint32_t i = 0; i < NumThreads; i++) {
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
        if(AK_Atomic_Compare_Exchange_Bool_U64_Relaxed(&ThreadData->Flag, 0, 1)) {
            ThreadData->SharedValue++;
            AK_Atomic_Store_U64_Relaxed(&ThreadData->Flag, 0);
            Count++;
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
    for(uint32_t i = 0; i < NumThreads; i++) {
        AK_Thread_Create(&Threads[i], AK_Compare_Exchange_Thread_Callback_U64, &ThreadData);
    }

    for(uint32_t i = 0; i < NumThreads; i++) {
        AK_Thread_Wait(&Threads[i]);
    }

    ASSERT_TRUE(ThreadData.SharedValue == Iterations*NumThreads);

    for(uint32_t i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(&Threads[i]);
    }

    free(Threads);
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
    for(uint64_t i = Base+1; i <= Base+ThreadData->Data->ValuesPerThread; i++) {
        uint64_t OldValue = AK_Atomic_Exchange_U64_Relaxed(&ThreadData->Data->SharedInt, i);
        ThreadData->Data->ValuesSeen[i] = OldValue;
    }

    return 0;
}

UTEST(AK_Atomic, Exchange64) {
    static const uint64_t ValuesPerThread = 100000;
    uint64_t NumThreads = AK_Get_Processor_Thread_Count();
    uint64_t NumValues = (ValuesPerThread*NumThreads)+1;
    uint64_t* ValuesSeen = malloc(sizeof(uint64_t)*NumValues);
    ak_atomic_exchange_thread_data_u64* ThreadsData = malloc(sizeof(ak_atomic_exchange_thread_data_u64)*NumThreads);
    memset(ValuesSeen, 0, sizeof(uint64_t)*NumValues);

    ak_atomic_exchange_test_data_u64 TestData = {0};
    TestData.ValuesSeen = ValuesSeen;
    TestData.ValuesPerThread = ValuesPerThread;

    ak_thread* Threads = malloc(sizeof(ak_thread)*NumThreads);
    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        ak_atomic_exchange_thread_data_u64* ThreadData = ThreadsData + ThreadIndex;
        ThreadData->Data = &TestData;
        ThreadData->ThreadIndex = ThreadIndex;
        AK_Thread_Create(&Threads[ThreadIndex], AK_Exchange_Thread_Callback_U64, ThreadData);
    }

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ValuesSeen[0] = TestData.SharedInt.Nonatomic;
    qsort(ValuesSeen, NumValues, sizeof(uint64_t), AK_Test_Compare_U64);
    uint64_t* Expected = (uint64_t*)malloc(sizeof(uint64_t)*NumValues);
    for(uint64_t i = 0; i < NumValues; i++) {
        Expected[i] = i;
    }
    qsort(Expected, NumValues, sizeof(uint64_t), AK_Test_Compare_U64);
    bool Success = memcmp(ValuesSeen, Expected, sizeof(uint64_t)*NumValues) == 0;

    ASSERT_TRUE(Success);

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
    }

    free(ThreadsData);
    free(Expected);
    free(Threads);
    free(ValuesSeen);
}

typedef struct {
    uint32_t      Iterations;
    ak_atomic_u32 SharedInt;
} ak_atomic_increment_test_data_u32;

int32_t AK_Increment_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u32* TestData = (ak_atomic_increment_test_data_u32*)UserData;
    for(uint32_t i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Increment_U32_Relaxed(&TestData->SharedInt);
    }
    return 0;
}

int32_t AK_Decrement_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u32* TestData = (ak_atomic_increment_test_data_u32*)UserData;
    for(uint32_t i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Decrement_U32_Relaxed(&TestData->SharedInt);
    }
    return 0;
}

int32_t AK_Fetch_Add_Increment_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u32* TestData = (ak_atomic_increment_test_data_u32*)UserData;
    for(uint32_t i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Fetch_Add_U32_Relaxed(&TestData->SharedInt, 1);
    }
    return 0;
}

int32_t AK_Fetch_Add_Decrement_Thread_Callback_U32(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u32* TestData = (ak_atomic_increment_test_data_u32*)UserData;
    for(uint32_t i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Fetch_Add_U32_Relaxed(&TestData->SharedInt, -1);
    }
    return 0;
}

UTEST(AK_Atomic, Increment32) {
    const uint32_t Iterations = 100000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();

    ak_atomic_increment_test_data_u32 TestData = {0};
    TestData.Iterations = Iterations;

    ak_thread* Threads = (ak_thread*)malloc(sizeof(ak_thread)*NumThreads);
    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Create(&Threads[ThreadIndex], AK_Increment_Thread_Callback_U32, &TestData);
    }

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == Iterations*NumThreads);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Decrement_Thread_Callback_U32, &TestData);
    }

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == 0);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Fetch_Add_Increment_Thread_Callback_U32, &TestData);
    }

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == Iterations*NumThreads);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Fetch_Add_Decrement_Thread_Callback_U32, &TestData);
    }

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == 0);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
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
    for(uint32_t i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Increment_U64_Relaxed(&TestData->SharedInt);
    }
    return 0;
}

int32_t AK_Decrement_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u64* TestData = (ak_atomic_increment_test_data_u64*)UserData;
    for(uint32_t i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Decrement_U64_Relaxed(&TestData->SharedInt);
    }
    return 0;
}

int32_t AK_Fetch_Add_Increment_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u64* TestData = (ak_atomic_increment_test_data_u64*)UserData;
    for(uint32_t i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Fetch_Add_U64_Relaxed(&TestData->SharedInt, 1);
    }
    return 0;
}

int32_t AK_Fetch_Add_Decrement_Thread_Callback_U64(ak_thread* Thread, void* UserData) {
    ak_atomic_increment_test_data_u64* TestData = (ak_atomic_increment_test_data_u64*)UserData;
    for(uint32_t i = 0; i < TestData->Iterations; i++) {
        AK_Atomic_Fetch_Add_U64_Relaxed(&TestData->SharedInt, -1);
    }
    return 0;
}

UTEST(AK_Atomic, Increment64) {
    const uint32_t Iterations = 100000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();

    ak_atomic_increment_test_data_u64 TestData = {0};
    TestData.Iterations = Iterations;

    ak_thread* Threads = (ak_thread*)malloc(sizeof(ak_thread)*NumThreads);
    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Create(&Threads[ThreadIndex], AK_Increment_Thread_Callback_U64, &TestData);
    }

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == Iterations*NumThreads);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Decrement_Thread_Callback_U64, &TestData);
    }

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == 0);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Fetch_Add_Increment_Thread_Callback_U64, &TestData);
    }

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == Iterations*NumThreads);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
        AK_Thread_Create(&Threads[ThreadIndex], AK_Fetch_Add_Decrement_Thread_Callback_U64, &TestData);
    }

    for(uint64_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    ASSERT_TRUE(TestData.SharedInt.Nonatomic == 0);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
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

    for(uint32_t i = 0; i < ThreadData->Iterations; i++) {
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
    static const uint32_t Iterations = 2000000;
    uint32_t NumThreads = AK_Get_Processor_Thread_Count();

    ak_atomic_compare_exchange_node* Nodes = (ak_atomic_compare_exchange_node*)malloc(sizeof(ak_atomic_compare_exchange_node)*NumThreads*Iterations);
    ak_atomic_compare_exchange_thread_data* ThreadData = (ak_atomic_compare_exchange_thread_data*)malloc(sizeof(ak_atomic_compare_exchange_thread_data)*NumThreads); 
    ak_thread* Threads = (ak_thread*)malloc(sizeof(ak_thread)*NumThreads);
    memset(Nodes, 0, sizeof(ak_atomic_compare_exchange_node)*NumThreads*Iterations);
    memset(ThreadData, 0, sizeof(ak_atomic_compare_exchange_thread_data)*NumThreads);
    memset(Threads, 0, sizeof(ak_thread)*NumThreads);

    ak_atomic_ptr ListHead = {0};
    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        ThreadData[ThreadIndex].Nodes = Nodes + ThreadIndex*Iterations;
        ThreadData[ThreadIndex].ListHead = &ListHead;
        ThreadData[ThreadIndex].Iterations = Iterations;

        AK_Thread_Create(&Threads[ThreadIndex], AK_Compare_Exchange_Thread_Callback_Ptr, &ThreadData[ThreadIndex]);
    }

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Wait(&Threads[ThreadIndex]);
    }

    uint32_t Count = 0;
    ak_atomic_compare_exchange_node* Node = (ak_atomic_compare_exchange_node*)ListHead.Nonatomic;
    while(Node) {
        ListHead.Nonatomic = Node->Next;
        Count++;
        Node = (ak_atomic_compare_exchange_node*)ListHead.Nonatomic;
    }
    ASSERT_EQ(Count, 2000000*NumThreads);

    for(uint32_t ThreadIndex = 0; ThreadIndex < NumThreads; ThreadIndex++) {
        AK_Thread_Delete(&Threads[ThreadIndex]);
    }

    free(Threads);
    free(ThreadData);
    free(Nodes);
}

UTEST_MAIN()

#define AK_ATOMIC_IMPLEMENTATION
#include <ak_atomic.h>