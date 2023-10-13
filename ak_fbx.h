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
AKFBXDEF const char* AK_FBX_Error_Message(void);

#ifdef __cplusplus
}
#endif

#endif //AK_FBX_H

#ifdef AK_FBX_IMPLEMENTATION

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

#if !defined(AK_FBX_STRNCMP)
#define AK_FBX_STRNCMP(strA, strB, length) strncmp(strA, strB, length)
#endif

#if !defined(AK_FBX_STRNCPY)
#define AK_FBX_STRNCPY(strA, strB, length) strncpy(strA, strB, length)
#endif

#ifndef AK_FBX_ASSERT
#include <assert.h>
#define AK_FBX_ASSERT(x) assert(x)
#endif

#pragma warning(push)
#pragma warning(disable : 4996 4820 5045)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#define ak_fbx__nullptr nullptr
#else
#define ak_fbx__nullptr 0
#endif


#define AK_FBX__SLL_Push_Back(First, Last, Node) (!First ? (First = Last = Node) : (Last->Next = Node, Last = Node))
#define AK_FBX__DLL_Push_Back_NP(First, Last, Node, Next, Prev) (!(First) ? ((First) = (Last) = (Node)) : ((Node)->Prev = (Last), (Last)->Next = (Node), (Last) = (Node)))

AK_FBX_THREAD_LOCAL const char* AK_FBX__G_Error_Message;

#ifndef AK_FBX_NO_ERROR_MESSAGE
static int AK_FBX__Error(const char *Str)
{
   AK_FBX__G_Error_Message = Str;
   return 0;
}
#endif

enum {
    ak_fbx__false = 0,
    ak_fbx__true = 1
};

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
#define AK_FBX__Arena_Push_Array(arena, count, type) (type*)AK_FBX__Arena_Push(arena, sizeof(type)*(count))

typedef struct ak_fbx__stream {
    const ak_fbx_u8* Stream;
    ak_fbx_u64 Length;
    ak_fbx_u64 At;
} ak_fbx__stream;

static inline ak_fbx_s8 AK_FBX__Stream_Is_Valid(ak_fbx__stream* Stream) {
    return (Stream->At < Stream->Length);
}

static inline const void* AK_FBX__Stream_Peek(ak_fbx__stream* Stream) {
    if(Stream->At >= Stream->Length) {
        return ak_fbx__nullptr;
    }
    return Stream->Stream+Stream->At;
}

static inline void AK_FBX__Stream_Skip(ak_fbx__stream* Stream, ak_fbx_u64 Offset) {
    if(Offset) {
        AK_FBX_ASSERT(Stream->At < Stream->Length);
        Stream->At += Offset;
    }
}

static inline const void* AK_FBX__Stream_Consume(ak_fbx__stream* Stream, ak_fbx_u64 Size) {
    if(!Size) {
        return ak_fbx__nullptr;
    }

    AK_FBX_ASSERT(Stream->At < Stream->Length);
    const void* Result = Stream->Stream + Stream->At;
    Stream->At += Size;
    return Result;
}

static inline ak_fbx_u8 AK_FBX__Stream_Consume8(ak_fbx__stream* Stream) {
    return *(ak_fbx_u8*)AK_FBX__Stream_Consume(Stream, sizeof(ak_fbx_u8));
}

static inline ak_fbx_u16 AK_FBX__Stream_Consume16(ak_fbx__stream* Stream) {
    return *(ak_fbx_u16*)AK_FBX__Stream_Consume(Stream, sizeof(ak_fbx_u16));
}

static inline ak_fbx_u32 AK_FBX__Stream_Consume32(ak_fbx__stream* Stream) {
    return *(ak_fbx_u32*)AK_FBX__Stream_Consume(Stream, sizeof(ak_fbx_u32));
}

static inline ak_fbx_u64 AK_FBX__Stream_Consume64(ak_fbx__stream* Stream) {
    return *(ak_fbx_u64*)AK_FBX__Stream_Consume(Stream, sizeof(ak_fbx_u64));
}

typedef struct ak_fbx__buffer {
    ak_fbx_u32       Length;
    const ak_fbx_u8* Ptr;
} ak_fbx__buffer;

typedef struct ak_fbx__string {
    ak_fbx_u32 Length;
    const char* Str;
} ak_fbx__string;

static ak_fbx__string AK_FBX__Make_String(ak_fbx__arena* Arena, const char* Str, ak_fbx_u32 StrLength) {
    char* Buffer = (char*)AK_FBX__Arena_Push(Arena, (StrLength+1)*sizeof(char));
    Buffer[StrLength] = 0;
    AK_FBX_STRNCPY(Buffer, Str, StrLength);

    ak_fbx__string Result;
    Result.Str = Buffer;
    Result.Length = StrLength;
    return Result;
}

