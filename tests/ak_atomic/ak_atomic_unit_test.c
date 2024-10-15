#include "ak_atomic_test_header.h"
#include <tests/utest.h>

#define true_t 1
#define false_t 0

#define IMAX_BITS(m) ((m)/((m)%255+1) / 255%255*8 + 7-86/((m)%255+12))
#define RAND_MAX_WIDTH IMAX_BITS(RAND_MAX)
AK_ATOMIC__COMPILE_TIME_ASSERT((RAND_MAX & (RAND_MAX + 1u)) == 0);

static uint64_t Random_U64(void) {
  uint64_t r = 0;
  int i;
  for (i = 0; i < 64; i += RAND_MAX_WIDTH) {
    r <<= RAND_MAX_WIDTH;
    r ^= (unsigned) rand();
  }
  return r;
}

static uint32_t Random_U32(void) {
	return (uint32_t)Random_U64();
}

static uint16_t Random_U16(void) {
	return (uint16_t)Random_U64();
}

static uint8_t Random_U8(void) {
	return (uint8_t)Random_U64();
}

typedef struct {
	ak_atomic_u8 A;
	ak_atomic_u8 B;
	int 		 HasProcessed;
	int 		 HasPassed;
} loadload8_entry;

typedef struct {
	ak_thread* 		 Thread;
	uint32_t 		 Count;
	loadload8_entry* Entries;
} loadload8_thread;

typedef struct {
	uint32_t 		  ThreadCount;
	loadload8_thread* Threads;
} loadload8_context;

