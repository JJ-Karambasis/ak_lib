#ifndef AK_FBX_H
#define AK_FBX_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AKFBXDEF
# ifdef AK_FBX_STATIC
# define AKFBXDEF static
# else
# define AKFBXDEF extern
# endif
#endif

#ifndef AK_FBX_NO_STDIO
#include <stdio.h>
#endif

#define AK_FBX__STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(!!(COND))*2-1]
// token pasting madness:
#define AK_FBX__COMPILE_TIME_ASSERT3(X,L) AK_FBX__STATIC_ASSERT(X,static_assertion_at_line_##L)
#define AK_FBX__COMPILE_TIME_ASSERT2(X,L) AK_FBX__COMPILE_TIME_ASSERT3(X,L)
#define AK_FBX__COMPILE_TIME_ASSERT(X)    AK_FBX__COMPILE_TIME_ASSERT2(X,__LINE__)

#ifndef AK_FBX_CUSTOM_TYPES
typedef char ak_fbx_s8;
typedef short ak_fbx_s16;
typedef int ak_fbx_s32;
typedef long long ak_fbx_s64;
typedef unsigned char ak_fbx_u8;
typedef unsigned short ak_fbx_u16;
typedef unsigned int ak_fbx_u32;
typedef unsigned long long ak_fbx_u64;
#endif

AK_FBX__COMPILE_TIME_ASSERT(sizeof(ak_fbx_s8) == 1);
AK_FBX__COMPILE_TIME_ASSERT(sizeof(ak_fbx_u8) == 1);
AK_FBX__COMPILE_TIME_ASSERT(sizeof(ak_fbx_s16) == 2);
AK_FBX__COMPILE_TIME_ASSERT(sizeof(ak_fbx_u16) == 2);
AK_FBX__COMPILE_TIME_ASSERT(sizeof(ak_fbx_s32) == 4);
AK_FBX__COMPILE_TIME_ASSERT(sizeof(ak_fbx_u32) == 4);
AK_FBX__COMPILE_TIME_ASSERT(sizeof(ak_fbx_s64) == 8);
AK_FBX__COMPILE_TIME_ASSERT(sizeof(ak_fbx_u64) == 8);

typedef struct ak_fbx_scene ak_fbx_scene;

AKFBXDEF ak_fbx_scene* AK_FBX_Load_From_Memory(const void* Buffer, ak_fbx_u64 Length, void* UserData);

#ifndef AK_FBX_NO_STDIO
AKFBXDEF ak_fbx_scene* AK_FBX_Load(const char* Filename, void* UserData);
AKFBXDEF ak_fbx_scene* AK_FBX_Load_From_File(FILE* File, void* UserData);
#endif

AKFBXDEF void AK_FBX_Free(ak_fbx_scene* Scene);

//If thread local storage is supported, this is thread safe
//If thread local storage is not supported, this is not thread safe
//If AK_FBX_NO_ERROR_MESSAGE is defined, this returns null
AKFBXDEF const char* AK_FBX_Error_Message();

#ifdef __cplusplus
}
#endif 

#ifndef AK_FBX_THREAD_LOCAL
# if defined(__cplusplus) &&  __cplusplus >= 201103L
# define AK_FBX_THREAD_LOCAL       thread_local
# elif defined(__GNUC__) && __GNUC__ < 5
# define AK_FBX_THREAD_LOCAL       __thread
# elif defined(_MSC_VER)
# define AK_FBX_THREAD_LOCAL       __declspec(thread)
# elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
# define AK_FBX_THREAD_LOCAL       _Thread_local
# endif

# ifndef AK_FBX_THREAD_LOCAL
#  if defined(__GNUC__)
#  define AK_FBX_THREAD_LOCAL       __thread
#  endif
# endif

#endif

#if !defined(AK_FBX_MALLOC)
#define AK_FBX_MALLOC(size, user_data) ((void)(user_data), malloc(size))
#define AK_FBX_FREE(ptr, user_data) ((void)(user_data), free(ptr))
#endif

#ifndef AK_FBX_ASSERT
#include <assert.h>
#define AK_FBX_ASSERT(x) assert(x)
#endif

#endif //AK_FBX_H

#ifdef AK_FBX_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#define ak_fbx__nullptr nullptr
#else
#define ak_fbx__nullptr 0
#endif


#define AK_FBX__SLL_Push_Back(First, Last, Node) (!First ? (First = Last = Node) : (Last->Next = Node, Last = Node))

AK_FBX_THREAD_LOCAL const char* AK_FBX__G_Error_Message;

#ifndef AK_FBX_NO_ERROR_MESSAGE
static int AK_FBX__Error(const char *Str)
{
   AK_FBX__G_Error_Message = Str;
   return 0;
}
#endif

#ifdef AK_FBX_NO_ERROR_MESSAGE
   #define AK_FBX__Error(x,y)  0
#else
   #define AK_FBX__Error(x,y)  AK_FBX__Error(x)
#endif

typedef struct ak_fbx__arena_block {
    ak_fbx_u8* Bytes;
    size_t Capacity;
    size_t Used;
    struct ak_fbx__arena_block* Next;
} ak_fbx__arena_block;

typedef struct ak_fbx__arena {
    void* UserData;
    ak_fbx__arena_block* FirstBlock;
    ak_fbx__arena_block* LastBlock;
    ak_fbx__arena_block* CurrentBlock;
} ak_fbx__arena;