typedef struct ak_fbx__bool_array {
    ak_fbx_s8* Ptr;
    ak_fbx_u32 Count;
} ak_fbx__bool_array;

typedef struct ak_fbx__s32_array {
    ak_fbx_s32* Ptr;
    ak_fbx_u32 Count;
} ak_fbx__s32_array;

typedef struct ak_fbx__f32_array {
    float* Ptr;
    ak_fbx_u32   Count;
} ak_fbx__f32_array;

typedef struct ak_fbx__s64_array {
    ak_fbx_s64* Ptr;
    ak_fbx_u32 Count;
} ak_fbx__s64_array;

typedef struct ak_fbx__f64_array {
    double* Ptr;
    ak_fbx_u32   Count;
} ak_fbx__f64_array;

typedef enum ak_fbx__property_type {
    AK_FBX__PROPERTY_TYPE_BOOL,
    AK_FBX__PROPERTY_TYPE_S16,
    AK_FBX__PROPERTY_TYPE_S32,
    AK_FBX__PROPERTY_TYPE_F32,
    AK_FBX__PROPERTY_TYPE_F64,
    AK_FBX__PROPERTY_TYPE_S64,
    AK_FBX__PROPERTY_TYPE_BUFFER,
    AK_FBX__PROPERTY_TYPE_STRING,
    AK_FBX__PROPERTY_TYPE_BOOL_ARRAY,
    AK_FBX__PROPERTY_TYPE_S32_ARRAY,
    AK_FBX__PROPERTY_TYPE_F32_ARRAY,
    AK_FBX__PROPERTY_TYPE_S64_ARRAY,
    AK_FBX__PROPERTY_TYPE_F64_ARRAY,
} ak_fbx__property_type;

typedef struct ak_fbx__property {
    ak_fbx__property_type Type;

    union {
        ak_fbx_s8          Bool;
        ak_fbx_s16         S16;
        ak_fbx_s32         S32;
        float              F32;
        double             F64;
        ak_fbx_s64         S64;
        ak_fbx__buffer     Buffer;
        ak_fbx__string     String;
        ak_fbx__bool_array BoolArray;
        ak_fbx__s32_array  S32Array;
        ak_fbx__f32_array  F32Array;
        ak_fbx__s64_array  S64Array;
        ak_fbx__f64_array  F64Array;
    } Data;
} ak_fbx__property;

typedef struct ak_fbx__property_array {
    ak_fbx__property* Ptr;
    ak_fbx_u64 Count;
} ak_fbx__property_array;

typedef struct ak_fbx__node {
    ak_fbx__string Name;

    struct ak_fbx__node* Parent;
    struct ak_fbx__node* FirstChild;
    struct ak_fbx__node* LastChild;
    struct ak_fbx__node* PrevSibling;
    struct ak_fbx__node* NextSibling;
} ak_fbx__node;

static void AK_FBX__Node_Add_Child(ak_fbx__node* Parent, ak_fbx__node* Child) {
    Child->Parent = Parent;
    AK_FBX__DLL_Push_Back_NP(Parent->FirstChild, Parent->LastChild, Child, PrevSibling, NextSibling);
}

typedef enum ak_fbx__binary_node_status {
    AK_FBX__BINARY_NODE_STATUS_SUCCESS,
    AK_FBX__BINARY_NODE_STATUS_TERMINATE,
    AK_FBX__BINARY_NODE_STATUS_ERROR
} ak_fbx__binary_node_status;

#define AK_FBX__BINARY_READ_WORD(stream) Is64Bit ? AK_FBX__Stream_Consume64(stream) : AK_FBX__Stream_Consume32(stream)