static AK_THREAD_CALLBACK_DEFINE(LoadLoadU8) {
	uint32_t i;
	int FinishedProcessing;
	loadload8_thread* Context = (loadload8_thread*)UserData;
	(void)Thread;
	
	for (;;) {
		FinishedProcessing = true_t;

		for (i = 0; i < Context->Count; i++) {
			loadload8_entry* Entry = Context->Entries + i;
			if (!Entry->HasProcessed) {
				/*Note that the Load to Entry->A must have an acquire barrier otherwise B 
				  may be read before A and therefore it may not have the value of 2 yet*/
				uint8_t A = AK_Atomic_Load_U8(&Entry->A, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
				uint8_t B = AK_Atomic_Load_U8(&Entry->B, AK_ATOMIC_MEMORY_ORDER_RELAXED);
				if (A == 2) {
					Entry->HasProcessed = true_t;
					Entry->HasPassed = (B == 2);
				} else {
					FinishedProcessing = false_t;
				}
			}
		}

		if (FinishedProcessing) {
			return 0;
		}
	}
}

UTEST(LoadLoad, U8) {
	uint32_t i, j;
	loadload8_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (loadload8_thread*)Allocate_Memory(sizeof(loadload8_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload8_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (loadload8_entry*)Allocate_Memory(sizeof(loadload8_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(loadload8_entry));
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload8_thread* Thread = Context.Threads + i;
		Thread->Thread = AK_Thread_Create(LoadLoadU8, Thread);
	}
	for (i = 0; i < Context.ThreadCount; i++) {
		loadload8_thread* Thread = Context.Threads + i;
		
		for (j = 0; j < Thread->Count; j++) {
			loadload8_entry* Entry = Thread->Entries + j;
			AK_Atomic_Store_U8(&Entry->B, 2, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
			AK_Atomic_Store_U8(&Entry->A, 2, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		}
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload8_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->Thread);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_TRUE(Thread->Entries[j].HasPassed == true_t);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	ak_atomic_u16 A;
	ak_atomic_u16 B;
	int 		  HasProcessed;
	int 		  HasPassed;
} loadload16_entry;

typedef struct {
	ak_thread* 		  Thread;
	uint32_t 		  Count;
	loadload16_entry* Entries;
} loadload16_thread;

typedef struct {
	uint32_t 		   ThreadCount;
	loadload16_thread* Threads;
} loadload16_context;

static AK_THREAD_CALLBACK_DEFINE(LoadLoadU16) {
	uint32_t i;
	int FinishedProcessing;
	loadload16_thread* Context = (loadload16_thread*)UserData;
	(void)Thread;
	
	for (;;) {
		FinishedProcessing = true_t;

		for (i = 0; i < Context->Count; i++) {
			loadload16_entry* Entry = Context->Entries + i;
			if (!Entry->HasProcessed) {
				/*Note that the Load to Entry->A must have an acquire barrier otherwise B 
				  may be read before A and therefore it may not have the value of 2 yet*/
				uint16_t A = AK_Atomic_Load_U16(&Entry->A, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
				uint16_t B = AK_Atomic_Load_U16(&Entry->B, AK_ATOMIC_MEMORY_ORDER_RELAXED);
				if (A == 2) {
					Entry->HasProcessed = true_t;
					Entry->HasPassed = (B == 2);
				} else {
					FinishedProcessing = false_t;
				}
			}
		}

		if (FinishedProcessing) {
			return 0;
		}
	}
}

UTEST(LoadLoad, U16) {
	uint32_t i, j;
	loadload16_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (loadload16_thread*)Allocate_Memory(sizeof(loadload16_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload16_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (loadload16_entry*)Allocate_Memory(sizeof(loadload16_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(loadload16_entry));
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload16_thread* Thread = Context.Threads + i;
		Thread->Thread = AK_Thread_Create(LoadLoadU16, Thread);
	}
	for (i = 0; i < Context.ThreadCount; i++) {
		loadload16_thread* Thread = Context.Threads + i;
		
		for (j = 0; j < Thread->Count; j++) {
			loadload16_entry* Entry = Thread->Entries + j;
			AK_Atomic_Store_U16(&Entry->B, 2, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
			AK_Atomic_Store_U16(&Entry->A, 2, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		}
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload16_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->Thread);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_TRUE(Thread->Entries[j].HasPassed == true_t);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	ak_atomic_u32 A;
	ak_atomic_u32 B;
	int 		  HasProcessed;
	int 		  HasPassed;
} loadload32_entry;

typedef struct {
	ak_thread* 		  Thread;
	uint32_t 		  Count;
	loadload32_entry* Entries;
} loadload32_thread;

typedef struct {
	uint32_t 		   ThreadCount;
	loadload32_thread* Threads;
} loadload32_context;

static AK_THREAD_CALLBACK_DEFINE(LoadLoadU32) {
	uint32_t i;
	int FinishedProcessing;
	loadload32_thread* Context = (loadload32_thread*)UserData;
	(void)Thread;
	
	for (;;) {
		FinishedProcessing = true_t;

		for (i = 0; i < Context->Count; i++) {
			loadload32_entry* Entry = Context->Entries + i;
			if (!Entry->HasProcessed) {
				/*Note that the Load to Entry->A must have an acquire barrier otherwise B 
				  may be read before A and therefore it may not have the value of 2 yet*/
				uint32_t A = AK_Atomic_Load_U32(&Entry->A, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
				uint32_t B = AK_Atomic_Load_U32(&Entry->B, AK_ATOMIC_MEMORY_ORDER_RELAXED);
				if (A == 2) {
					Entry->HasProcessed = true_t;
					Entry->HasPassed = (B == 2);
				} else {
					FinishedProcessing = false_t;
				}
			}
		}

		if (FinishedProcessing) {
			return 0;
		}
	}
}

UTEST(LoadLoad, U32) {
	uint32_t i, j;
	loadload32_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (loadload32_thread*)Allocate_Memory(sizeof(loadload32_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload32_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (loadload32_entry*)Allocate_Memory(sizeof(loadload32_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(loadload32_entry));
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload32_thread* Thread = Context.Threads + i;
		Thread->Thread = AK_Thread_Create(LoadLoadU32, Thread);
	}
	for (i = 0; i < Context.ThreadCount; i++) {
		loadload32_thread* Thread = Context.Threads + i;
		
		for (j = 0; j < Thread->Count; j++) {
			loadload32_entry* Entry = Thread->Entries + j;
			AK_Atomic_Store_U32(&Entry->B, 2, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
			AK_Atomic_Store_U32(&Entry->A, 2, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		}
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload32_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->Thread);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_TRUE(Thread->Entries[j].HasPassed == true_t);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	ak_atomic_u64 A;
	ak_atomic_u64 B;
	int 		  HasProcessed;
	int 		  HasPassed;
} loadload64_entry;

typedef struct {
	ak_thread* 		  Thread;
	uint32_t 		  Count;
	loadload64_entry* Entries;
} loadload64_thread;

typedef struct {
	uint32_t 		   ThreadCount;
	loadload64_thread* Threads;
} loadload64_context;

static AK_THREAD_CALLBACK_DEFINE(LoadLoadU64) {
	uint32_t i;
	int FinishedProcessing;
	loadload64_thread* Context = (loadload64_thread*)UserData;
	(void)Thread;
	
	for (;;) {
		FinishedProcessing = true_t;

		for (i = 0; i < Context->Count; i++) {
			loadload64_entry* Entry = Context->Entries + i;
			if (!Entry->HasProcessed) {
				/*Note that the Load to Entry->A must have an acquire barrier otherwise B 
				  may be read before A and therefore it may not have the value of 2 yet*/
				uint64_t A = AK_Atomic_Load_U64(&Entry->A, AK_ATOMIC_MEMORY_ORDER_ACQUIRE);
				uint64_t B = AK_Atomic_Load_U64(&Entry->B, AK_ATOMIC_MEMORY_ORDER_RELAXED);
				if (A == 2) {
					Entry->HasProcessed = true_t;
					Entry->HasPassed = (B == 2);
				} else {
					FinishedProcessing = false_t;
				}
			}
		}

		if (FinishedProcessing) {
			return 0;
		}
	}
}

UTEST(LoadLoad, U64) {
	uint32_t i, j;
	loadload64_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (loadload64_thread*)Allocate_Memory(sizeof(loadload64_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload64_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (loadload64_entry*)Allocate_Memory(sizeof(loadload64_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(loadload64_entry));
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload64_thread* Thread = Context.Threads + i;
		Thread->Thread = AK_Thread_Create(LoadLoadU64, Thread);
	}
	for (i = 0; i < Context.ThreadCount; i++) {
		loadload64_thread* Thread = Context.Threads + i;
		
		for (j = 0; j < Thread->Count; j++) {
			loadload64_entry* Entry = Thread->Entries + j;
			AK_Atomic_Store_U64(&Entry->B, 2, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
			AK_Atomic_Store_U64(&Entry->A, 2, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		}
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload64_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->Thread);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_TRUE(Thread->Entries[j].HasPassed == true_t);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	uint8_t A;
	ak_atomic_u8 B;
	int HasPassed;
	int HasProcessed;
} storestore8_entry;

typedef struct {
	ak_thread* 		  WorkThread;
	ak_thread* 		  TestThread;
	uint32_t 		  Count;
	storestore8_entry* Entries;
} storestore8_thread;

typedef struct {
	uint32_t 		  ThreadCount;
	storestore8_thread* Threads;
} storestore8_context;

static AK_THREAD_CALLBACK_DEFINE(TestStoreStoreU8) {
	uint32_t i;
	int FinishedProcessing;
	storestore8_thread* Context = (storestore8_thread*)UserData;
	(void)Thread;

	for(;;) {
		FinishedProcessing = true_t;

		for (i = 0; i < Context->Count; i++) {
			storestore8_entry* Entry = Context->Entries + i;
			if(!Entry->HasProcessed) {
				if (AK_Atomic_Load_U8(&Entry->B, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 8) {
					Entry->HasPassed = Entry->A == 6;
					Entry->HasProcessed = true_t;
				} else {
					FinishedProcessing = false_t;
				}
			}
		}

		if (FinishedProcessing) {
			return 0;
		}
	}
}


static AK_THREAD_CALLBACK_DEFINE(StoreStoreU8) {
	uint32_t i;
	storestore8_thread* Context = (storestore8_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storestore8_entry* Entry = Context->Entries + i;
		Entry->A = 6;
		AK_Atomic_Store_U8(&Entry->B, 8, AK_ATOMIC_MEMORY_ORDER_RELEASE);
	}

	return 0;
}

UTEST(StoreStore, U8) {
	uint32_t i, j;
	storestore8_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (storestore8_thread*)Allocate_Memory(sizeof(storestore8_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		storestore8_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (storestore8_entry*)Allocate_Memory(sizeof(storestore8_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(storestore8_entry));
	}
	
	for (i = 0; i < Context.ThreadCount; i++) {
		storestore8_thread* Thread = Context.Threads + i;
		Thread->WorkThread = AK_Thread_Create(StoreStoreU8, Thread);
		Thread->TestThread = AK_Thread_Create(TestStoreStoreU8, Thread);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		storestore8_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->WorkThread);
		AK_Thread_Delete(Thread->TestThread);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_TRUE(Thread->Entries[j].HasPassed == true_t);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	uint16_t A;
	ak_atomic_u16 B;
	int HasPassed;
	int HasProcessed;
} storestore16_entry;

typedef struct {
	ak_thread* 		  WorkThread;
	ak_thread* 		  TestThread;
	uint32_t 		  Count;
	storestore16_entry* Entries;
} storestore16_thread;

typedef struct {
	uint32_t 		  ThreadCount;
	storestore16_thread* Threads;
} storestore16_context;

static AK_THREAD_CALLBACK_DEFINE(TestStoreStoreU16) {
	uint32_t i;
	int FinishedProcessing;
	storestore16_thread* Context = (storestore16_thread*)UserData;
	(void)Thread;

	for(;;) {
		FinishedProcessing = true_t;

		for (i = 0; i < Context->Count; i++) {
			storestore16_entry* Entry = Context->Entries + i;
			if(!Entry->HasProcessed) {
				if (AK_Atomic_Load_U16(&Entry->B, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 8) {
					Entry->HasPassed = Entry->A == 6;
					Entry->HasProcessed = true_t;
				} else {
					FinishedProcessing = false_t;
				}
			}
		}

		if (FinishedProcessing) {
			return 0;
		}
	}
}


static AK_THREAD_CALLBACK_DEFINE(StoreStoreU16) {
	uint32_t i;
	storestore16_thread* Context = (storestore16_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storestore16_entry* Entry = Context->Entries + i;
		Entry->A = 6;
		AK_Atomic_Store_U16(&Entry->B, 8, AK_ATOMIC_MEMORY_ORDER_RELEASE);
	}

	return 0;
}

UTEST(StoreStore, U16) {
	uint32_t i, j;
	storestore16_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (storestore16_thread*)Allocate_Memory(sizeof(storestore16_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		storestore16_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (storestore16_entry*)Allocate_Memory(sizeof(storestore16_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(storestore16_entry));
	}
	
	for (i = 0; i < Context.ThreadCount; i++) {
		storestore16_thread* Thread = Context.Threads + i;
		Thread->WorkThread = AK_Thread_Create(StoreStoreU16, Thread);
		Thread->TestThread = AK_Thread_Create(TestStoreStoreU16, Thread);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		storestore16_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->WorkThread);
		AK_Thread_Delete(Thread->TestThread);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_TRUE(Thread->Entries[j].HasPassed == true_t);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	uint32_t A;
	ak_atomic_u32 B;
	int HasPassed;
	int HasProcessed;
} storestore32_entry;

typedef struct {
	ak_thread* 		  WorkThread;
	ak_thread* 		  TestThread;
	uint32_t 		  Count;
	storestore32_entry* Entries;
} storestore32_thread;

typedef struct {
	uint32_t 		  ThreadCount;
	storestore32_thread* Threads;
} storestore32_context;

static AK_THREAD_CALLBACK_DEFINE(TestStoreStoreU32) {
	uint32_t i;
	int FinishedProcessing;
	storestore32_thread* Context = (storestore32_thread*)UserData;
	(void)Thread;

	for(;;) {
		FinishedProcessing = true_t;

		for (i = 0; i < Context->Count; i++) {
			storestore32_entry* Entry = Context->Entries + i;
			if(!Entry->HasProcessed) {
				if (AK_Atomic_Load_U32(&Entry->B, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 8) {
					Entry->HasPassed = Entry->A == 6;
					Entry->HasProcessed = true_t;
				} else {
					FinishedProcessing = false_t;
				}
			}
		}

		if (FinishedProcessing) {
			return 0;
		}
	}
}


static AK_THREAD_CALLBACK_DEFINE(StoreStoreU32) {
	uint32_t i;
	storestore32_thread* Context = (storestore32_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storestore32_entry* Entry = Context->Entries + i;
		Entry->A = 6;
		AK_Atomic_Store_U32(&Entry->B, 8, AK_ATOMIC_MEMORY_ORDER_RELEASE);
	}

	return 0;
}

UTEST(StoreStore, U32) {
	uint32_t i, j;
	storestore32_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (storestore32_thread*)Allocate_Memory(sizeof(storestore32_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		storestore32_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (storestore32_entry*)Allocate_Memory(sizeof(storestore32_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(storestore32_entry));
	}
	
	for (i = 0; i < Context.ThreadCount; i++) {
		storestore32_thread* Thread = Context.Threads + i;
		Thread->WorkThread = AK_Thread_Create(StoreStoreU32, Thread);
		Thread->TestThread = AK_Thread_Create(TestStoreStoreU32, Thread);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		storestore32_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->WorkThread);
		AK_Thread_Delete(Thread->TestThread);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_TRUE(Thread->Entries[j].HasPassed == true_t);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	uint64_t A;
	ak_atomic_u64 B;
	int HasPassed;
	int HasProcessed;
} storestore64_entry;

typedef struct {
	ak_thread* 		  WorkThread;
	ak_thread* 		  TestThread;
	uint32_t 		  Count;
	storestore64_entry* Entries;
} storestore64_thread;

typedef struct {
	uint32_t 		  ThreadCount;
	storestore64_thread* Threads;
} storestore64_context;

static AK_THREAD_CALLBACK_DEFINE(TestStoreStoreU64) {
	uint32_t i;
	int FinishedProcessing;
	storestore64_thread* Context = (storestore64_thread*)UserData;
	(void)Thread;

	for(;;) {
		FinishedProcessing = true_t;

		for (i = 0; i < Context->Count; i++) {
			storestore64_entry* Entry = Context->Entries + i;
			if(!Entry->HasProcessed) {
				if (AK_Atomic_Load_U64(&Entry->B, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 8) {
					Entry->HasPassed = Entry->A == 6;
					Entry->HasProcessed = true_t;
				} else {
					FinishedProcessing = false_t;
				}
			}
		}

		if (FinishedProcessing) {
			return 0;
		}
	}
}


static AK_THREAD_CALLBACK_DEFINE(StoreStoreU64) {
	uint32_t i;
	storestore64_thread* Context = (storestore64_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storestore64_entry* Entry = Context->Entries + i;
		Entry->A = 6;
		AK_Atomic_Store_U64(&Entry->B, 8, AK_ATOMIC_MEMORY_ORDER_RELEASE);
	}

	return 0;
}

UTEST(StoreStore, U64) {
	uint32_t i, j;
	storestore64_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (storestore64_thread*)Allocate_Memory(sizeof(storestore64_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		storestore64_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (storestore64_entry*)Allocate_Memory(sizeof(storestore64_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(storestore64_entry));
	}
	
	for (i = 0; i < Context.ThreadCount; i++) {
		storestore64_thread* Thread = Context.Threads + i;
		Thread->WorkThread = AK_Thread_Create(StoreStoreU64, Thread);
		Thread->TestThread = AK_Thread_Create(TestStoreStoreU64, Thread);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		storestore64_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->WorkThread);
		AK_Thread_Delete(Thread->TestThread);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_TRUE(Thread->Entries[j].HasPassed == true_t);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	ak_atomic_u8 X;
	ak_atomic_u8 Y;
	uint8_t R1;
	uint8_t R2;
} storeload8_entry;

typedef struct {
	uint32_t Count;
	storeload8_entry* Entries;
	ak_thread* ThreadA;
	ak_thread* ThreadB;
} storeload8_thread;

typedef struct {
	uint32_t ThreadCount;
	storeload8_thread* Threads;
} storeload8_context;

static AK_THREAD_CALLBACK_DEFINE(StoreLoadU8_A) {
	uint32_t i;
	storeload8_thread* Context = (storeload8_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storeload8_entry* Entry = Context->Entries + i;
		AK_Atomic_Store_U8(&Entry->X, 1, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		Entry->R1 = AK_Atomic_Load_U8(&Entry->Y, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
	}

	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(StoreLoadU8_B) {
	uint32_t i;
	storeload8_thread* Context = (storeload8_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storeload8_entry* Entry = Context->Entries + i;
		AK_Atomic_Store_U8(&Entry->Y, 1, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		Entry->R2 = AK_Atomic_Load_U8(&Entry->X, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
	}

	return 0;
}

UTEST(StoreLoad, U8) {
	uint32_t i, j;
	storeload8_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (storeload8_thread*)Allocate_Memory(sizeof(storeload8_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		storeload8_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (storeload8_entry*)Allocate_Memory(sizeof(storeload8_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(storeload8_entry));
	}
	
	for (i = 0; i < Context.ThreadCount; i++) {
		storeload8_thread* Thread = Context.Threads + i;
		Thread->ThreadA = AK_Thread_Create(StoreLoadU8_A, Thread);
		Thread->ThreadB = AK_Thread_Create(StoreLoadU8_B, Thread);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		storeload8_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->ThreadA);
		AK_Thread_Delete(Thread->ThreadB);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_FALSE(Thread->Entries[j].R1 == 0 && Thread->Entries[j].R2 == 0);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	ak_atomic_u16 X;
	ak_atomic_u16 Y;
	uint16_t R1;
	uint16_t R2;
} storeload16_entry;

typedef struct {
	uint32_t Count;
	storeload16_entry* Entries;
	ak_thread* ThreadA;
	ak_thread* ThreadB;
} storeload16_thread;

typedef struct {
	uint32_t ThreadCount;
	storeload16_thread* Threads;
} storeload16_context;

static AK_THREAD_CALLBACK_DEFINE(StoreLoadU16_A) {
	uint32_t i;
	storeload16_thread* Context = (storeload16_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storeload16_entry* Entry = Context->Entries + i;
		AK_Atomic_Store_U16(&Entry->X, 1, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		Entry->R1 = AK_Atomic_Load_U16(&Entry->Y, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
	}

	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(StoreLoadU16_B) {
	uint32_t i;
	storeload16_thread* Context = (storeload16_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storeload16_entry* Entry = Context->Entries + i;
		AK_Atomic_Store_U16(&Entry->Y, 1, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		Entry->R2 = AK_Atomic_Load_U16(&Entry->X, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
	}

	return 0;
}

UTEST(StoreLoad, U16) {
	uint32_t i, j;
	storeload16_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (storeload16_thread*)Allocate_Memory(sizeof(storeload16_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		storeload16_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (storeload16_entry*)Allocate_Memory(sizeof(storeload16_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(storeload16_entry));
	}
	
	for (i = 0; i < Context.ThreadCount; i++) {
		storeload16_thread* Thread = Context.Threads + i;
		Thread->ThreadA = AK_Thread_Create(StoreLoadU16_A, Thread);
		Thread->ThreadB = AK_Thread_Create(StoreLoadU16_B, Thread);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		storeload16_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->ThreadA);
		AK_Thread_Delete(Thread->ThreadB);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_FALSE(Thread->Entries[j].R1 == 0 && Thread->Entries[j].R2 == 0);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	ak_atomic_u32 X;
	ak_atomic_u32 Y;
	uint32_t R1;
	uint32_t R2;
} storeload32_entry;

typedef struct {
	uint32_t Count;
	storeload32_entry* Entries;
	ak_thread* ThreadA;
	ak_thread* ThreadB;
} storeload32_thread;

typedef struct {
	uint32_t ThreadCount;
	storeload32_thread* Threads;
} storeload32_context;

static AK_THREAD_CALLBACK_DEFINE(StoreLoadU32_A) {
	uint32_t i;
	storeload32_thread* Context = (storeload32_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storeload32_entry* Entry = Context->Entries + i;
		AK_Atomic_Store_U32(&Entry->X, 1, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		Entry->R1 = AK_Atomic_Load_U32(&Entry->Y, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
	}

	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(StoreLoadU32_B) {
	uint32_t i;
	storeload32_thread* Context = (storeload32_thread*)UserData;
	(void)Thread;
	for(i = 0; i < Context->Count; i++) {
		storeload32_entry* Entry = Context->Entries + i;
		AK_Atomic_Store_U32(&Entry->Y, 1, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		Entry->R2 = AK_Atomic_Load_U32(&Entry->X, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
	}

	return 0;
}

UTEST(StoreLoad, U32) {
	uint32_t i, j;
	storeload32_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (storeload32_thread*)Allocate_Memory(sizeof(storeload32_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		storeload32_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (storeload32_entry*)Allocate_Memory(sizeof(storeload32_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(storeload32_entry));
	}
	
	for (i = 0; i < Context.ThreadCount; i++) {
		storeload32_thread* Thread = Context.Threads + i;
		Thread->ThreadA = AK_Thread_Create(StoreLoadU32_A, Thread);
		Thread->ThreadB = AK_Thread_Create(StoreLoadU32_B, Thread);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		storeload32_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->ThreadA);
		AK_Thread_Delete(Thread->ThreadB);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_FALSE(Thread->Entries[j].R1 == 0 && Thread->Entries[j].R2 == 0);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct {
	ak_atomic_u64 X;
	ak_atomic_u64 Y;
	uint64_t R1;
	uint64_t R2;
} storeload64_entry;

typedef struct {
	uint32_t Count;
	storeload64_entry* Entries;
	ak_thread* ThreadA;
	ak_thread* ThreadB;
} storeload64_thread;

typedef struct {
	uint32_t ThreadCount;
	storeload64_thread* Threads;
} storeload64_context;

static AK_THREAD_CALLBACK_DEFINE(StoreLoadU64_A) {
	uint32_t i;
	storeload64_thread* Context = (storeload64_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storeload64_entry* Entry = Context->Entries + i;
		AK_Atomic_Store_U64(&Entry->X, 1, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		Entry->R1 = AK_Atomic_Load_U64(&Entry->Y, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
	}

	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(StoreLoadU64_B) {
	uint32_t i;
	storeload64_thread* Context = (storeload64_thread*)UserData;
	(void)Thread;

	for(i = 0; i < Context->Count; i++) {
		storeload64_entry* Entry = Context->Entries + i;
		AK_Atomic_Store_U64(&Entry->Y, 1, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
		Entry->R2 = AK_Atomic_Load_U64(&Entry->X, AK_ATOMIC_MEMORY_ORDER_SEQ_CST);
	}

	return 0;
}

UTEST(StoreLoad, U64) {
	uint32_t i, j;
	storeload64_context Context;

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (storeload64_thread*)Allocate_Memory(sizeof(storeload64_thread)*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		storeload64_thread* Thread = Context.Threads + i;
		
		Thread->Count = 100000;
		Thread->Entries = (storeload64_entry*)Allocate_Memory(sizeof(storeload64_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(storeload64_entry));
	}
	
	for (i = 0; i < Context.ThreadCount; i++) {
		storeload64_thread* Thread = Context.Threads + i;
		Thread->ThreadA = AK_Thread_Create(StoreLoadU64_A, Thread);
		Thread->ThreadB = AK_Thread_Create(StoreLoadU64_B, Thread);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		storeload64_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->ThreadA);
		AK_Thread_Delete(Thread->ThreadB);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_FALSE(Thread->Entries[j].R1 == 0 && Thread->Entries[j].R2 == 0);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

typedef struct exchange8_context exchange8_context;

typedef struct {
	ak_thread*         Thread;
	uint8_t 		   Base;
	exchange8_context* Context;
} exchange8_thread;

struct exchange8_context {
	uint8_t ThreadCount;
	exchange8_thread* Threads;

	uint8_t ValuesPerThread;
	uint8_t* Values;

	ak_atomic_u8 SharedInt;
};

int32_t Exchange_CompareU8(const void* A, const void* B) {
    uint8_t AValue = *((const uint8_t*)A);
    uint8_t BValue = *((const uint8_t*)B);
    return (int32_t)BValue - (int32_t)AValue;
}

static AK_THREAD_CALLBACK_DEFINE(ExchangeU8) {
	uint8_t i;
	exchange8_thread* ExThread = (exchange8_thread*)UserData;
	exchange8_context* Context = ExThread->Context;
	(void)Thread;

	for(i = 1; i <= Context->ValuesPerThread; i++) {
		uint8_t Index = (uint8_t)(i+ExThread->Base);
		uint8_t OldValue = AK_Atomic_Exchange_U8(&Context->SharedInt, Index, AK_ATOMIC_MEMORY_ORDER_RELAXED);
		Context->Values[Index] = OldValue;
	}

	return 0;
}

UTEST(Exchange, U8) {
	uint8_t i;
	exchange8_context Context;
	Memory_Clear(&Context, sizeof(exchange8_context));

	Context.ThreadCount = (uint8_t)AK_Get_Processor_Thread_Count();
	Context.Threads = (exchange8_thread*)Allocate_Memory(sizeof(exchange8_thread)*Context.ThreadCount);
	Context.ValuesPerThread = 254u/Context.ThreadCount; /*Truncating down is fine*/
	uint8_t NumValues = (Context.ValuesPerThread*Context.ThreadCount)+1u;
	Assert(NumValues <= 255);

	Context.Values = (uint8_t*)Allocate_Memory(sizeof(uint8_t)*NumValues);
	Memory_Clear(Context.Values, sizeof(uint8_t)*NumValues);

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange8_thread* Thread = Context.Threads + i;
		Thread->Base = (uint8_t)(Context.ValuesPerThread*i);
		Thread->Context = &Context;
		Thread->Thread = AK_Thread_Create(ExchangeU8, Thread);
	}

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange8_thread* Thread = Context.Threads + i;
		AK_Thread_Wait(Thread->Thread);
	}

	Context.Values[0] = AK_Atomic_Load_U8(&Context.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	qsort(Context.Values, NumValues, sizeof(uint8_t), Exchange_CompareU8);

	for(i = 0; i < NumValues; i++) {
		uint8_t ActualIndex = (NumValues-i)-1u;
		ASSERT_TRUE(Context.Values[ActualIndex] == i);
	}

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange8_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->Thread);
	}

	Free_Memory(Context.Values);
	Free_Memory(Context.Threads);
}

typedef struct exchange16_context exchange16_context;

typedef struct {
	ak_thread*          Thread;
	uint16_t 		    Base;
	exchange16_context* Context;
} exchange16_thread;

struct exchange16_context {
	uint16_t ThreadCount;
	exchange16_thread* Threads;

	uint16_t ValuesPerThread;
	uint16_t* Values;

	ak_atomic_u16 SharedInt;
};

int32_t Exchange_CompareU16(const void* A, const void* B) {
    uint16_t AValue = *((const uint16_t*)A);
    uint16_t BValue = *((const uint16_t*)B);
    return (int32_t)BValue - (int32_t)AValue;
}

static AK_THREAD_CALLBACK_DEFINE(ExchangeU16) {
	uint16_t i;
	exchange16_thread* ExThread = (exchange16_thread*)UserData;
	exchange16_context* Context = ExThread->Context;
	(void)Thread;

	for(i = 1; i <= Context->ValuesPerThread; i++) {
		uint16_t Index = (uint16_t)(i+ExThread->Base);
		uint16_t OldValue = AK_Atomic_Exchange_U16(&Context->SharedInt, Index, AK_ATOMIC_MEMORY_ORDER_RELAXED);
		Context->Values[Index] = OldValue;
	}

	return 0;
}

UTEST(Exchange, U16) {
	uint16_t i;
	exchange16_context Context;
	Memory_Clear(&Context, sizeof(exchange16_context));

	Context.ThreadCount = (uint16_t)AK_Get_Processor_Thread_Count();
	Context.Threads = (exchange16_thread*)Allocate_Memory(sizeof(exchange16_thread)*Context.ThreadCount);
	Context.ValuesPerThread = 65534u/Context.ThreadCount; /*Truncating down is fine*/
	uint16_t NumValues = (Context.ValuesPerThread*Context.ThreadCount)+1u;
	Assert(NumValues <= 65535);

	Context.Values = (uint16_t*)Allocate_Memory(sizeof(uint16_t)*NumValues);
	Memory_Clear(Context.Values, sizeof(uint16_t)*NumValues);

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange16_thread* Thread = Context.Threads + i;
		Thread->Base = (uint16_t)(Context.ValuesPerThread*i);
		Thread->Context = &Context;
		Thread->Thread = AK_Thread_Create(ExchangeU16, Thread);
	}

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange16_thread* Thread = Context.Threads + i;
		AK_Thread_Wait(Thread->Thread);
	}

	Context.Values[0] = AK_Atomic_Load_U16(&Context.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	qsort(Context.Values, NumValues, sizeof(uint16_t), Exchange_CompareU16);

	for(i = 0; i < NumValues; i++) {
		uint16_t ActualIndex = (NumValues-i)-1u;
		ASSERT_TRUE(Context.Values[ActualIndex] == i);
	}

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange16_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->Thread);
	}

	Free_Memory(Context.Values);
	Free_Memory(Context.Threads);
}

typedef struct exchange32_context exchange32_context;

typedef struct {
	ak_thread*          Thread;
	uint32_t 		    Base;
	exchange32_context* Context;
} exchange32_thread;

struct exchange32_context {
	uint32_t ThreadCount;
	exchange32_thread* Threads;

	uint32_t ValuesPerThread;
	uint32_t* Values;

	ak_atomic_u32 SharedInt;
};

int32_t Exchange_CompareU32(const void* A, const void* B) {
    uint32_t AValue = *((const uint32_t*)A);
    uint32_t BValue = *((const uint32_t*)B);
    return (int32_t)BValue - (int32_t)AValue;
}

static AK_THREAD_CALLBACK_DEFINE(ExchangeU32) {
	uint32_t i;
	exchange32_thread* ExThread = (exchange32_thread*)UserData;
	exchange32_context* Context = ExThread->Context;
	(void)Thread;

	for(i = 1; i <= Context->ValuesPerThread; i++) {
		uint32_t Index = i+ExThread->Base;
		uint32_t OldValue = AK_Atomic_Exchange_U32(&Context->SharedInt, Index, AK_ATOMIC_MEMORY_ORDER_RELAXED);
		Context->Values[Index] = OldValue;
	}

	return 0;
}

UTEST(Exchange, U32) {
	uint32_t i;
	exchange32_context Context;
	Memory_Clear(&Context, sizeof(exchange32_context));

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (exchange32_thread*)Allocate_Memory(sizeof(exchange32_thread)*Context.ThreadCount);
	Context.ValuesPerThread = 100000; /*Truncating down is fine*/
	uint32_t NumValues = (Context.ValuesPerThread*Context.ThreadCount)+1;

	Context.Values = (uint32_t*)Allocate_Memory(sizeof(uint32_t)*NumValues);
	Memory_Clear(Context.Values, sizeof(uint32_t)*NumValues);

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange32_thread* Thread = Context.Threads + i;
		Thread->Base = Context.ValuesPerThread*i;
		Thread->Context = &Context;
		Thread->Thread = AK_Thread_Create(ExchangeU32, Thread);
	}

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange32_thread* Thread = Context.Threads + i;
		AK_Thread_Wait(Thread->Thread);
	}

	Context.Values[0] = AK_Atomic_Load_U32(&Context.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	qsort(Context.Values, NumValues, sizeof(uint32_t), Exchange_CompareU32);

	for(i = 0; i < NumValues; i++) {
		uint32_t ActualIndex = (NumValues-i)-1;
		ASSERT_TRUE(Context.Values[ActualIndex] == i);
	}

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange32_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->Thread);
	}

	Free_Memory(Context.Values);
	Free_Memory(Context.Threads);
}

typedef struct exchange64_context exchange64_context;

typedef struct {
	ak_thread*          Thread;
	uint32_t 		    Base;
	exchange64_context* Context;
} exchange64_thread;

struct exchange64_context {
	uint32_t ThreadCount;
	exchange64_thread* Threads;

	uint32_t ValuesPerThread;
	uint64_t* Values;

	ak_atomic_u64 SharedInt;
};

int32_t Exchange_CompareU64(const void* A, const void* B) {
    uint64_t AValue = *((const uint64_t*)A);
    uint64_t BValue = *((const uint64_t*)B);
	if(BValue > AValue) return 1;
	if(BValue < AValue) return -1;
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(ExchangeU64) {
	uint32_t i;
	exchange64_thread* ExThread = (exchange64_thread*)UserData;
	exchange64_context* Context = ExThread->Context;
	(void)Thread;

	for(i = 1; i <= Context->ValuesPerThread; i++) {
		uint64_t Index = i+ExThread->Base;
		uint64_t OldValue = AK_Atomic_Exchange_U64(&Context->SharedInt, Index, AK_ATOMIC_MEMORY_ORDER_RELAXED);
		Context->Values[Index] = OldValue;
	}

	return 0;
}

UTEST(Exchange, U64) {
	uint32_t i;
	exchange64_context Context;
	Memory_Clear(&Context, sizeof(exchange64_context));

	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (exchange64_thread*)Allocate_Memory(sizeof(exchange64_thread)*Context.ThreadCount);
	Context.ValuesPerThread = 100000; /*Truncating down is fine*/
	uint32_t NumValues = (Context.ValuesPerThread*Context.ThreadCount)+1;

	Context.Values = (uint64_t*)Allocate_Memory(sizeof(uint64_t)*NumValues);
	Memory_Clear(Context.Values, sizeof(uint64_t)*NumValues);

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange64_thread* Thread = Context.Threads + i;
		Thread->Base = Context.ValuesPerThread*i;
		Thread->Context = &Context;
		Thread->Thread = AK_Thread_Create(ExchangeU64, Thread);
	}

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange64_thread* Thread = Context.Threads + i;
		AK_Thread_Wait(Thread->Thread);
	}

	Context.Values[0] = AK_Atomic_Load_U64(&Context.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	qsort(Context.Values, NumValues, sizeof(uint64_t), Exchange_CompareU64);

	for(i = 0; i < NumValues; i++) {
		uint64_t ActualIndex = (NumValues-i)-1;
		ASSERT_TRUE(Context.Values[ActualIndex] == i);
	}

	for(i = 0; i < Context.ThreadCount; i++) {
		exchange64_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->Thread);
	}

	Free_Memory(Context.Values);
	Free_Memory(Context.Threads);
}

typedef struct {
	uint32_t Iterations;
	ak_atomic_u8 Flag;
	uint32_t SharedValue;
} compare_exchange8_context;

static AK_THREAD_CALLBACK_DEFINE(CompareExchangeU8) {
	compare_exchange8_context* Context = (compare_exchange8_context*)UserData;
	(void)Thread;

	uint32_t Count = 0;
    while(Count < Context->Iterations) {
        if(AK_Atomic_Compare_Exchange_Strong_U8(&Context->Flag, 0, 1, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 0) {
            Context->SharedValue++;
            Count++;
            AK_Atomic_Store_U8(&Context->Flag, 0, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        }
    }
    return 0;
}

UTEST(CompareExchange, U8) {
	uint32_t i;
	compare_exchange8_context Context;

	Memory_Clear(&Context, sizeof(compare_exchange8_context));

	Context.Iterations = 1000;
	uint32_t NumThreads = AK_Get_Processor_Thread_Count();

	ak_thread** Threads = (ak_thread**)Allocate_Memory(sizeof(ak_thread*)*NumThreads);
	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(CompareExchangeU8, &Context);
	}

	for(i = 0; i < NumThreads; i++) {
		AK_Thread_Wait(Threads[i]);
	}

	ASSERT_TRUE(Context.SharedValue == Context.Iterations*NumThreads);

	for(i = 0; i < NumThreads; i++) {
		AK_Thread_Delete(Threads[i]);
	}

	Free_Memory(Threads);
}

typedef struct {
	uint32_t Iterations;
	ak_atomic_u16 Flag;
	uint32_t SharedValue;
} compare_exchange16_context;

static AK_THREAD_CALLBACK_DEFINE(CompareExchangeU16) {
	compare_exchange16_context* Context = (compare_exchange16_context*)UserData;
	(void)Thread;

	uint32_t Count = 0;
    while(Count < Context->Iterations) {
        if(AK_Atomic_Compare_Exchange_Strong_U16(&Context->Flag, 0, 1, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 0) {
            Context->SharedValue++;
            Count++;
            AK_Atomic_Store_U16(&Context->Flag, 0, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        }
    }
    return 0;
}

UTEST(CompareExchange, U16) {
	uint32_t i;
	compare_exchange16_context Context;
	Memory_Clear(&Context, sizeof(compare_exchange16_context));

	Context.Iterations = 1000;
	uint32_t NumThreads = AK_Get_Processor_Thread_Count();

	ak_thread** Threads = (ak_thread**)Allocate_Memory(sizeof(ak_thread*)*NumThreads);
	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(CompareExchangeU16, &Context);
	}

	for(i = 0; i < NumThreads; i++) {
		AK_Thread_Wait(Threads[i]);
	}

	ASSERT_TRUE(Context.SharedValue == Context.Iterations*NumThreads);

	for(i = 0; i < NumThreads; i++) {
		AK_Thread_Delete(Threads[i]);
	}

	Free_Memory(Threads);
}

typedef struct {
	uint32_t Iterations;
	ak_atomic_u32 Flag;
	uint32_t SharedValue;
} compare_exchange32_context;

static AK_THREAD_CALLBACK_DEFINE(CompareExchangeU32) {
	compare_exchange32_context* Context = (compare_exchange32_context*)UserData;
	(void)Thread;

	uint32_t Count = 0;
    while(Count < Context->Iterations) {
        if(AK_Atomic_Compare_Exchange_Strong_U32(&Context->Flag, 0, 1, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 0) {
            Context->SharedValue++;
            Count++;
            AK_Atomic_Store_U32(&Context->Flag, 0, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        }
    }
    return 0;
}

UTEST(CompareExchange, U32) {
	uint32_t i;
	compare_exchange32_context Context;
	Memory_Clear(&Context, sizeof(compare_exchange32_context));

	Context.Iterations = 1000;
	uint32_t NumThreads = AK_Get_Processor_Thread_Count();

	ak_thread** Threads = (ak_thread**)Allocate_Memory(sizeof(ak_thread*)*NumThreads);
	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(CompareExchangeU32, &Context);
	}

	for(i = 0; i < NumThreads; i++) {
		AK_Thread_Wait(Threads[i]);
	}

	ASSERT_TRUE(Context.SharedValue == Context.Iterations*NumThreads);

	for(i = 0; i < NumThreads; i++) {
		AK_Thread_Delete(Threads[i]);
	}

	Free_Memory(Threads);
}

typedef struct {
	uint32_t Iterations;
	ak_atomic_u64 Flag;
	uint32_t SharedValue;
} compare_exchange64_context;

static AK_THREAD_CALLBACK_DEFINE(CompareExchangeU64) {
	compare_exchange64_context* Context = (compare_exchange64_context*)UserData;
	(void)Thread;

	uint32_t Count = 0;
    while(Count < Context->Iterations) {
        if(AK_Atomic_Compare_Exchange_Strong_U64(&Context->Flag, 0, 1, AK_ATOMIC_MEMORY_ORDER_ACQUIRE) == 0) {
            Context->SharedValue++;
            Count++;
            AK_Atomic_Store_U64(&Context->Flag, 0, AK_ATOMIC_MEMORY_ORDER_RELEASE);
        }
    }
    return 0;
}

UTEST(CompareExchange, U64) {
	uint32_t i;
	compare_exchange64_context Context;
	Memory_Clear(&Context, sizeof(compare_exchange64_context));

	Context.Iterations = 1000;
	uint32_t NumThreads = AK_Get_Processor_Thread_Count();

	ak_thread** Threads = (ak_thread**)Allocate_Memory(sizeof(ak_thread*)*NumThreads);
	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(CompareExchangeU64, &Context);
	}

	for(i = 0; i < NumThreads; i++) {
		AK_Thread_Wait(Threads[i]);
	}

	ASSERT_TRUE(Context.SharedValue == Context.Iterations*NumThreads);

	for(i = 0; i < NumThreads; i++) {
		AK_Thread_Delete(Threads[i]);
	}

	Free_Memory(Threads);
}

UTEST(FetchReturnValues, U8) {
	uint32_t i;

	ak_atomic_u8 Object;
	AK_Atomic_Store_U8(&Object, 0, AK_ATOMIC_MEMORY_ORDER_RELAXED);

	uint8_t Mirror = 0;
	uint8_t Operand, Expected;
	for(i = 0; i < 1000; i++) {
		Expected = (Random_U32() & 1) != 0 ? Mirror : Random_U8();
		Operand = Random_U8();
		if(AK_Atomic_Compare_Exchange_Strong_U8(&Object, Expected, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		
		if(Expected == Mirror)
			Mirror = Operand;

		Operand = Random_U8();
		if(AK_Atomic_Fetch_Add_U8(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror += Operand;

		Operand = Random_U8();
		if(AK_Atomic_Fetch_And_U8(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror &= Operand;

		Operand = Random_U8();
		if(AK_Atomic_Fetch_Or_U8(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror |= Operand;
	}
}

UTEST(FetchReturnValues, U16) {
	uint32_t i;

	ak_atomic_u16 Object;
	AK_Atomic_Store_U16(&Object, 0, AK_ATOMIC_MEMORY_ORDER_RELAXED);

	uint16_t Mirror = 0;
	uint16_t Operand, Expected;
	for(i = 0; i < 1000; i++) {
		Expected = (Random_U32() & 1) != 0 ? Mirror : Random_U16();
		Operand = Random_U16();
		if(AK_Atomic_Compare_Exchange_Strong_U16(&Object, Expected, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		
		if(Expected == Mirror)
			Mirror = Operand;

		Operand = Random_U16();
		if(AK_Atomic_Fetch_Add_U16(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror += Operand;

		Operand = Random_U16();
		if(AK_Atomic_Fetch_And_U16(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror &= Operand;

		Operand = Random_U16();
		if(AK_Atomic_Fetch_Or_U16(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror |= Operand;
	}
}

UTEST(FetchReturnValues, U32) {
	uint32_t i;

	ak_atomic_u32 Object;
	AK_Atomic_Store_U32(&Object, 0, AK_ATOMIC_MEMORY_ORDER_RELAXED);

	uint32_t Mirror = 0;
	uint32_t Operand, Expected;
	for(i = 0; i < 1000; i++) {
		Expected = (Random_U32() & 1) != 0 ? Mirror : Random_U32();
		Operand = Random_U32();
		if(AK_Atomic_Compare_Exchange_Strong_U32(&Object, Expected, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		
		if(Expected == Mirror)
			Mirror = Operand;

		Operand = Random_U32();
		if(AK_Atomic_Fetch_Add_U32(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror += Operand;

		Operand = Random_U32();
		if(AK_Atomic_Fetch_And_U32(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror &= Operand;

		Operand = Random_U32();
		if(AK_Atomic_Fetch_Or_U32(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror |= Operand;
	}
}

UTEST(FetchReturnValues, U64) {
	uint32_t i;

	ak_atomic_u64 Object;
	AK_Atomic_Store_U64(&Object, 0, AK_ATOMIC_MEMORY_ORDER_RELAXED);

	uint64_t Mirror = 0;
	uint64_t Operand, Expected;
	for(i = 0; i < 1000; i++) {
		Expected = (Random_U32() & 1) != 0 ? Mirror : Random_U64();
		Operand = Random_U64();
		if(AK_Atomic_Compare_Exchange_Strong_U64(&Object, Expected, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		
		if(Expected == Mirror)
			Mirror = Operand;

		Operand = Random_U16();
		if(AK_Atomic_Fetch_Add_U64(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror += Operand;

		Operand = Random_U16();
		if(AK_Atomic_Fetch_And_U64(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror &= Operand;

		Operand = Random_U16();
		if(AK_Atomic_Fetch_Or_U64(&Object, Operand, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Mirror)
			ASSERT_FALSE(!"Failed");
		Mirror |= Operand;
	}
}

typedef struct {
    uint8_t      Iterations;
    ak_atomic_u8 SharedInt;
} increment8_data;

static AK_THREAD_CALLBACK_DEFINE(IncrementU8) {
	increment8_data* Data = (increment8_data*)UserData;
	(void)Thread;

	uint8_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Increment_U8(&Data->SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(DecrementU8) {
	increment8_data* Data = (increment8_data*)UserData;
	(void)Thread;

	uint8_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Decrement_U8(&Data->SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(FetchAddU8) {
	increment8_data* Data = (increment8_data*)UserData;
	(void)Thread;

	uint8_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Fetch_Add_U8(&Data->SharedInt, 1, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(FetchSubU8) {
	increment8_data* Data = (increment8_data*)UserData;
	(void)Thread;
	uint8_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Fetch_Sub_U8(&Data->SharedInt, 1, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

UTEST(Increment, U8) {
	increment8_data Data;
	Memory_Clear(&Data, sizeof(increment8_data));

	uint8_t NumThreads = (uint8_t)AK_Get_Processor_Thread_Count();
	Data.Iterations = 255u/NumThreads;
	uint8_t TotalAmount = (uint8_t)(Data.Iterations*NumThreads);
	
	ak_thread** Threads = (ak_thread**)Allocate_Memory(sizeof(ak_thread*)*NumThreads);

	uint64_t i;
	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(IncrementU8, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U8(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == TotalAmount);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(DecrementU8, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U8(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == 0);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(FetchAddU8, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U8(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == TotalAmount);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(FetchSubU8, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U8(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == 0);

	Free_Memory(Threads);
}

typedef struct {
    uint16_t      Iterations;
    ak_atomic_u16 SharedInt;
} increment16_data;

static AK_THREAD_CALLBACK_DEFINE(IncrementU16) {
	increment16_data* Data = (increment16_data*)UserData;
	(void)Thread;
	uint16_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Increment_U16(&Data->SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(DecrementU16) {
	increment16_data* Data = (increment16_data*)UserData;
	(void)Thread;
	uint16_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Decrement_U16(&Data->SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(FetchAddU16) {
	increment16_data* Data = (increment16_data*)UserData;
	(void)Thread;
	uint16_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Fetch_Add_U16(&Data->SharedInt, 1, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(FetchSubU16) {
	increment16_data* Data = (increment16_data*)UserData;
	(void)Thread;
	uint16_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Fetch_Sub_U16(&Data->SharedInt, 1, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

UTEST(Increment, U16) {
	increment16_data Data;
	Memory_Clear(&Data, sizeof(increment16_data));

	uint16_t NumThreads = (uint16_t)AK_Get_Processor_Thread_Count();
	Data.Iterations = 65535u/NumThreads;
	uint16_t TotalAmount = (uint16_t)(Data.Iterations*NumThreads);
	
	ak_thread** Threads = (ak_thread**)Allocate_Memory(sizeof(ak_thread*)*NumThreads);

	uint64_t i;
	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(IncrementU16, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U16(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == TotalAmount);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(DecrementU16, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U16(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == 0);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(FetchAddU16, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U16(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == TotalAmount);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(FetchSubU16, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U16(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == 0);

	Free_Memory(Threads);
}

typedef struct {
    uint32_t      Iterations;
    ak_atomic_u32 SharedInt;
} increment32_data;

static AK_THREAD_CALLBACK_DEFINE(IncrementU32) {
	increment32_data* Data = (increment32_data*)UserData;
	(void)Thread;
	uint32_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Increment_U32(&Data->SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(DecrementU32) {
	increment32_data* Data = (increment32_data*)UserData;
	(void)Thread;
	uint32_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Decrement_U32(&Data->SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(FetchAddU32) {
	increment32_data* Data = (increment32_data*)UserData;
	(void)Thread;
	uint32_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Fetch_Add_U32(&Data->SharedInt, 1, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(FetchSubU32) {
	increment32_data* Data = (increment32_data*)UserData;
	(void)Thread;
	uint32_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Fetch_Sub_U32(&Data->SharedInt, 1, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

UTEST(Increment, U32) {
	increment32_data Data;
	Memory_Clear(&Data, sizeof(increment32_data));

	uint32_t NumThreads = AK_Get_Processor_Thread_Count();
	Data.Iterations = 100000;
	uint32_t TotalAmount = Data.Iterations*NumThreads;
	
	ak_thread** Threads = (ak_thread**)Allocate_Memory(sizeof(ak_thread*)*NumThreads);

	uint64_t i;
	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(IncrementU32, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U32(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == TotalAmount);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(DecrementU32, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U32(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == 0);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(FetchAddU32, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U32(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == TotalAmount);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(FetchSubU32, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U32(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == 0);

	Free_Memory(Threads);
}

typedef struct {
    uint64_t      Iterations;
    ak_atomic_u64 SharedInt;
} increment64_data;

static AK_THREAD_CALLBACK_DEFINE(IncrementU64) {
	increment64_data* Data = (increment64_data*)UserData;
	(void)Thread;
	uint64_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Increment_U64(&Data->SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(DecrementU64) {
	increment64_data* Data = (increment64_data*)UserData;
	(void)Thread;
	uint64_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Decrement_U64(&Data->SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(FetchAddU64) {
	increment64_data* Data = (increment64_data*)UserData;
	(void)Thread;
	uint64_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Fetch_Add_U64(&Data->SharedInt, 1, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

static AK_THREAD_CALLBACK_DEFINE(FetchSubU64) {
	increment64_data* Data = (increment64_data*)UserData;
	(void)Thread;
	uint64_t i;
	for(i = 0; i < Data->Iterations; i++) {
		AK_Atomic_Fetch_Sub_U64(&Data->SharedInt, 1, AK_ATOMIC_MEMORY_ORDER_RELAXED);
	}
	return 0;
}

UTEST(Increment, U64) {
	increment64_data Data;
	Memory_Clear(&Data, sizeof(increment64_data));

	uint32_t NumThreads = AK_Get_Processor_Thread_Count();
	Data.Iterations = 100000;
	uint64_t TotalAmount = Data.Iterations*NumThreads;
	
	ak_thread** Threads = (ak_thread**)Allocate_Memory(sizeof(ak_thread*)*NumThreads);

	uint64_t i;

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(IncrementU64, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U64(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == TotalAmount);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(DecrementU64, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U64(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == 0);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(FetchAddU64, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U64(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == TotalAmount);

	for(i = 0; i < NumThreads; i++) {
		Threads[i] = AK_Thread_Create(FetchSubU64, &Data);
	}

	for(i = 0; i < NumThreads; i++) {
        AK_Thread_Delete(Threads[i]);
    }

	ASSERT_TRUE(AK_Atomic_Load_U64(&Data.SharedInt, AK_ATOMIC_MEMORY_ORDER_RELAXED) == 0);

	Free_Memory(Threads);
}

typedef struct compare_exchange_link_list_node compare_exchange_link_list_node;
struct compare_exchange_link_list_node {
	compare_exchange_link_list_node* Next;
};

typedef struct {
	compare_exchange_link_list_node* Nodes;
	ak_thread* Thread;
	ak_atomic_ptr* ListHead;
	uint32_t Iterations;
} compare_exchange_link_list_thread;

typedef struct {
	uint32_t ThreadCount;
	compare_exchange_link_list_thread* Threads;
} compare_exchange_link_list_context;

static AK_THREAD_CALLBACK_DEFINE(CompareExchangeLinkList) {
	compare_exchange_link_list_thread* Context = (compare_exchange_link_list_thread *)UserData;
	compare_exchange_link_list_node* Nodes = Context->Nodes;
	ak_atomic_ptr* ListHead = Context->ListHead;

	uint32_t i; 
    for(i = 0; i < Context->Iterations; i++) {
        compare_exchange_link_list_node* Insert = Nodes + i;
        compare_exchange_link_list_node* Head;
        do {
            Head = (compare_exchange_link_list_node*)AK_Atomic_Load_Ptr(ListHead, AK_ATOMIC_MEMORY_ORDER_RELAXED);
            Insert->Next = Head;
        } while(AK_Atomic_Compare_Exchange_Weak_Ptr(ListHead, Head, Insert, AK_ATOMIC_MEMORY_ORDER_RELAXED) != Head);
    }

    return 0;
}

UTEST(CompareExchange, LinkList) {
	uint32_t Iterations = 100000;

	compare_exchange_link_list_context Context;
	Context.ThreadCount = AK_Get_Processor_Thread_Count();
	Context.Threads = (compare_exchange_link_list_thread *)Allocate_Memory(sizeof(compare_exchange_link_list_thread)*Context.ThreadCount);
	Memory_Clear(Context.Threads, sizeof(compare_exchange_link_list_thread)*Context.ThreadCount);

	ak_atomic_ptr ListHead;
	Memory_Clear(&ListHead, sizeof(ak_atomic_ptr));

	uint32_t i;
	for (i = 0; i < Context.ThreadCount; i++) {
		compare_exchange_link_list_thread* Thread = Context.Threads + i;
		Thread->Nodes = (compare_exchange_link_list_node*)Allocate_Memory(sizeof(compare_exchange_link_list_node)*Iterations);
		Thread->ListHead = &ListHead;
		Thread->Iterations = Iterations;
		Memory_Clear(Thread->Nodes, sizeof(compare_exchange_link_list_node)*Iterations);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		compare_exchange_link_list_thread* Thread = Context.Threads + i;
		Thread->Thread = AK_Thread_Create(CompareExchangeLinkList, Thread);
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		AK_Thread_Wait(Context.Threads[i].Thread);
	}

    uint32_t Count = 0;
    compare_exchange_link_list_node* Node = (compare_exchange_link_list_node*)AK_Atomic_Load_Ptr(&ListHead, AK_ATOMIC_MEMORY_ORDER_RELAXED);
    while(Node) {
        AK_Atomic_Store_Ptr(&ListHead, Node->Next, AK_ATOMIC_MEMORY_ORDER_RELAXED);
        Count++;
        Node = (compare_exchange_link_list_node*)AK_Atomic_Load_Ptr(&ListHead, AK_ATOMIC_MEMORY_ORDER_RELAXED);
    }
    ASSERT_EQ(Count, Iterations*Context.ThreadCount);

	for (i = 0; i < Context.ThreadCount; i++) {
		Free_Memory(Context.Threads[i].Nodes);
		AK_Thread_Delete(Context.Threads[i].Thread);
	}

	Free_Memory(Context.Threads);
}

#ifndef __ANDROID__
UTEST_MAIN();
#endif

#ifdef AK_ATOMIC_COMPILER_MSVC
#pragma warning(pop)
#endif

#ifndef AK_ATOMIC_IMPLEMENTATION
#define AK_ATOMIC_IMPLEMENTATION
#include <ak_atomic.h>
#endif
