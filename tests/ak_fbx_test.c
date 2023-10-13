#include <stdlib.h>

typedef struct allocator_stats {
    size_t MemoryAllocated;
} allocator_stats;

void* Allocate_Memory(size_t Size, void* UserData) {
    allocator_stats* Stats = (allocator_stats*)UserData;
    size_t* Result = (size_t*)malloc(Size+sizeof(size_t));
    *Result = Size;
    Stats->MemoryAllocated += Size;
    return Result+1;
}

void Free_Memory(void* Memory, void* UserData) {
    if(Memory) {
        allocator_stats* Stats = (allocator_stats*)UserData;
        size_t* Ptr = ((size_t*)Memory)-1;
        Stats->MemoryAllocated -= *Ptr;
        free(Ptr);
    }
}

#define AK_FBX_MALLOC(size, user_data) Allocate_Memory(size, user_data)
#define AK_FBX_FREE(ptr, user_data) Free_Memory(ptr, user_data)
#include <ak_fbx.h>

#pragma warning(push)
#pragma warning(disable : 5045)
#include "utest.h"
#pragma warning(pop)

UTEST(AK_FBX, Box) {
    allocator_stats Stats;
    Stats.MemoryAllocated = 0;

    ak_fbx_scene* Scene = AK_FBX_Load("test_files/Box.fbx", &Stats);
    ASSERT_TRUE(Scene != 0);

    AK_FBX_Free(Scene);
    ASSERT_EQ(Stats.MemoryAllocated, 0);
}

UTEST_MAIN()

#define AK_FBX_IMPLEMENTATION
#include <ak_fbx.h>