static ak_fbx_s8 AK_FBX__Binary_Read_Property(ak_fbx__stream* Stream, ak_fbx__property* Property, ak_fbx__arena* Arena) {
    if(!AK_FBX__Stream_Is_Valid(Stream)) {
        //TODO: Diagnostic and error logging
        return ak_fbx__false;
    }

    ak_fbx_u8 Type = AK_FBX__Stream_Consume8(Stream);
    switch(Type) {
        //16 bit signed integer
        case 'Y': {
            Property->Type = AK_FBX__PROPERTY_TYPE_S16;
            Property->Data.S16 = AK_FBX__Stream_Consume16(Stream);
        } break;

        //1 bit bool flag
        case 'C': {
            ak_fbx_s8 Data = AK_FBX__Stream_Consume8(Stream);
        } break;

        //32 bit signed integer
        case 'I': {
            ak_fbx_s32 Data = AK_FBX__Stream_Consume32(Stream);
        } break;

        //32 bit floating point
        case 'F': {
            float Data = *(float*)AK_FBX__Stream_Consume(Stream, sizeof(float));
        } break;

        //64 bit floating point
        case 'D': {
            double Data = *(double*)AK_FBX__Stream_Consume(Stream, sizeof(double));
        } break;

        //64 bit signed integer
        case 'L': {
            ak_fbx_s64 Data = AK_FBX__Stream_Consume64(Stream);
        } break;

        //Raw binary data
        case 'R': {
            ak_fbx__buffer Data;
            Data.Length = AK_FBX__Stream_Consume32(Stream);
            Data.Ptr = AK_FBX__Stream_Consume(Stream, Data.Length);
        } break;

        case 'S': {
            ak_fbx__string Data;
            Data.Length = AK_FBX__Stream_Consume32(Stream);
            Data.Str = AK_FBX__Stream_Consume(Stream, Data.Length);
        } break;

        //Array section
        case 'b':
        case 'c':
        case 'f':
        case 'd':
        case 'l':
        case 'i': {
            ak_fbx_u32 ArrayLength = AK_FBX__Stream_Consume32(Stream);
            ak_fbx_u32 Encoding = AK_FBX__Stream_Consume32(Stream);
            ak_fbx_u32 CompressedLength = AK_FBX__Stream_Consume32(Stream);
            
            if(Encoding == 0) {
                //Normal length array
            } else if(Encoding == 1) {
                //GZip encoding array


            } else {
                //TODO: Diagnostic and error logging
                return ak_fbx__false;
            }

            AK_FBX__Stream_Skip(Stream, CompressedLength);
        } break;

        default: {
            //TODO: Diagnostic and error logging
            return ak_fbx__false; 
        } break;
    }

    if(Stream->At > Stream->Length) {
        //TODO: Diagnostic and error logging
        return ak_fbx__false;
    }

    return ak_fbx__true;
}

static ak_fbx__binary_node_status AK_FBX__Binary_Read_Node(ak_fbx__stream* Stream, ak_fbx__node* Node, ak_fbx__arena* Arena, ak_fbx_s8 Is64Bit) {
    // The first word always contains the node end offset
    ak_fbx_u64 EndOffset = AK_FBX__BINARY_READ_WORD(Stream);

    //Sometimes the end offset is 0, which occurs when there is a strange
    //footer in the file. Doesn't seem like any readable information is in that file.
    if(!EndOffset) {
        return AK_FBX__BINARY_NODE_STATUS_TERMINATE;
    }

    if(EndOffset > Stream->Length) {
        //TODO: Diagnostic and error logging
        return AK_FBX__BINARY_NODE_STATUS_ERROR;
    } else if(EndOffset < Stream->At) {
        //TODO: Diagnostic and error logging
        return AK_FBX__BINARY_NODE_STATUS_ERROR;  
    }

    // The second word contains the number of properties in the node
    ak_fbx_u64 PropertyCount = AK_FBX__BINARY_READ_WORD(Stream);

    // The third word contains the length of the property list
    ak_fbx_u64 PropertyLength = AK_FBX__BINARY_READ_WORD(Stream);

    ak_fbx_u8 NodeNameLength = AK_FBX__Stream_Consume8(Stream);
    const char* NodeName = (const char*)AK_FBX__Stream_Consume(Stream, NodeNameLength);

    Node->Name = AK_FBX__Make_String(Arena, NodeName, NodeNameLength);
    
    if((Stream->At+PropertyLength) > Stream->Length) {
        //TODO: Diagnostic and error logging
        return AK_FBX__BINARY_NODE_STATUS_ERROR;
    }

    ak_fbx__stream PropertyStream;
    PropertyStream.Stream = Stream->Stream + Stream->At;
    PropertyStream.Length = PropertyLength;
    PropertyStream.At = 0;

    ak_fbx__property_array Properties;
    Properties.Count = PropertyCount;
    Properties.Ptr = AK_FBX__Arena_Push_Array(Arena, PropertyCount, ak_fbx__property);

    ak_fbx_u64 PropertyIndex;
    for(PropertyIndex = 0; PropertyIndex < PropertyCount; PropertyIndex++) {
        if(!AK_FBX__Binary_Read_Property(&PropertyStream, &Properties.Ptr[PropertyIndex], Arena)) {
            return AK_FBX__BINARY_NODE_STATUS_ERROR;
        }
    }

    if(PropertyStream.Length != PropertyStream.At) {
        //TODO: Diagnostic and error logging
        return AK_FBX__BINARY_NODE_STATUS_ERROR;
    }

    AK_FBX__Stream_Skip(Stream, PropertyLength);

    //At the end of each nested node, there is a null record to indicate that subnodes exist.
    size_t SentinelBlockLength = Is64Bit ? (sizeof(ak_fbx_u64)*3 + 1) : (sizeof(ak_fbx_u32)*3+1);

    if(Stream->At < EndOffset) {
        if(EndOffset-Stream->At < SentinelBlockLength) {
            //TODO: Diagnostic and error logging
            return AK_FBX__BINARY_NODE_STATUS_ERROR;
        }

        ak_fbx__stream NestedStream;
        NestedStream.Stream = Stream->Stream;
        NestedStream.Length = EndOffset-SentinelBlockLength;
        NestedStream.At = Stream->At;

        while(AK_FBX__Stream_Is_Valid(&NestedStream)) {
            ak_fbx__node* ChildNode = AK_FBX__Arena_Push_Struct(Arena, ak_fbx__node);
            if(AK_FBX__Binary_Read_Node(&NestedStream, ChildNode, Arena, Is64Bit) == AK_FBX__BINARY_NODE_STATUS_ERROR) {
                return AK_FBX__BINARY_NODE_STATUS_ERROR;
            }

            AK_FBX__Node_Add_Child(Node, ChildNode);
        }

        Stream->At = NestedStream.At;
        if(!AK_FBX__Stream_Is_Valid(Stream)) {
            //TODO: Diagnostic and error logging
            return AK_FBX__BINARY_NODE_STATUS_ERROR;
        }

        for (size_t i = 0; i < SentinelBlockLength; i++) {
            const ak_fbx_u8* At = Stream->Stream+Stream->At;
            if(At[i] != '\0') {
                //TODO: Diagnostic and error logging
                return AK_FBX__BINARY_NODE_STATUS_ERROR;
            }
        }

        AK_FBX__Stream_Skip(Stream, SentinelBlockLength);
    }

    if(Stream->At != EndOffset) {
        //TODO: Diagnostic and error logging
        return AK_FBX__BINARY_NODE_STATUS_ERROR;
    }

    return AK_FBX__BINARY_NODE_STATUS_SUCCESS;
}