static ak_fbx__arena_block* AK_FBX__Arena_Get_Current_Block(ak_fbx__arena* Arena, size_t Size) {
    if(!Arena->CurrentBlock) return ak_fbx__nullptr;

    ak_fbx__arena_block* Block = Arena->CurrentBlock;
    while(Block->Used+Size > Block->Capacity) {
        Block = Block->Next;
        if(!Block) return ak_fbx__nullptr;
    }

    return Block;
}

static ak_fbx__arena_block* AK_FBX__Arena_Allocate_Block(ak_fbx__arena* Arena, size_t BlockSize) {
    ak_fbx__arena_block* Block = (ak_fbx__arena_block*)AK_FBX_MALLOC(sizeof(ak_fbx__arena_block)+BlockSize, Arena->UserData);
    Block->Bytes = (ak_fbx_u8*)(Block+1);
    Block->Capacity = BlockSize;
    return Block;
}

static ak_fbx__arena AK_FBX__Arena_Create(void* UserData) {
    ak_fbx__arena Result;
    Result.UserData   = UserData;
    Result.FirstBlock = Result.LastBlock = Result.CurrentBlock = ak_fbx__nullptr;
    return Result;
}

static void* AK_FBX__Arena_Push(ak_fbx__arena* Arena, size_t Size) {
    if(!Size) {
        return ak_fbx__nullptr;
    }

    ak_fbx__arena_block* Block = AK_FBX__Arena_Get_Current_Block(Arena, Size);
    if(!Block) {
        size_t BlockSize = 1024*1024; //Default is 1MB
        if(Size > BlockSize) {
            BlockSize = Size;
        }

        Block = AK_FBX__Arena_Allocate_Block(Arena, BlockSize);
        if(!Block) {
            return ak_fbx__nullptr;
        }

        AK_FBX__SLL_Push_Back(Arena->FirstBlock, Arena->LastBlock, Block);
    }

    Arena->CurrentBlock = Block;
    void* Result = Arena->CurrentBlock->Bytes + Arena->CurrentBlock->Used;
    Arena->CurrentBlock->Used += Size;
    AK_FBX_ASSERT(Arena->CurrentBlock->Used <= Arena->CurrentBlock->Capacity);

    return Result;
}

static void AK_FBX__Arena_Delete(ak_fbx__arena* Arena) {
    ak_fbx__arena_block* Block = Arena->FirstBlock;
    Arena->FirstBlock = Arena->CurrentBlock = Arena->LastBlock = ak_fbx__nullptr;
    
    while(Block) {
        ak_fbx__arena_block* BlockToDelete = Block;
        Block = Block->Next;
        AK_FBX_FREE(BlockToDelete, Arena->UserData);
    }    

}

#define AK_FBX__Arena_Push_Struct(arena, type) (type*)AK_FBX__Arena_Push(arena, sizeof(type))

struct ak_fbx_scene {
    void*         UserData;
    ak_fbx__arena Arena;
};

AKFBXDEF ak_fbx_scene* AK_FBX_Load_From_Memory(const void* Buffer, ak_fbx_u64 Length, void* UserData) {
    ak_fbx__arena Arena = AK_FBX__Arena_Create(UserData);
    ak_fbx_scene* Scene = AK_FBX__Arena_Push_Struct(&Arena, ak_fbx_scene);

    Scene->UserData = UserData;
    Scene->Arena    = Arena;

    return Scene;
}

#ifndef AK_FBX_NO_STDIO
AKFBXDEF ak_fbx_scene* AK_FBX_Load(const char* Filename, void* UserData) {
    FILE* File = fopen(Filename, "rb");
    if(!File) {
        //TODO: Diagnostic and error logging
        return ak_fbx__nullptr;
    }

    ak_fbx_scene* Result = AK_FBX_Load_From_File(File, UserData);
    fclose(File);

    return Result;
}

AKFBXDEF ak_fbx_scene* AK_FBX_Load_From_File(FILE* File, void* UserData) {
    fseek(File, 0, SEEK_END);
    ak_fbx_s64 FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    void* Buffer = AK_FBX_MALLOC(FileSize, UserData);
    if(!Buffer) {
        //TODO: Diagnostic and error logging
        return ak_fbx__nullptr;
    }

    if(fread(Buffer, 1, FileSize, File) != FileSize) {
        AK_FBX_FREE(Buffer, UserData);
        //TODO: Diagnostic and error logging
        return ak_fbx__nullptr;
    }

    ak_fbx_scene* Result = AK_FBX_Load_From_Memory(Buffer, FileSize, UserData);
    AK_FBX_FREE(Buffer, UserData);

    return Result;
}
#endif

AKFBXDEF void AK_FBX_Free(ak_fbx_scene* Scene) {
    if(Scene) {
        ak_fbx__arena* Arena = &Scene->Arena;
        AK_FBX__Arena_Delete(Arena);
    }
}

//If thread local storage is supported, this is thread safe
//If thread local storage is not supported, this is not thread safe
//If AK_FBX_NO_ERROR_MESSAGE is defined, this returns null
AKFBXDEF const char* AK_FBX_Error_Message() {
    return AK_FBX__G_Error_Message;
}

#ifdef __cplusplus
}
#endif

#endif //AK_FBX_IMPLEMENTATION