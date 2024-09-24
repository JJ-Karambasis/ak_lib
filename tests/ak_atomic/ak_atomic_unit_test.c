#define AK_ATOMIC_IMPLEMENTATION
#include "ak_atomic_test_header.h"
#include <tests/utest.h>

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
	uint32_t 		  Count;
	loadload8_entry*  Entries;
} loadload8_context;

static AK_THREAD_CALLBACK_DEFINE(LoadLoadU8_A) {
	uint32_t i;
	int FinishedProcessing;
	loadload8_thread* Context = (loadload8_thread*)UserData;
	(void)Thread;
	
	for (;;) {
		FinishedProcessing = ak_atomic_true;

		for (i = 0; i < Context->Count; i++) {
			loadload8_entry* Entry = Context->Entries + i;
			if (!Entry->HasProcessed) {
				uint8_t A = AK_Atomic_Load_U8(&Entry->A, AK_ATOMIC_MEMORY_ORDER_RELAXED);
				uint8_t B = AK_Atomic_Load_U8(&Entry->B, AK_ATOMIC_MEMORY_ORDER_RELAXED);
				if (A == 2) {
					Entry->HasProcessed = ak_atomic_true;
					Entry->HasPassed = (B == 2);
				} else {
					FinishedProcessing = ak_atomic_false;
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
		
		Thread->Count = 10000000;
		Thread->Entries = (loadload8_entry*)Allocate_Memory(sizeof(loadload8_entry)*Thread->Count);
		Memory_Clear(Thread->Entries, Thread->Count * sizeof(loadload8_entry));
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload8_thread* Thread = Context.Threads + i;
		Thread->Thread = AK_Thread_Create(LoadLoadU8_A, Thread);
	}
	for (i = 0; i < Context.ThreadCount; i++) {
		loadload8_thread* Thread = Context.Threads + i;
		
		for (j = 0; j < Thread->Count; j++) {
			loadload8_entry* Entry = Thread->Entries + j;
			Entry->B.Nonatomic = 2;
			AK_Atomic_Fence_Seq_Cst();
			Entry->A.Nonatomic = 2;
		}
	}

	for (i = 0; i < Context.ThreadCount; i++) {
		loadload8_thread* Thread = Context.Threads + i;
		AK_Thread_Delete(Thread->Thread);

		for (j = 0; j < Thread->Count; j++) {
			ASSERT_TRUE(Thread->Entries[j].HasPassed == ak_atomic_true);
		}

		Free_Memory(Thread->Entries);
	}

	Free_Memory(Context.Threads);
}

UTEST_MAIN();

#ifdef AK_ATOMIC_COMPILER_MSVC
#pragma warning(pop)
#endif