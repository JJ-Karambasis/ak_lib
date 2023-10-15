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

#include <stdbool.h>

void DEBUG_PRINT_NODE(ak_fbx_node* Node, int Level) {
    for(int LevelIndex = 0; LevelIndex < Level; LevelIndex++) {
        printf("\t");
    }
    printf("%.*s\n", Node->Name.Size, Node->Name.Str);
    
    for(ak_fbx_node* Child = Node->FirstChild; Child; Child = Child->NextSibling) {
        DEBUG_PRINT_NODE(Child, Level+1);
    }
}

UTEST(AK_FBX, Box) {
    allocator_stats Stats;
    Stats.MemoryAllocated = 0;

    ak_fbx_scene* Scene = AK_FBX_Load("test_files/Box.fbx", &Stats);
    ASSERT_TRUE(Scene != 0);

    bool FoundCamera = false;
    bool FoundLight = false;
    bool FoundCube = false;

    ak_fbx_u32 NodeIndex = 0;
    ak_fbx_node* NodeStack[1024];
    NodeStack[NodeIndex++] = Scene->RootNode;

    while(NodeIndex) {
        ak_fbx_node* Node = NodeStack[--NodeIndex];
        if(strncmp(Node->Name.Str, "Camera", Node->Name.Size)) {
            FoundCamera = true;
        }

        if(strncmp(Node->Name.Str, "Cube", Node->Name.Size)) {
            FoundCube = true;
        }

        if(strncmp(Node->Name.Str, "Light", Node->Name.Size)) {
            FoundLight = true;    
        }

        for(ak_fbx_node* Child = Node->FirstChild; Child; Child = Child->NextSibling) {
            NodeStack[NodeIndex++] = Child;
        }
    }

    ASSERT_TRUE(FoundCamera);
    ASSERT_TRUE(FoundCube);
    ASSERT_TRUE(FoundLight);

    AK_FBX_Free(Scene);
    ASSERT_EQ(Stats.MemoryAllocated, 0);
}

UTEST(AK_FBX, Geometry) {
    allocator_stats Stats;
    Stats.MemoryAllocated = 0;

    ak_fbx_scene* Scene = AK_FBX_Load("test_files/Geometry.fbx", &Stats);
    ASSERT_TRUE(Scene != 0);

    AK_FBX_Free(Scene);
    ASSERT_EQ(Stats.MemoryAllocated, 0);
}

UTEST_MAIN()

#define AK_FBX_IMPLEMENTATION
#include <ak_fbx.h>