#undef AK_FBX__BINARY_READ_WORD

struct ak_fbx_scene {
    void*         UserData;
    ak_fbx__arena Arena;
};

AKFBXDEF ak_fbx_scene* AK_FBX_Load_From_Memory(const void* Buffer, ak_fbx_u64 Length, void* UserData) {
    ak_fbx__arena Arena = AK_FBX__Arena_Create(UserData);
    ak_fbx_scene* Scene = AK_FBX__Arena_Push_Struct(&Arena, ak_fbx_scene);

    Scene->UserData = UserData;
    Scene->Arena    = Arena;

    ak_fbx__stream Stream;
    Stream.Stream = (const ak_fbx_u8*)Buffer;
    Stream.Length = Length;
    Stream.At = 0;

    printf("Debug");

    ak_fbx__node* RootNode = AK_FBX__Arena_Push_Struct(&Scene->Arena, ak_fbx__node);
    RootNode->Name = AK_FBX__Make_String(&Scene->Arena, "Root", 4);

    if(AK_FBX_STRNCMP((const char*)AK_FBX__Stream_Peek(&Stream), "Kaydara FBX Binary", 18) == 0) {
        AK_FBX__Stream_Skip(&Stream, 23);

        if(!AK_FBX__Stream_Is_Valid(&Stream)) {
            //TODO: Diagnostic and error logging. Free resources
            return ak_fbx__nullptr;
        }
        
        ak_fbx_u32 Version = AK_FBX__Stream_Consume32(&Stream);
        ak_fbx_s8 Is64Bit = Version >= 7500;

        while(AK_FBX__Stream_Is_Valid(&Stream)) {
            ak_fbx__node* ChildNode = AK_FBX__Arena_Push_Struct(&Arena, ak_fbx__node);
            ak_fbx__binary_node_status Status = AK_FBX__Binary_Read_Node(&Stream, ChildNode, &Scene->Arena, Is64Bit);

            if(Status == AK_FBX__BINARY_NODE_STATUS_ERROR) {
                //TODO: Diagnostic and error logging. Free resources
                return ak_fbx__nullptr;
            }

            if(Status == AK_FBX__BINARY_NODE_STATUS_TERMINATE) {
                break;
            }

            AK_FBX__Node_Add_Child(RootNode, ChildNode);
        }

    } else {
        //FBX file is not binary
        AK_FBX_ASSERT(!"Not Implemented!");
    }

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

    if(fread(Buffer, 1, FileSize, File) != (size_t)FileSize) {
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
AKFBXDEF const char* AK_FBX_Error_Message(void) {
    return AK_FBX__G_Error_Message;
}

#ifdef __cplusplus
}
#endif

#pragma warning(pop)

#endif //AK_FBX_IMPLEMENTATION