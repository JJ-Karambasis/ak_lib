#ifndef AK_FBX_H
#define AK_FBX_H


#pragma warning(push)
#pragma warning(disable : 4820)

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

typedef struct ak_fbx_s32_array {
    ak_fbx_s32* Ptr;
    ak_fbx_u32  Count;
} ak_fbx_s32_array;

typedef struct ak_fbx_string {
    const char* Str;
    ak_fbx_u32  Size;
} ak_fbx_string;

typedef struct ak_fbx_v2 {
    double Data[2];
} ak_fbx_v2;

typedef struct ak_fbx_v3 {
    double Data[3];
} ak_fbx_v3;

typedef struct ak_fbx_m4x3 {
    double Data[12];
} ak_fbx_m4x3;

typedef struct ak_fbx_v2_array {
    ak_fbx_v2* Ptr;
    ak_fbx_u32 Count;
} ak_fbx_v2_array;

typedef struct ak_fbx_v3_array {
    ak_fbx_v3* Ptr;
    ak_fbx_u32 Count;
} ak_fbx_v3_array;

typedef struct ak_fbx_polygon {
    ak_fbx_u32 IndexArrayOffset;
    ak_fbx_u32 IndexArrayCount;
} ak_fbx_polygon;

typedef struct ak_fbx_polygon_array {
    ak_fbx_polygon* Ptr;
    ak_fbx_u32      Count;
} ak_fbx_polygon_array;

typedef struct ak_fbx_polygons {
    ak_fbx_polygon_array PolygonArray;
    ak_fbx_s32_array     VertexIndices;
    ak_fbx_s32_array     NormalIndices;
} ak_fbx_polygons;

typedef enum ak_fbx_node_type {
    AK_FBX_NODE_TYPE_NONE,
    AK_FBX_NODE_TYPE_GEOMETRY
} ak_fbx_node_type;

typedef struct ak_fbx_node {
    ak_fbx_node_type    Type;
    ak_fbx_string       Name;
    ak_fbx_m4x3         GlobalTransform;
    ak_fbx_m4x3         LocalTransform;
    struct ak_fbx_node* Parent;
    struct ak_fbx_node* FirstChild;
    struct ak_fbx_node* LastChild;
    struct ak_fbx_node* PrevSibling;
    struct ak_fbx_node* NextSibling;
} ak_fbx_node;

typedef struct ak_fbx_node_array {
    ak_fbx_node** Ptr;
    ak_fbx_u32    Count;
} ak_fbx_node_array;

typedef struct ak_fbx_uv_map {
    ak_fbx_string    Name;
    ak_fbx_v2_array  UVs;
    ak_fbx_s32_array UVIndices;
} ak_fbx_uv_map;

typedef struct ak_fbx_uv_map_array {
    ak_fbx_uv_map* Ptr;
    ak_fbx_u32     Count;
} ak_fbx_uv_map_array;

typedef struct ak_fbx_geometry {
    ak_fbx_node*        Node;
    ak_fbx_v3_array     Vertices;
    ak_fbx_v3_array     Normals;
    ak_fbx_polygons     Polygons;
    ak_fbx_uv_map_array UVMaps;
} ak_fbx_geometry;

typedef struct ak_fbx_geometry_array {
    ak_fbx_geometry** Ptr;
    ak_fbx_u32        Count;
} ak_fbx_geometry_array;

typedef struct ak_fbx_scene {
    ak_fbx_node*          RootNode;
    ak_fbx_node_array     Nodes;
    ak_fbx_geometry_array Geometries;
} ak_fbx_scene;

AKFBXDEF ak_fbx_scene* AK_FBX_Load_From_Memory(const void* Buffer, ak_fbx_u64 Length, void* UserData);

#ifndef AK_FBX_NO_STDIO
AKFBXDEF ak_fbx_scene* AK_FBX_Load(const char* Filename, void* UserData);
AKFBXDEF ak_fbx_scene* AK_FBX_Load_From_File(FILE* File, void* UserData);
#endif

AKFBXDEF void AK_FBX_Free(ak_fbx_scene* Scene);

//Node utils
AKFBXDEF ak_fbx_geometry* AK_FBX_Node_Get_Geometry(ak_fbx_node* Node);

//If thread local storage is supported, this is thread safe
//If thread local storage is not supported, this is not thread safe
//If AK_FBX_NO_ERROR_MESSAGE is defined, this returns null
AKFBXDEF const char* AK_FBX_Error_Message(void);

//Math util functions
AKFBXDEF void AK_FBX_V3_Zero(ak_fbx_v3* V);
AKFBXDEF void AK_FBX_V3(ak_fbx_v3* V, double x, double y, double z);
AKFBXDEF void AK_FBX_M4x3_Identity(ak_fbx_m4x3* M);

#ifdef __cplusplus
}
#endif

#pragma warning(pop)

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

#define AK_FBX__STR_LIT_EXPAND(literal) literal, sizeof(literal)-1

#if !defined(AK_FBX_STRNCPY)
#define AK_FBX_STRNCPY(strA, strB, length) strncpy(strA, strB, length)
#endif

#if !defined(AK_FBX_MEMSET)
#define AK_FBX_MEMSET(mem, index, size) memset(mem, index, size)
#endif
 
#if !defined(AK_FBX_MEMCPY)
#define AK_FBX_MEMCPY(dst, src, length) memcpy(dst, src, length)
#endif

#if !defined(AK_FBX_SIN) || !defined(AK_FBX_COS)
#include <math.h>
#endif

#if !defined(AK_FBX_SIN)
#define AK_FBX_SIN(angle) sin(angle)
#endif

#if !defined(AK_FBX_COS)
#define AK_FBX_COS(angle) cos(angle)
#endif

#ifndef AK_FBX_ASSERT
#include <assert.h>
#define AK_FBX_ASSERT(x) assert(x)
#endif

#pragma warning(push)
#pragma warning(disable : 4062 4820 4996 5045)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
#define ak_fbx__nullptr nullptr
#else
#define ak_fbx__nullptr 0
#endif

//0.01745329251 is PI/180
#define AK_FBX__To_Radians(degrees) ((degrees)*0.01745329251)

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

static bool AK_FBX__Str_Cmp_Raw(const char* StrA, const char* StrB, size_t N) {
    return AK_FBX_STRNCMP(StrA, StrB, N) == 0;
}

static bool AK_FBX__Str_Cmp(ak_fbx_string String, const char* Str, size_t N) {
    if(String.Size != N) return false;
    return AK_FBX__Str_Cmp_Raw(String.Str, Str, N);
}

typedef struct ak_fbx__m3 {
    double Data[9];
} ak_fbx__m3;

static void AK_FBX__M3_Identity(ak_fbx__m3* M) {
    AK_FBX_MEMSET(M, 0, sizeof(ak_fbx__m3));
    M->Data[0] = 1;
    M->Data[4] = 1;
    M->Data[8] = 1;
}

static void AK_FBX__M3_RotX(ak_fbx__m3* M, double Pitch) {
    AK_FBX__M3_Identity(M);

    double s = AK_FBX_SIN(Pitch);
    double c = AK_FBX_COS(Pitch);

    M->Data[4] =  c;
    M->Data[5] =  s;
    M->Data[7] = -s;
    M->Data[8] =  c;
}

static void AK_FBX__M3_RotY(ak_fbx__m3* M, double Yaw) {
    AK_FBX__M3_Identity(M);

    double s = AK_FBX_SIN(Yaw);
    double c = AK_FBX_COS(Yaw);

    M->Data[0] =  c;
    M->Data[2] = -s;
    M->Data[6] =  s;
    M->Data[8] =  c;
}

static void AK_FBX__M3_RotZ(ak_fbx__m3* M, double Roll) {
    AK_FBX__M3_Identity(M);

    double s = AK_FBX_SIN(Roll);
    double c = AK_FBX_COS(Roll);

    M->Data[0] =  c;
    M->Data[1] =  s;
    M->Data[3] = -s;
    M->Data[4] =  c;
}

static void AK_FBX__M3_Mul(ak_fbx__m3* Result, const ak_fbx__m3* A, const ak_fbx__m3* B) {
    Result->Data[0] = A->Data[0]*B->Data[0] + A->Data[1]*B->Data[3] + A->Data[2]*B->Data[6];
    Result->Data[1] = A->Data[0]*B->Data[1] + A->Data[1]*B->Data[4] + A->Data[2]*B->Data[7];
    Result->Data[2] = A->Data[0]*B->Data[2] + A->Data[1]*B->Data[5] + A->Data[2]*B->Data[8];

    Result->Data[3] = A->Data[3]*B->Data[0] + A->Data[4]*B->Data[3] + A->Data[5]*B->Data[6];
    Result->Data[4] = A->Data[3]*B->Data[1] + A->Data[4]*B->Data[4] + A->Data[5]*B->Data[7];
    Result->Data[5] = A->Data[3]*B->Data[2] + A->Data[4]*B->Data[5] + A->Data[5]*B->Data[8];

    Result->Data[6] = A->Data[6]*B->Data[0] + A->Data[7]*B->Data[3] + A->Data[8]*B->Data[6];
    Result->Data[7] = A->Data[6]*B->Data[1] + A->Data[7]*B->Data[4] + A->Data[8]*B->Data[7];
    Result->Data[8] = A->Data[6]*B->Data[2] + A->Data[7]*B->Data[5] + A->Data[8]*B->Data[8];
}

static void AK_FBX__M4x3_Mul(ak_fbx_m4x3* Result, const ak_fbx_m4x3* A, const ak_fbx_m4x3* B) {
    Result->Data[0] = A->Data[0]*B->Data[0] + A->Data[1]*B->Data[3] + A->Data[2]*B->Data[6];
    Result->Data[1] = A->Data[0]*B->Data[1] + A->Data[1]*B->Data[4] + A->Data[2]*B->Data[7];
    Result->Data[2] = A->Data[0]*B->Data[2] + A->Data[1]*B->Data[5] + A->Data[2]*B->Data[8];

    Result->Data[3] = A->Data[3]*B->Data[0] + A->Data[4]*B->Data[3] + A->Data[5]*B->Data[6];
    Result->Data[4] = A->Data[3]*B->Data[1] + A->Data[4]*B->Data[4] + A->Data[5]*B->Data[7];
    Result->Data[5] = A->Data[3]*B->Data[2] + A->Data[4]*B->Data[5] + A->Data[5]*B->Data[8];

    Result->Data[6] = A->Data[6]*B->Data[0] + A->Data[7]*B->Data[3] + A->Data[8]*B->Data[6];
    Result->Data[7] = A->Data[6]*B->Data[1] + A->Data[7]*B->Data[4] + A->Data[8]*B->Data[7];
    Result->Data[8] = A->Data[6]*B->Data[2] + A->Data[7]*B->Data[5] + A->Data[8]*B->Data[8];

    Result->Data[9]  = A->Data[9]*B->Data[0] + A->Data[10]*B->Data[3] + A->Data[11]*B->Data[6] + B->Data[9];
    Result->Data[10] = A->Data[9]*B->Data[1] + A->Data[10]*B->Data[4] + A->Data[11]*B->Data[7] + B->Data[10];
    Result->Data[11] = A->Data[9]*B->Data[2] + A->Data[10]*B->Data[5] + A->Data[11]*B->Data[8] + B->Data[11];
}

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

static ak_fbx__buffer AK_FBX__Make_Buffer(ak_fbx__arena* Arena, const void* Buffer, ak_fbx_u32 BufferLength) {
    void* DstBuffer = AK_FBX__Arena_Push(Arena, BufferLength);
    AK_FBX_MEMCPY(DstBuffer, Buffer, BufferLength);

    ak_fbx__buffer Result;
    Result.Length = BufferLength;
    Result.Ptr = (ak_fbx_u8*)DstBuffer;

    return Result;
}

static ak_fbx_string AK_FBX__Make_String(ak_fbx__arena* Arena, const char* Str, ak_fbx_u32 StrLength) {
    char* Buffer = (char*)AK_FBX__Arena_Push(Arena, (StrLength+1)*sizeof(char));
    Buffer[StrLength] = 0;
    AK_FBX_STRNCPY(Buffer, Str, StrLength);

    ak_fbx_string Result;
    Result.Str = Buffer;
    Result.Size = StrLength;
    return Result;
}

typedef struct ak_fbx__bool_array {
    ak_fbx_s8* Ptr;
    ak_fbx_u32 Count;
} ak_fbx__bool_array;

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
        ak_fbx_string      String;
        ak_fbx__bool_array BoolArray;
        ak_fbx_s32_array   S32Array;
        ak_fbx__f32_array  F32Array;
        ak_fbx__s64_array  S64Array;
        ak_fbx__f64_array  F64Array;
    } Data;
} ak_fbx__property;

typedef struct ak_fbx__property_array {
    ak_fbx__property* Ptr;
    ak_fbx_u64 Count;
} ak_fbx__property_array;

typedef struct ak_fbx__parsing_node {
    ak_fbx_string          Name;
    ak_fbx__property_array Properties;

    struct ak_fbx__parsing_node* Parent;
    struct ak_fbx__parsing_node* FirstChild;
    struct ak_fbx__parsing_node* LastChild;
    struct ak_fbx__parsing_node* PrevSibling;
    struct ak_fbx__parsing_node* NextSibling;
} ak_fbx__parsing_node;

void DEBUG_Print_Property(const ak_fbx__property* Property) {
    switch(Property->Type) {

        case AK_FBX__PROPERTY_TYPE_BOOL: {
            if(Property->Data.Bool) {
                printf("true");
            } else {
                printf("false");
            }
        } break;
        
        case AK_FBX__PROPERTY_TYPE_S16: {
            printf("%d", Property->Data.S16);
        } break;

        case AK_FBX__PROPERTY_TYPE_S32: {
            printf("%d", Property->Data.S32);
        } break;
        
        case AK_FBX__PROPERTY_TYPE_F32: {
            printf("%f", Property->Data.F32);
        } break;

        case AK_FBX__PROPERTY_TYPE_F64: {
            printf("%f", Property->Data.F64);
        } break;

        case AK_FBX__PROPERTY_TYPE_S64: {
            printf("%lld", Property->Data.S64);
        } break;

        case AK_FBX__PROPERTY_TYPE_BUFFER: {
            fwrite(Property->Data.Buffer.Ptr, 1, Property->Data.Buffer.Length, stdout);
        } break;

        case AK_FBX__PROPERTY_TYPE_STRING: {
            printf("%.*s", Property->Data.String.Size, Property->Data.String.Str);
        } break;

        case AK_FBX__PROPERTY_TYPE_BOOL_ARRAY: {
            for(ak_fbx_u32 i = 0; i < Property->Data.BoolArray.Count; i++) {
                if(Property->Data.BoolArray.Ptr[i]) {
                    printf("true");
                } else {
                    printf("false");
                }

                if(i != Property->Data.BoolArray.Count-1) 
                    printf(", ");  
            }
        } break;

        case AK_FBX__PROPERTY_TYPE_S32_ARRAY: {
            for(ak_fbx_u32 i = 0; i < Property->Data.S32Array.Count; i++) {
                printf("%d", Property->Data.S32Array.Ptr[i]);
                if(i != Property->Data.S32Array.Count-1) 
                    printf(", ");  
            }
        } break;

        case AK_FBX__PROPERTY_TYPE_F32_ARRAY: {
            for(ak_fbx_u32 i = 0; i < Property->Data.F32Array.Count; i++) {
                printf("%f", Property->Data.F32Array.Ptr[i]);
                if(i != Property->Data.F32Array.Count-1) 
                    printf(", ");  
            }
        } break;

        case AK_FBX__PROPERTY_TYPE_S64_ARRAY: {
            for(ak_fbx_u32 i = 0; i < Property->Data.S64Array.Count; i++) {
                printf("%lld", Property->Data.S64Array.Ptr[i]);
                if(i != Property->Data.S64Array.Count-1) 
                    printf(", ");  
            }
        } break;

        case AK_FBX__PROPERTY_TYPE_F64_ARRAY: {
            for(ak_fbx_u32 i = 0; i < Property->Data.F64Array.Count; i++) {
                printf("%f", Property->Data.F64Array.Ptr[i]);
                if(i != Property->Data.F64Array.Count-1) 
                    printf(", ");  
            }
        } break;
    }
}

void DEBUG_Print_Node_Name(ak_fbx__parsing_node* Node, int Level) {
    for(int LevelIndex = 0; LevelIndex < Level; LevelIndex++) {
        printf("\t");
    }
    
    printf("%.*s: ", Node->Name.Size, Node->Name.Str);
    
    for(ak_fbx_u32 PropertyIndex = 0; PropertyIndex < Node->Properties.Count; PropertyIndex++) {
        DEBUG_Print_Property(&Node->Properties.Ptr[PropertyIndex]);
        if(PropertyIndex != Node->Properties.Count-1)
            printf(", ");   
    }

    printf("\n");
    for(ak_fbx__parsing_node* Child = Node->FirstChild; Child; Child = Child->NextSibling) {
        DEBUG_Print_Node_Name(Child, Level+1);
    }
}
 
static void AK_FBX__Parsing_Node_Add_Child(ak_fbx__parsing_node* Parent, ak_fbx__parsing_node* Child) {
    Child->Parent = Parent;
    AK_FBX__DLL_Push_Back_NP(Parent->FirstChild, Parent->LastChild, Child, NextSibling, PrevSibling);
}

static const ak_fbx__property* AK_FBX__Get_Property(ak_fbx__parsing_node* Node, ak_fbx_u64 PropertyIndex) {
    AK_FBX_ASSERT(PropertyIndex < Node->Properties.Count);
    return &Node->Properties.Ptr[PropertyIndex];
}

static ak_fbx_s32 AK_FBX__Property_Get_S32(const ak_fbx__property* Property) {
    AK_FBX_ASSERT(Property->Type == AK_FBX__PROPERTY_TYPE_S32);
    return Property->Data.S32;
}

static ak_fbx_s64 AK_FBX__Property_Get_S64(const ak_fbx__property* Property) {
    AK_FBX_ASSERT(Property->Type == AK_FBX__PROPERTY_TYPE_S64);
    return Property->Data.S64;
}

static double AK_FBX__Property_Get_F64(const ak_fbx__property* Property) {
    AK_FBX_ASSERT(Property->Type == AK_FBX__PROPERTY_TYPE_F64);
    return Property->Data.F64;
}

static ak_fbx_s32_array AK_FBX__Property_Get_S32_Array(const ak_fbx__property* Property) {
    AK_FBX_ASSERT(Property->Type == AK_FBX__PROPERTY_TYPE_S32_ARRAY);
    return Property->Data.S32Array;
}

static ak_fbx__f64_array AK_FBX__Property_Get_F64_Array(const ak_fbx__property* Property) {
    AK_FBX_ASSERT(Property->Type == AK_FBX__PROPERTY_TYPE_F64_ARRAY);
    return Property->Data.F64Array;
}

static ak_fbx_string AK_FBX__Property_Get_Str(const ak_fbx__property* Property) {
    AK_FBX_ASSERT(Property->Type == AK_FBX__PROPERTY_TYPE_STRING);
    return Property->Data.String;
}

typedef enum ak_fbx__binary_node_status {
    AK_FBX__BINARY_NODE_STATUS_SUCCESS,
    AK_FBX__BINARY_NODE_STATUS_TERMINATE,
    AK_FBX__BINARY_NODE_STATUS_ERROR
} ak_fbx__binary_node_status;

#define AK_FBX__BINARY_READ_WORD(stream) Is64Bit ? AK_FBX__Stream_Consume64(stream) : AK_FBX__Stream_Consume32(stream)

//Copy zlib from stb_image.h
//https://github.com/nothings/stb/blob/master/stb_image.h
static int ak_fbx__stbi_zlib_decode_buffer(char *obuffer, int olen, const char *ibuffer, int ilen);

static ak_fbx_s8 AK_FBX__Decompress_ZLib(void* OutputBuffer, ak_fbx_u32 OutputLength, const void* InputBuffer, ak_fbx_u32 InputLength) {
    //TODO: Should we do some validation on the compression length 
    return ak_fbx__stbi_zlib_decode_buffer((char*)OutputBuffer, (int)OutputLength, (const char*)InputBuffer, (int)InputLength) != -1;
}

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
            Property->Type = AK_FBX__PROPERTY_TYPE_BOOL;
            Property->Data.Bool = AK_FBX__Stream_Consume8(Stream); 
        } break;

        //32 bit signed integer
        case 'I': {
            Property->Type = AK_FBX__PROPERTY_TYPE_S32;
            Property->Data.S32 = AK_FBX__Stream_Consume32(Stream);
        } break;

        //32 bit floating point
        case 'F': {
            Property->Type = AK_FBX__PROPERTY_TYPE_F32;
            Property->Data.F32 = *(float*)AK_FBX__Stream_Consume(Stream, sizeof(float)); 
        } break;

        //64 bit floating point
        case 'D': {
            Property->Type = AK_FBX__PROPERTY_TYPE_F64;
            Property->Data.F64 = *(double*)AK_FBX__Stream_Consume(Stream, sizeof(double)); 
        } break;

        //64 bit signed integer
        case 'L': {
            Property->Type = AK_FBX__PROPERTY_TYPE_S64;
            Property->Data.S64 = AK_FBX__Stream_Consume64(Stream); 
        } break;

        //Raw binary data
        case 'R': {
            ak_fbx_u32 BufferLength = AK_FBX__Stream_Consume32(Stream);
            const void* Buffer = AK_FBX__Stream_Consume(Stream, BufferLength);

            Property->Type = AK_FBX__PROPERTY_TYPE_BUFFER;
            Property->Data.Buffer = AK_FBX__Make_Buffer(Arena, Buffer, BufferLength);
        } break;

        case 'S': {
            ak_fbx_u32 StrLength = AK_FBX__Stream_Consume32(Stream);
            const char* Str = (const char*)AK_FBX__Stream_Consume(Stream, StrLength);

            Property->Type = AK_FBX__PROPERTY_TYPE_STRING;
            Property->Data.String = AK_FBX__Make_String(Arena, Str, StrLength);
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

            ak_fbx_u32 Stride = 0;
                            
            switch(Type) {
                case 'b':
                case 'c': {
                    Stride = 1;
                } break;

                case 'f':
                case 'i': {
                    Stride = 4;
                } break;
                
                case 'l':
                case 'd': {
                    Stride = 8;
                } break;

                default: {
                    AK_FBX_ASSERT(!"Invalid case");
                } break;
            }

            const void* CompressedBuffer = AK_FBX__Stream_Consume(Stream, CompressedLength);
            ak_fbx_u32 UncompressedLength = ArrayLength*Stride;
            void* UncompressedBuffer = AK_FBX__Arena_Push(Arena, UncompressedLength);
;
            
            if(Encoding == 0) {
                //Normal length array
                if(UncompressedLength != CompressedLength) {
                    //TODO: Diagnostic and error logging
                    return ak_fbx__false; 
                }

                AK_FBX_MEMCPY(UncompressedBuffer, CompressedBuffer, UncompressedLength);
            } else if(Encoding == 1) {
                //GZip encoding array
                if(!AK_FBX__Decompress_ZLib(UncompressedBuffer, UncompressedLength, CompressedBuffer, CompressedLength)) {
                    return ak_fbx__false;
                }
            } else {
                //TODO: Diagnostic and error logging
                return ak_fbx__false;
            }

            switch(Type) {
                case 'b':
                case 'c': {
                    Property->Type = AK_FBX__PROPERTY_TYPE_BOOL_ARRAY;
                    Property->Data.BoolArray.Count = ArrayLength;
                    Property->Data.BoolArray.Ptr = (ak_fbx_s8*)UncompressedBuffer;
                } break;

                case 'f': {
                    Property->Type = AK_FBX__PROPERTY_TYPE_F32_ARRAY;
                    Property->Data.F32Array.Count = ArrayLength;
                    Property->Data.F32Array.Ptr = (float*)UncompressedBuffer;
                } break;

                case 'i': {
                    Property->Type = AK_FBX__PROPERTY_TYPE_S32_ARRAY;
                    Property->Data.S32Array.Count = ArrayLength;
                    Property->Data.S32Array.Ptr = (ak_fbx_s32*)UncompressedBuffer;
                } break;
                
                case 'l': {
                    Property->Type = AK_FBX__PROPERTY_TYPE_S64_ARRAY;
                    Property->Data.S64Array.Count = ArrayLength;
                    Property->Data.S64Array.Ptr = (ak_fbx_s64*)UncompressedBuffer;
                } break;

                case 'd': {
                    Property->Type = AK_FBX__PROPERTY_TYPE_F64_ARRAY;
                    Property->Data.F64Array.Count = ArrayLength;
                    Property->Data.F64Array.Ptr = (double*)UncompressedBuffer;
                } break;

                default: {
                    AK_FBX_ASSERT(!"Invalid case");
                } break;
            }
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

static ak_fbx__binary_node_status AK_FBX__Binary_Read_Node(ak_fbx__stream* Stream, ak_fbx__parsing_node* Node, ak_fbx__arena* Arena, ak_fbx_s8 Is64Bit) {
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

    Node->Properties = Properties;

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
            ak_fbx__parsing_node* ChildNode = AK_FBX__Arena_Push_Struct(Arena, ak_fbx__parsing_node);
            if(AK_FBX__Binary_Read_Node(&NestedStream, ChildNode, Arena, Is64Bit) == AK_FBX__BINARY_NODE_STATUS_ERROR) {
                return AK_FBX__BINARY_NODE_STATUS_ERROR;
            }

            AK_FBX__Parsing_Node_Add_Child(Node, ChildNode);
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

static ak_fbx__parsing_node* AK_FBX__Parse(ak_fbx__arena* Arena, const void* Buffer, ak_fbx_u64 Length) {
    ak_fbx__stream Stream;
    Stream.Stream = (const ak_fbx_u8*)Buffer;
    Stream.Length = Length;
    Stream.At = 0;

    ak_fbx__parsing_node* RootNode = AK_FBX__Arena_Push_Struct(Arena, ak_fbx__parsing_node);
    RootNode->Name = AK_FBX__Make_String(Arena, "Root", 4);

    if(AK_FBX__Str_Cmp_Raw((const char*)AK_FBX__Stream_Peek(&Stream), AK_FBX__STR_LIT_EXPAND("Kaydara FBX Binary"))) {
        AK_FBX__Stream_Skip(&Stream, 23);

        if(!AK_FBX__Stream_Is_Valid(&Stream)) {
            //TODO: Diagnostic and error logging.
            return ak_fbx__nullptr;
        }
        
        ak_fbx_u32 Version = AK_FBX__Stream_Consume32(&Stream);
        ak_fbx_s8 Is64Bit = Version >= 7500;

        while(AK_FBX__Stream_Is_Valid(&Stream)) {
            ak_fbx__parsing_node* ChildNode = AK_FBX__Arena_Push_Struct(Arena, ak_fbx__parsing_node);
            ak_fbx__binary_node_status Status = AK_FBX__Binary_Read_Node(&Stream, ChildNode, Arena, Is64Bit);

            if(Status == AK_FBX__BINARY_NODE_STATUS_ERROR) {
                //TODO: Diagnostic and error logging.
                return ak_fbx__nullptr;
            }

            if(Status == AK_FBX__BINARY_NODE_STATUS_TERMINATE) {
                break;
            }

            AK_FBX__Parsing_Node_Add_Child(RootNode, ChildNode);
        }

    } else {
        //FBX file is not binary
        AK_FBX_ASSERT(!"Not Implemented!");
    }

    return RootNode;
}

typedef struct ak_fbx_scene__impl {
    ak_fbx_scene  Base;
    void*         UserData;
    ak_fbx__arena Arena;
} ak_fbx_scene__impl;

typedef struct ak_fbx_node__impl {
    ak_fbx_node Node;

    union {
        ak_fbx_geometry* Geometry;
    } Object;
} ak_fbx_node__impl;

static void AK_FBX__Node_Add_Child(ak_fbx_node__impl* Parent, ak_fbx_node__impl* Child) {
    Child->Node.Parent = &Parent->Node;   
    AK_FBX__DLL_Push_Back_NP(Parent->Node.FirstChild, Parent->Node.LastChild, &Child->Node, NextSibling, PrevSibling);
}

static ak_fbx_u32 AK_FBX__Hash_U64(ak_fbx_u64 Key) {
    Key = (~Key) + (Key << 18); // Key = (Key << 18) - Key - 1;
    Key = Key ^ (Key >> 31);
    Key = Key * 21; // Key = (Key + (Key << 2)) + (Key << 4);
    Key = Key ^ (Key >> 11);
    Key = Key + (Key << 6);
    Key = Key ^ (Key >> 22);
    return (ak_fbx_u32)Key;
}

typedef struct ak_fbx__hash_slot {
    ak_fbx_u32 Hash;
    ak_fbx_u32 ItemIndex;
    ak_fbx_u32 BaseCount;
} ak_fbx__hash_slot;

static ak_fbx_u32 AK_FBX__Find_Free_Slot(ak_fbx__hash_slot* Slots, ak_fbx_u32 SlotMask, ak_fbx_u32 BaseSlot) {
    ak_fbx_u32 BaseCount = Slots[BaseSlot].BaseCount;
    ak_fbx_u32 Slot = BaseSlot;
    ak_fbx_u32 FirstFree = Slot;
    while (BaseCount) 
    {
        if (Slots[Slot].ItemIndex == (ak_fbx_u32)-1 && Slots[FirstFree].ItemIndex != (ak_fbx_u32)-1) FirstFree = Slot;
        ak_fbx_u32 SlotHash = Slots[Slot].Hash;
        ak_fbx_u32 SlotBase = (SlotHash & SlotMask);
        if (SlotBase == BaseSlot) 
            --BaseCount;
        Slot = (Slot + 1) & SlotMask;
    }
    
    Slot = FirstFree;
    while (Slots[Slot].ItemIndex != (ak_fbx_u32)-1) 
        Slot = (Slot + 1) & SlotMask;

    return Slot;
}

typedef struct ak_fbx__id_ptr_map {
    ak_fbx__hash_slot* Slots;
    ak_fbx_s64*        IDs;
    void**             Ptrs;
    ak_fbx_u32         SlotCapacity;
    ak_fbx_u32         ItemCapacity;
    ak_fbx_u32         Count;
} ak_fbx__id_ptr_map;

static ak_fbx_u32 AK_FBX__Ceil_Pow2(ak_fbx_u32 V) {
    V--;
    V |= V >> 1;
    V |= V >> 2;
    V |= V >> 4;
    V |= V >> 8;
    V |= V >> 16;
    V++;
    return V;
}

static ak_fbx__id_ptr_map AK_FBX__Empty_ID_Ptr_Map(void) {
    ak_fbx__id_ptr_map Result;
    Result.Slots = ak_fbx__nullptr;
    Result.IDs = ak_fbx__nullptr;
    Result.Ptrs = ak_fbx__nullptr;
    Result.SlotCapacity = 0;
    Result.ItemCapacity = 0;
    Result.Count = 0;
    return Result;
}

static ak_fbx__id_ptr_map AK_FBX__ID_Ptr_Map_Create(ak_fbx__arena* Arena, ak_fbx_u32 Count) {
    ak_fbx__id_ptr_map Result = AK_FBX__Empty_ID_Ptr_Map();

    ak_fbx_u32 ItemCapacity = Count;
    ak_fbx_u32 SlotCapacity = AK_FBX__Ceil_Pow2(ItemCapacity*2);

    size_t AllocationSize = ItemCapacity*(sizeof(ak_fbx_s64)+sizeof(void*));
    AllocationSize += SlotCapacity*sizeof(ak_fbx__hash_slot);

    Result.Slots = (ak_fbx__hash_slot*)AK_FBX__Arena_Push(Arena, AllocationSize);
    Result.IDs = (ak_fbx_s64*)(Result.Slots + SlotCapacity);
    Result.Ptrs = (void**)(Result.IDs + ItemCapacity);
    Result.SlotCapacity = SlotCapacity;
    Result.ItemCapacity = ItemCapacity;
    Result.Count = 0;

    for(ak_fbx_u32 SlotIndex = 0; SlotIndex < SlotCapacity; SlotIndex++) {
        ak_fbx__hash_slot Slot;
        Slot.Hash = 0;
        Slot.ItemIndex = (ak_fbx_u32)-1;
        Slot.BaseCount = 0;

        Result.Slots[SlotIndex] = Slot;
    }

    return Result;
}

static ak_fbx_u32 AK_FBX__ID_Ptr_Map_Find_Slot(ak_fbx__id_ptr_map* Map, ak_fbx_s64 Key, ak_fbx_u32 Hash) {
    ak_fbx_u32 SlotMask = Map->SlotCapacity-1;
    ak_fbx_u32 BaseSlot = (Hash & SlotMask);
    ak_fbx_u32 BaseCount = Map->Slots[BaseSlot].BaseCount;
    ak_fbx_u32 Slot = BaseSlot;
    
    while (BaseCount > 0) {
        if (Map->Slots[Slot].ItemIndex != (ak_fbx_u32)-1) {
            ak_fbx_u32 SlotHash = Map->Slots[Slot].Hash;
            ak_fbx_u32 SlotBase = (SlotHash & SlotMask);
            if (SlotBase == BaseSlot) {
                AK_FBX_ASSERT(BaseCount > 0);
                BaseCount--;
                            
                if (SlotHash == Hash) { 
                    if(Key == Map->IDs[Map->Slots[Slot].ItemIndex]) {
                        return Slot;
                    }
                }
            }
        }
        
        Slot = (Slot + 1) & SlotMask;
    }
    
    return (ak_fbx_u32)-1;
}

static void AK_FBX__ID_Ptr_Map_Add(ak_fbx__id_ptr_map* Map, ak_fbx_s64 Key, void* Ptr) {
    ak_fbx_u32 Hash = AK_FBX__Hash_U64(Key);
    AK_FBX_ASSERT(AK_FBX__ID_Ptr_Map_Find_Slot(Map, Key, Hash) == -1);

    ak_fbx_u32 SlotMask = Map->SlotCapacity-1;
    ak_fbx_u32 BaseSlot = (Hash & SlotMask);
    ak_fbx_u32 Slot = AK_FBX__Find_Free_Slot(Map->Slots, SlotMask, BaseSlot);
    
    AK_FBX_ASSERT(Map->Count < Map->ItemCapacity);
    AK_FBX_ASSERT(Map->Slots[Slot].ItemIndex == (ak_fbx_u32)-1 && (Hash & SlotMask) == BaseSlot);
    
    Map->Slots[Slot].Hash = Hash;
    Map->Slots[Slot].ItemIndex = Map->Count;
    Map->Slots[BaseSlot].BaseCount++;
    
    Map->IDs[Map->Count] = Key;
    Map->Ptrs[Map->Count] = Ptr;

    Map->Count++;
}

static void* AK_FBX__ID_Ptr_Map_Get(ak_fbx__id_ptr_map* Map, ak_fbx_s64 Key) {
    ak_fbx_u32 Hash = AK_FBX__Hash_U64(Key);
    ak_fbx_u32 Slot = AK_FBX__ID_Ptr_Map_Find_Slot(Map, Key, Hash);
    if(Slot == (ak_fbx_u32)-1) {
        return ak_fbx__nullptr;
    }
    return Map->Ptrs[Map->Slots[Slot].ItemIndex];
}

typedef struct ak_fbx__definitions {
    ak_fbx_u32 ModelCount;
    ak_fbx_u32 GeometryCount;
} ak_fbx__definitions;

typedef enum ak_fbx__object_type {
    AK_FBX__OBJECT_TYPE_NODE,
    AK_FBX__OBJECT_TYPE_GEOMETRY,
    AK_FBX__OBJECT_TYPE_COUNT
} ak_fbx__object_type;

typedef struct ak_fbx__object {
    ak_fbx__object_type Type;
    void* Ptr;
} ak_fbx__object;

typedef struct ak_fbx__objects {
    ak_fbx_u32         NodeCount;
    ak_fbx_u32         GeometryCount;
    ak_fbx_node__impl* Nodes;
    ak_fbx_geometry*   Geometries;
    ak_fbx__id_ptr_map ObjectIDMap;
} ak_fbx__objects;

static ak_fbx_s8 AK_FBX__Validate_Objects(ak_fbx__objects* Objects) {
    if(!Objects->Nodes) {
        //TODO: Diagnostic and error logging
        return ak_fbx__false;
    }

    return ak_fbx__true;
}

static ak_fbx_s32 AK_FBX__Parse_Count(ak_fbx__parsing_node* ParsingNode) {
    for(ak_fbx__parsing_node* Child = ParsingNode->FirstChild; Child; Child = Child->NextSibling) {
        if(AK_FBX__Str_Cmp(Child->Name, AK_FBX__STR_LIT_EXPAND("Count"))) {
            ak_fbx_s32 Count = AK_FBX__Property_Get_S32(AK_FBX__Get_Property(Child, 0));
            return Count;
        }
    }
    AK_FBX_ASSERT(ak_fbx__false);
    return -1;
}

static void AK_FBX__Parse_Definitions(ak_fbx__definitions* Definitions, ak_fbx__parsing_node* DefintionNode) {
    //TODO: Might need to parse the version and determine if different version have different formats

    for(ak_fbx__parsing_node* ParsingNode = DefintionNode->FirstChild; ParsingNode; ParsingNode = ParsingNode->NextSibling) {
        if(AK_FBX__Str_Cmp(ParsingNode->Name, AK_FBX__STR_LIT_EXPAND("ObjectType"))) {
            ak_fbx_string TypeStr = AK_FBX__Property_Get_Str(AK_FBX__Get_Property(ParsingNode, 0));
            if(AK_FBX__Str_Cmp(TypeStr, AK_FBX__STR_LIT_EXPAND("Model"))) {
                Definitions->ModelCount = (ak_fbx_u32)AK_FBX__Parse_Count(ParsingNode);
            } else if(AK_FBX__Str_Cmp(TypeStr, AK_FBX__STR_LIT_EXPAND("Geometry"))) {
                Definitions->GeometryCount = (ak_fbx_u32)AK_FBX__Parse_Count(ParsingNode);
            }
        }
    }
}

#pragma warning(disable : 4189)

static void AK_FBX__Parse_Model(ak_fbx__objects* Objects, ak_fbx__parsing_node* ModelNode, ak_fbx__arena* TempArena, ak_fbx__arena* Arena) {
    //TODO: Might need to parse the version and determine if different version have different formats

    ak_fbx_node__impl* NodeImpl = &Objects->Nodes[Objects->NodeCount++];
    ak_fbx_node* Node = &NodeImpl->Node;
    
    //Model nodes start have an ID property, a name property, and a type property
    ak_fbx_s64 ID = AK_FBX__Property_Get_S64(AK_FBX__Get_Property(ModelNode, 0));
    ak_fbx_string String = AK_FBX__Property_Get_Str(AK_FBX__Get_Property(ModelNode, 1));

    Node->Name = AK_FBX__Make_String(Arena, String.Str, String.Size);
    
    //Always set the global to the identity matrix. We will update it later after parsing the scene
    AK_FBX_M4x3_Identity(&Node->GlobalTransform);

    ak_fbx_v3 LclTranslation; AK_FBX_V3_Zero(&LclTranslation);
    ak_fbx_v3 LclRotation; AK_FBX_V3_Zero(&LclRotation);
    ak_fbx_v3 LclScale; AK_FBX_V3(&LclScale, 1.0, 1.0, 1.0);

    for(ak_fbx__parsing_node* ChildNode = ModelNode->FirstChild; ChildNode; ChildNode = ChildNode->NextSibling) {
        //TODO: Might need to parse the version and determine if different version have different formats

        //Find the properties
        if(AK_FBX__Str_Cmp(ChildNode->Name, AK_FBX__STR_LIT_EXPAND("Properties70"))) {
            ak_fbx__parsing_node* PropertiesNode = ChildNode;

            for(ak_fbx__parsing_node* PropertyNode = PropertiesNode->FirstChild; PropertyNode; PropertyNode = PropertyNode->NextSibling) {
                ak_fbx_string PropertyName = AK_FBX__Property_Get_Str(AK_FBX__Get_Property(PropertyNode, 0));

                if(AK_FBX__Str_Cmp(PropertyName, AK_FBX__STR_LIT_EXPAND("Lcl Translation"))) {
                    LclTranslation.Data[0] = AK_FBX__Property_Get_F64(AK_FBX__Get_Property(PropertyNode, 4));
                    LclTranslation.Data[1] = AK_FBX__Property_Get_F64(AK_FBX__Get_Property(PropertyNode, 5));
                    LclTranslation.Data[2] = AK_FBX__Property_Get_F64(AK_FBX__Get_Property(PropertyNode, 6));
                } else if(AK_FBX__Str_Cmp(PropertyName, AK_FBX__STR_LIT_EXPAND("Lcl Scaling"))) {
                    LclScale.Data[0] = AK_FBX__Property_Get_F64(AK_FBX__Get_Property(PropertyNode, 4));
                    LclScale.Data[1] = AK_FBX__Property_Get_F64(AK_FBX__Get_Property(PropertyNode, 5));
                    LclScale.Data[2] = AK_FBX__Property_Get_F64(AK_FBX__Get_Property(PropertyNode, 6));
                } else if(AK_FBX__Str_Cmp(PropertyName, AK_FBX__STR_LIT_EXPAND("Lcl Rotation"))) {
                    LclRotation.Data[0] = AK_FBX__Property_Get_F64(AK_FBX__Get_Property(PropertyNode, 4));
                    LclRotation.Data[1] = AK_FBX__Property_Get_F64(AK_FBX__Get_Property(PropertyNode, 5));
                    LclRotation.Data[2] = AK_FBX__Property_Get_F64(AK_FBX__Get_Property(PropertyNode, 6));
                }
            }
        }
    }

    ak_fbx__m3 Pitch; AK_FBX__M3_RotX(&Pitch, AK_FBX__To_Radians(LclRotation.Data[0]));
    ak_fbx__m3 Yaw; AK_FBX__M3_RotY(&Yaw, AK_FBX__To_Radians(LclRotation.Data[1]));
    ak_fbx__m3 Roll; AK_FBX__M3_RotZ(&Roll, AK_FBX__To_Radians(LclRotation.Data[2]));

    ak_fbx__m3 TempOrientation;
    ak_fbx__m3 Orientation;
    AK_FBX__M3_Mul(&TempOrientation, &Pitch, &Yaw);
    AK_FBX__M3_Mul(&Orientation, &Roll, &TempOrientation);

    Node->LocalTransform.Data[0]  = Orientation.Data[0]*LclScale.Data[0];
    Node->LocalTransform.Data[1]  = Orientation.Data[1]*LclScale.Data[0];
    Node->LocalTransform.Data[2]  = Orientation.Data[2]*LclScale.Data[0];

    Node->LocalTransform.Data[3]  = Orientation.Data[3]*LclScale.Data[1];
    Node->LocalTransform.Data[4]  = Orientation.Data[4]*LclScale.Data[1];
    Node->LocalTransform.Data[5]  = Orientation.Data[5]*LclScale.Data[1];

    Node->LocalTransform.Data[6]  = Orientation.Data[6]*LclScale.Data[2];
    Node->LocalTransform.Data[7]  = Orientation.Data[7]*LclScale.Data[2];
    Node->LocalTransform.Data[8]  = Orientation.Data[8]*LclScale.Data[2];

    Node->LocalTransform.Data[9]  = LclTranslation.Data[0];
    Node->LocalTransform.Data[10] = LclTranslation.Data[1];
    Node->LocalTransform.Data[11] = LclTranslation.Data[2];

    ak_fbx__object* Object = AK_FBX__Arena_Push_Struct(TempArena, ak_fbx__object);
    Object->Type = AK_FBX__OBJECT_TYPE_NODE;
    Object->Ptr = Node;
    AK_FBX__ID_Ptr_Map_Add(&Objects->ObjectIDMap, ID, Object);
}

typedef struct ak_fbx_polygon__entry {
    ak_fbx_polygon Polygon;
    struct ak_fbx_polygon__entry* Next;
} ak_fbx_polygon__entry;

typedef struct ak_fbx_polygon__list {
    ak_fbx_polygon__entry* First;
    ak_fbx_polygon__entry* Last;
    ak_fbx_u32 Count;
} ak_fbx_polygon__list;

typedef enum ak_fbx__mapping_information_type {
    AK_FBX__MAPPING_INFORMATION_TYPE_UNKNOWN,
    AK_FBX__MAPPING_INFORMATION_TYPE_BY_POLYGON_VERTEX
} ak_fbx__mapping_information_type;

typedef enum ak_fbx__reference_information_type {
    AK_FBX__REFERENCE_INFORMATION_TYPE_UNKNOWN,
    AK_FBX__REFERENCE_INFORMATION_TYPE_DIRECT,
    AK_FBX__REFERENCE_INFORMATION_TYPE_INDEX_TO_DIRECT
} ak_fbx__reference_information_type;

static void AK_FBX__Parse_Normal(ak_fbx_geometry* Geometry, ak_fbx__parsing_node* NormalLayer, ak_fbx__arena* Arena) {
    //TODO: Might need to parse the version and determine if different version have different formats

    ak_fbx__mapping_information_type MappingInformationType = AK_FBX__MAPPING_INFORMATION_TYPE_UNKNOWN;
    ak_fbx__reference_information_type ReferenceInformationType = AK_FBX__REFERENCE_INFORMATION_TYPE_UNKNOWN;

    for(ak_fbx__parsing_node* Node = NormalLayer->FirstChild; Node; Node = Node->NextSibling) {
        if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("MappingInformationType"))) {
            ak_fbx_string Value = AK_FBX__Property_Get_Str(AK_FBX__Get_Property(Node, 0));
            if(AK_FBX__Str_Cmp(Value, AK_FBX__STR_LIT_EXPAND("ByPolygonVertex"))) {
                MappingInformationType = AK_FBX__MAPPING_INFORMATION_TYPE_BY_POLYGON_VERTEX;
            } else {
                AK_FBX_ASSERT(!"Not Implemented");
            }
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("ReferenceInformationType"))) {
            ak_fbx_string Value = AK_FBX__Property_Get_Str(AK_FBX__Get_Property(Node, 0));
            if(AK_FBX__Str_Cmp(Value, AK_FBX__STR_LIT_EXPAND("Direct"))) {
                ReferenceInformationType = AK_FBX__REFERENCE_INFORMATION_TYPE_DIRECT;
            } else if(AK_FBX__Str_Cmp(Value, AK_FBX__STR_LIT_EXPAND("IndexToDirect"))) {
                ReferenceInformationType = AK_FBX__REFERENCE_INFORMATION_TYPE_INDEX_TO_DIRECT;
            } else {
                AK_FBX_ASSERT(!"Not Implemented");
            } 
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("Normals"))) {
            AK_FBX_ASSERT(MappingInformationType != AK_FBX__MAPPING_INFORMATION_TYPE_UNKNOWN);
            AK_FBX_ASSERT(ReferenceInformationType != AK_FBX__REFERENCE_INFORMATION_TYPE_UNKNOWN);

            switch(MappingInformationType) {
                case AK_FBX__MAPPING_INFORMATION_TYPE_BY_POLYGON_VERTEX: {
                    ak_fbx__f64_array Normals = AK_FBX__Property_Get_F64_Array(AK_FBX__Get_Property(Node, 0));

                    Geometry->Normals.Count = Normals.Count/3;
                    Geometry->Normals.Ptr = AK_FBX__Arena_Push_Array(Arena, Geometry->Normals.Count, ak_fbx_v3);

                    AK_FBX_MEMCPY(Geometry->Normals.Ptr, Normals.Ptr, Normals.Count*sizeof(double));

                    if(ReferenceInformationType == AK_FBX__REFERENCE_INFORMATION_TYPE_DIRECT) {
                        AK_FBX_ASSERT(Geometry->Normals.Count == Geometry->Polygons.VertexIndices.Count);

                        Geometry->Polygons.NormalIndices.Count = Geometry->Polygons.VertexIndices.Count;
                        Geometry->Polygons.NormalIndices.Ptr = AK_FBX__Arena_Push_Array(Arena, Geometry->Polygons.NormalIndices.Count, ak_fbx_s32);

                        for(ak_fbx_s32 i = 0; i < (ak_fbx_s32)Geometry->Polygons.NormalIndices.Count; i++) {
                            Geometry->Polygons.NormalIndices.Ptr[i] = i;
                        }     
                    }
                } break;
            }
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("NormalIndex"))) {
            AK_FBX_ASSERT(ReferenceInformationType == AK_FBX__REFERENCE_INFORMATION_TYPE_INDEX_TO_DIRECT);

            ak_fbx_s32_array NormalIndices = AK_FBX__Property_Get_S32_Array(AK_FBX__Get_Property(Node, 0));
            AK_FBX_ASSERT(NormalIndices.Count == Geometry->Polygons.VertexIndices.Count);

            Geometry->Polygons.NormalIndices.Count = NormalIndices.Count;
            Geometry->Polygons.NormalIndices.Ptr = AK_FBX__Arena_Push_Array(Arena, Geometry->Polygons.NormalIndices.Count, ak_fbx_s32);
            AK_FBX_MEMCPY(Geometry->Polygons.NormalIndices.Ptr, NormalIndices.Ptr, NormalIndices.Count*sizeof(ak_fbx_s32));
        }
    }
}

static void AK_FBX__Parse_UV_Map(ak_fbx_geometry* Geometry, ak_fbx__parsing_node* UVLayer, ak_fbx__arena* Arena) {
    //TODO: Might need to parse the version and determine if different version have different formats
    ak_fbx_uv_map* Map = &Geometry->UVMaps.Ptr[Geometry->UVMaps.Count++];

    ak_fbx__mapping_information_type MappingInformationType = AK_FBX__MAPPING_INFORMATION_TYPE_UNKNOWN;
    ak_fbx__reference_information_type ReferenceInformationType = AK_FBX__REFERENCE_INFORMATION_TYPE_UNKNOWN;
    
    for(ak_fbx__parsing_node* Node = UVLayer->FirstChild; Node; Node = Node->NextSibling) {
        if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("Name"))) {
            ak_fbx_string Value = AK_FBX__Property_Get_Str(AK_FBX__Get_Property(Node, 0));
            Map->Name = AK_FBX__Make_String(Arena, Value.Str, Value.Size);
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("MappingInformationType"))) {
            ak_fbx_string Value = AK_FBX__Property_Get_Str(AK_FBX__Get_Property(Node, 0));
            if(AK_FBX__Str_Cmp(Value, AK_FBX__STR_LIT_EXPAND("ByPolygonVertex"))) {
                MappingInformationType = AK_FBX__MAPPING_INFORMATION_TYPE_BY_POLYGON_VERTEX;
            } else {
                AK_FBX_ASSERT(!"Not Implemented");
            }
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("ReferenceInformationType"))) {
            ak_fbx_string Value = AK_FBX__Property_Get_Str(AK_FBX__Get_Property(Node, 0));
            if(AK_FBX__Str_Cmp(Value, "Direct", Value.Size)) {
                ReferenceInformationType = AK_FBX__REFERENCE_INFORMATION_TYPE_DIRECT;
            } else if(AK_FBX__Str_Cmp(Value, AK_FBX__STR_LIT_EXPAND("IndexToDirect"))) {
                ReferenceInformationType = AK_FBX__REFERENCE_INFORMATION_TYPE_INDEX_TO_DIRECT;
            } else {
                AK_FBX_ASSERT(!"Not Implemented");
            } 
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("UV"))) {
            AK_FBX_ASSERT(MappingInformationType != AK_FBX__MAPPING_INFORMATION_TYPE_UNKNOWN);
            AK_FBX_ASSERT(ReferenceInformationType != AK_FBX__REFERENCE_INFORMATION_TYPE_UNKNOWN);

            switch(MappingInformationType) {
                case AK_FBX__MAPPING_INFORMATION_TYPE_BY_POLYGON_VERTEX: {
                    ak_fbx__f64_array UVs = AK_FBX__Property_Get_F64_Array(AK_FBX__Get_Property(Node, 0));

                    Map->UVs.Count = UVs.Count/2;
                    Map->UVs.Ptr = AK_FBX__Arena_Push_Array(Arena, Map->UVs.Count, ak_fbx_v2);

                    AK_FBX_MEMCPY(Map->UVs.Ptr, UVs.Ptr, UVs.Count*sizeof(double));

                    if(ReferenceInformationType == AK_FBX__REFERENCE_INFORMATION_TYPE_DIRECT) {
                        AK_FBX_ASSERT(Map->UVs.Count == Geometry->Polygons.VertexIndices.Count);

                        Map->UVIndices.Count = Geometry->Polygons.VertexIndices.Count;
                        Map->UVIndices.Ptr = AK_FBX__Arena_Push_Array(Arena, Map->UVIndices.Count, ak_fbx_s32);

                        for(ak_fbx_s32 i = 0; i < (ak_fbx_s32)Map->UVIndices.Count; i++) {
                            Map->UVIndices.Ptr[i] = i;
                        }     
                    }
                } break;
            }
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("UVIndex"))) {
            AK_FBX_ASSERT(ReferenceInformationType == AK_FBX__REFERENCE_INFORMATION_TYPE_INDEX_TO_DIRECT);

            ak_fbx_s32_array UVIndices = AK_FBX__Property_Get_S32_Array(AK_FBX__Get_Property(Node, 0));
            AK_FBX_ASSERT(UVIndices.Count == Geometry->Polygons.VertexIndices.Count);

            Map->UVIndices.Count = UVIndices.Count;
            Map->UVIndices.Ptr = AK_FBX__Arena_Push_Array(Arena, UVIndices.Count, ak_fbx_s32);
            AK_FBX_MEMCPY(Map->UVIndices.Ptr, UVIndices.Ptr, UVIndices.Count*sizeof(ak_fbx_s32));
        }
    }
}

#pragma warning(disable : 4100)
static void AK_FBX__Parse_Geometry(ak_fbx__objects* Objects, ak_fbx__parsing_node* GeometryNode, ak_fbx__arena* TempArena, ak_fbx__arena* Arena) {
    //TODO: Might need to parse the version and determine if different version have different formats

    ak_fbx_geometry* Geometry = &Objects->Geometries[Objects->GeometryCount++];
    ak_fbx_u32 UVMapCount = 0;

    for(ak_fbx__parsing_node* Node = GeometryNode->FirstChild; Node; Node = Node->NextSibling) {
        if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("Vertices"))) {
            ak_fbx__f64_array Vertices = AK_FBX__Property_Get_F64_Array(AK_FBX__Get_Property(Node, 0));

            //3 components per vertex
            Geometry->Vertices.Count = Vertices.Count/3;
            Geometry->Vertices.Ptr = AK_FBX__Arena_Push_Array(Arena, Geometry->Vertices.Count, ak_fbx_v3);
            AK_FBX_MEMCPY(Geometry->Vertices.Ptr, Vertices.Ptr, Vertices.Count*sizeof(double));
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("PolygonVertexIndex"))) {
            ak_fbx_s32_array PolygonVertexIndex = AK_FBX__Property_Get_S32_Array(AK_FBX__Get_Property(Node, 0));

            Geometry->Polygons.VertexIndices.Count = PolygonVertexIndex.Count;
            Geometry->Polygons.VertexIndices.Ptr = AK_FBX__Arena_Push_Array(Arena, PolygonVertexIndex.Count, ak_fbx_s32);

            ak_fbx_polygon__list Polygons;
            AK_FBX_MEMSET(&Polygons, 0, sizeof(ak_fbx_polygon__list));

            ak_fbx_u32 StartPolygonOffset = 0;
            for(ak_fbx_u32 Index = 0; Index < PolygonVertexIndex.Count; Index++) {
                if(PolygonVertexIndex.Ptr[Index] < 0) {

                    Geometry->Polygons.VertexIndices.Ptr[Index] = ~PolygonVertexIndex.Ptr[Index];
                    
                    ak_fbx_polygon__entry* Entry = AK_FBX__Arena_Push_Struct(TempArena, ak_fbx_polygon__entry);
                    Entry->Polygon.IndexArrayOffset = StartPolygonOffset;
                    Entry->Polygon.IndexArrayCount  = (Index - StartPolygonOffset)+1;  

                    StartPolygonOffset = Index+1;

                    Polygons.Count++;
                    AK_FBX__SLL_Push_Back(Polygons.First, Polygons.Last, Entry);
                } else {
                    Geometry->Polygons.VertexIndices.Ptr[Index] = PolygonVertexIndex.Ptr[Index];
                }
            }

            Geometry->Polygons.PolygonArray.Count = Polygons.Count;
            Geometry->Polygons.PolygonArray.Ptr = AK_FBX__Arena_Push_Array(Arena, Polygons.Count, ak_fbx_polygon);

            ak_fbx_u32 PolygonIndex = 0;
            for(ak_fbx_polygon__entry* Entry = Polygons.First; Entry; Entry = Entry->Next) {
                Geometry->Polygons.PolygonArray.Ptr[PolygonIndex++] = Entry->Polygon;
            }
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("LayerElementNormal"))) {
            AK_FBX__Parse_Normal(Geometry, Node, Arena);
        } else if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("LayerElementUV"))) {
            UVMapCount++;
        }
    }

    Geometry->UVMaps.Ptr = AK_FBX__Arena_Push_Array(Arena, UVMapCount, ak_fbx_uv_map);
    for(ak_fbx__parsing_node* Node = GeometryNode->FirstChild; Node; Node = Node->NextSibling) {
        if(AK_FBX__Str_Cmp(Node->Name, AK_FBX__STR_LIT_EXPAND("LayerElementUV"))) {
            AK_FBX__Parse_UV_Map(Geometry, Node, Arena);
        }
    }

    //Geometry nodes start have an ID property
    ak_fbx_s64 ID = AK_FBX__Property_Get_S64(AK_FBX__Get_Property(GeometryNode, 0));

    ak_fbx__object* Object = AK_FBX__Arena_Push_Struct(TempArena, ak_fbx__object);
    Object->Type = AK_FBX__OBJECT_TYPE_GEOMETRY;
    Object->Ptr = Geometry;
    AK_FBX__ID_Ptr_Map_Add(&Objects->ObjectIDMap, ID, Object);
}

static void AK_FBX__Parse_Objects(ak_fbx__objects* Objects, ak_fbx__parsing_node* ObjectNode, ak_fbx__arena* TempArena, ak_fbx__arena* Arena) {
    for(ak_fbx__parsing_node* ParsingNode = ObjectNode->FirstChild; ParsingNode; ParsingNode = ParsingNode->NextSibling) {
        if(AK_FBX__Str_Cmp(ParsingNode->Name, AK_FBX__STR_LIT_EXPAND("Model"))) {
            AK_FBX_ASSERT(Objects->Nodes);
            AK_FBX__Parse_Model(Objects, ParsingNode, TempArena, Arena);
        } else if(AK_FBX__Str_Cmp(ParsingNode->Name, AK_FBX__STR_LIT_EXPAND("Geometry"))) {
            AK_FBX_ASSERT(Objects->Geometries);
            AK_FBX__Parse_Geometry(Objects, ParsingNode, TempArena, Arena);
        }
    }
}

#define AK_FBX__CONNECT_OBJECTS_FUNC(name) void name(ak_fbx__object* ObjectA, ak_fbx__object* ObjectB)
typedef AK_FBX__CONNECT_OBJECTS_FUNC(ak_fbx__connect_object_funcs);

static AK_FBX__CONNECT_OBJECTS_FUNC(AK_FBX__Connect_Nodes) {
    //Parent child connection. NodeB is parent, NodeA is child
    ak_fbx_node__impl* NodeA = (ak_fbx_node__impl*)ObjectA->Ptr;
    ak_fbx_node__impl* NodeB = (ak_fbx_node__impl*)ObjectB->Ptr;
    AK_FBX__Node_Add_Child(NodeB, NodeA);
}

static AK_FBX__CONNECT_OBJECTS_FUNC(AK_FBX__Connect_Geometry_And_Node) {
    ak_fbx_geometry* GeometryA = (ak_fbx_geometry*)ObjectA->Ptr;
    ak_fbx_node__impl* NodeB = (ak_fbx_node__impl*)ObjectB->Ptr;

    NodeB->Node.Type = AK_FBX_NODE_TYPE_GEOMETRY;
    NodeB->Object.Geometry = GeometryA;
    GeometryA->Node = (ak_fbx_node*)NodeB;
}

static ak_fbx__connect_object_funcs* AK_FBX__Connect_Objects_Funcs[AK_FBX__OBJECT_TYPE_COUNT][AK_FBX__OBJECT_TYPE_COUNT] = {
    {AK_FBX__Connect_Nodes, ak_fbx__nullptr},
    {AK_FBX__Connect_Geometry_And_Node, ak_fbx__nullptr}
};

static void AK_FBX__Parse_Connections(ak_fbx__parsing_node* ConnectionNode, ak_fbx__objects* Objects) {
    for(ak_fbx__parsing_node* ParsingNode = ConnectionNode->FirstChild; ParsingNode; ParsingNode = ParsingNode->NextSibling) {
        //ak_fbx_string ConnectionType = AK_FBX__Property_Get_Str(AK_FBX__Get_Property(ParsingNode, 0));
        ak_fbx_s64 AID = AK_FBX__Property_Get_S64(AK_FBX__Get_Property(ParsingNode, 1));
        ak_fbx_s64 BID = AK_FBX__Property_Get_S64(AK_FBX__Get_Property(ParsingNode, 2));

        ak_fbx__object* ObjectA = (ak_fbx__object*)AK_FBX__ID_Ptr_Map_Get(&Objects->ObjectIDMap, AID);
        ak_fbx__object* ObjectB = (ak_fbx__object*)AK_FBX__ID_Ptr_Map_Get(&Objects->ObjectIDMap, BID);
        if(ObjectA && ObjectB) {
            AK_FBX__Connect_Objects_Funcs[ObjectA->Type][ObjectB->Type](ObjectA, ObjectB);
        }
    }
}

static void AK_FBX__Get_Root_Node_ID(ak_fbx__parsing_node* ParsingNode, ak_fbx_s64* RootNodeID) {
    for(ak_fbx__parsing_node* Child = ParsingNode->FirstChild; Child; Child = Child->NextSibling) {
        if(AK_FBX__Str_Cmp(Child->Name, AK_FBX__STR_LIT_EXPAND("RootNode"))) {
            *RootNodeID = AK_FBX__Property_Get_S64(AK_FBX__Get_Property(Child, 0));
            return;
        }
    }
}

static ak_fbx_s8 AK_FBX__Parse_Scene(ak_fbx_scene__impl* Scene, ak_fbx__parsing_node* RootParsingNode, ak_fbx__arena* TempArena) {
    DEBUG_Print_Node_Name(RootParsingNode, 0);
    
    ak_fbx__objects Objects;
    AK_FBX_MEMSET(&Objects, 0, sizeof(ak_fbx__objects));

    ak_fbx__definitions Definitions;
    AK_FBX_MEMSET(&Definitions, 0, sizeof(ak_fbx__definitions));

    ak_fbx_s64 RootNodeID = -1;

    //Iterate first to get the scene data
    for(ak_fbx__parsing_node* ParsingNode = RootParsingNode->FirstChild; ParsingNode; ParsingNode = ParsingNode->NextSibling) {
        
        if(AK_FBX__Str_Cmp(ParsingNode->Name, AK_FBX__STR_LIT_EXPAND("Documents"))) {
            for(ak_fbx__parsing_node* ChildNode = ParsingNode->FirstChild; ChildNode; ChildNode = ChildNode->NextSibling) {
                if(AK_FBX__Str_Cmp(ChildNode->Name, AK_FBX__STR_LIT_EXPAND("Document"))) {
                    AK_FBX__Get_Root_Node_ID(ChildNode, &RootNodeID);
                }
            }
        } else if(AK_FBX__Str_Cmp(ParsingNode->Name, AK_FBX__STR_LIT_EXPAND("Definitions"))) {
            if(RootNodeID == -1) {
                //TODO: Diagnostic and error logging
                return ak_fbx__false;
            }

            AK_FBX__Parse_Definitions(&Definitions, ParsingNode);

            //Plus one for the root node
            ak_fbx_u32 ModelCount = Definitions.ModelCount+1;
            Objects.Nodes = AK_FBX__Arena_Push_Array(&Scene->Arena, ModelCount, ak_fbx_node__impl);
            AK_FBX_MEMSET(Objects.Nodes, 0, sizeof(ak_fbx_node__impl)*ModelCount);

            Objects.Geometries = AK_FBX__Arena_Push_Array(&Scene->Arena, Definitions.GeometryCount, ak_fbx_geometry);
            AK_FBX_MEMSET(Objects.Geometries, 0, sizeof(ak_fbx_geometry)*Definitions.GeometryCount);

            ak_fbx_u32 ObjectCount = ModelCount+Definitions.GeometryCount;
            Objects.ObjectIDMap = AK_FBX__ID_Ptr_Map_Create(TempArena, ObjectCount);

            ak_fbx_node__impl* RootNode = &Objects.Nodes[Objects.NodeCount++];
            AK_FBX_MEMSET(RootNode, 0, sizeof(ak_fbx_node__impl));

            RootNode->Node.Name = AK_FBX__Make_String(&Scene->Arena, "RootNode", 8);
            AK_FBX_M4x3_Identity(&RootNode->Node.GlobalTransform);
            AK_FBX_M4x3_Identity(&RootNode->Node.LocalTransform);

            ak_fbx__object* RootNodeObject = AK_FBX__Arena_Push_Struct(TempArena, ak_fbx__object);
            RootNodeObject->Type = AK_FBX__OBJECT_TYPE_NODE;
            RootNodeObject->Ptr = RootNode;

            AK_FBX__ID_Ptr_Map_Add(&Objects.ObjectIDMap, RootNodeID, RootNodeObject);

            Scene->Base.RootNode = &RootNode->Node;
        } else if(AK_FBX__Str_Cmp(ParsingNode->Name, AK_FBX__STR_LIT_EXPAND("Objects"))) {
            if(!AK_FBX__Validate_Objects(&Objects)) {
                return ak_fbx__false;
            }

            AK_FBX__Parse_Objects(&Objects, ParsingNode, TempArena, &Scene->Arena);
        } if(AK_FBX__Str_Cmp(ParsingNode->Name, AK_FBX__STR_LIT_EXPAND("Connections"))) {
            
            //Minus one for the root node
            if((Objects.NodeCount-1) != Definitions.ModelCount) {
                //TODO: Diagnostic and error logging
                return ak_fbx__false;
            }

            if(Objects.GeometryCount != Definitions.GeometryCount) {
                //TODO: Diagnostic and error logging
                return ak_fbx__false;
            }

            AK_FBX__Parse_Connections(ParsingNode, &Objects);
        }
    }

    //If everything has succeeded, we can now build the global transform hierarchy
    ak_fbx_node* RootNode = Scene->Base.RootNode;

    ak_fbx_u32 NodeStackIndex = 0;
    ak_fbx_node** NodeStack = AK_FBX__Arena_Push_Array(TempArena, Objects.NodeCount, ak_fbx_node*);

    NodeStack[NodeStackIndex++] = RootNode;
    while(NodeStackIndex) {
        ak_fbx_node* Node = NodeStack[--NodeStackIndex];

        if(Node->Parent) {
            AK_FBX__M4x3_Mul(&Node->GlobalTransform, &Node->LocalTransform, &Node->Parent->GlobalTransform);
        } else {
            AK_FBX_ASSERT(Node == RootNode);
            Node->GlobalTransform = Node->LocalTransform;
        }

        for(ak_fbx_node* ChildNode = Node->FirstChild; ChildNode; ChildNode = ChildNode->NextSibling) {
            NodeStack[NodeStackIndex++] = ChildNode;
        }
    }

    Scene->Base.Nodes.Ptr = AK_FBX__Arena_Push_Array(&Scene->Arena, Objects.NodeCount, ak_fbx_node*);
    Scene->Base.Geometries.Ptr = AK_FBX__Arena_Push_Array(&Scene->Arena, Objects.GeometryCount, ak_fbx_geometry*);

    for(ak_fbx_u32 NodeIndex = 0; NodeIndex < Objects.NodeCount; NodeIndex++) {
        Scene->Base.Nodes.Ptr[NodeIndex] = (ak_fbx_node*)(Objects.Nodes + NodeIndex);
    }

    for(ak_fbx_u32 GeometryIndex = 0; GeometryIndex < Objects.GeometryCount; GeometryIndex++) {
        Scene->Base.Geometries.Ptr[GeometryIndex] = Objects.Geometries + GeometryIndex;
    }
    
    Scene->Base.Nodes.Count = Objects.NodeCount;
    Scene->Base.Geometries.Count = Objects.GeometryCount;

    return ak_fbx__true;
}

AKFBXDEF ak_fbx_scene* AK_FBX_Load_From_Memory(const void* Buffer, ak_fbx_u64 Length, void* UserData) {
    ak_fbx_scene__impl* Scene = ak_fbx__nullptr;
    
    ak_fbx__arena TempArena = AK_FBX__Arena_Create(UserData);
    ak_fbx__parsing_node* RootParsingNode = AK_FBX__Parse(&TempArena, Buffer, Length);
    if(RootParsingNode) {
        ak_fbx__arena Arena = AK_FBX__Arena_Create(UserData);
        Scene = AK_FBX__Arena_Push_Struct(&Arena, ak_fbx_scene__impl); 
        Scene->Arena = Arena;
        Scene->UserData = UserData;

        if(!AK_FBX__Parse_Scene(Scene, RootParsingNode, &TempArena)) {
            AK_FBX__Arena_Delete(&Scene->Arena);
            Scene = ak_fbx__nullptr;
        }
    }
    AK_FBX__Arena_Delete(&TempArena);

    return &Scene->Base;
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
        ak_fbx_scene__impl* SceneImpl = (ak_fbx_scene__impl*)Scene;
        ak_fbx__arena* Arena = &SceneImpl->Arena;
        AK_FBX__Arena_Delete(Arena);
    }
}

//Node utils
AKFBXDEF ak_fbx_geometry* AK_FBX_Node_Get_Geometry(ak_fbx_node* _Node) {
    ak_fbx_node__impl* Node = (ak_fbx_node__impl*)_Node;
    if(Node->Node.Type != AK_FBX_NODE_TYPE_GEOMETRY) return NULL;
    return Node->Object.Geometry;
}

//If thread local storage is supported, this is thread safe
//If thread local storage is not supported, this is not thread safe
//If AK_FBX_NO_ERROR_MESSAGE is defined, this returns null
AKFBXDEF const char* AK_FBX_Error_Message(void) {
    return AK_FBX__G_Error_Message;
}

//~Math utility functions
AKFBXDEF void AK_FBX_V3_Zero(ak_fbx_v3* V) {
    V->Data[0] = 0;
    V->Data[1] = 0;
    V->Data[2] = 0;
}

AKFBXDEF void AK_FBX_V3(ak_fbx_v3* V, double x, double y, double z) {
    V->Data[0] = x;
    V->Data[1] = y;
    V->Data[2] = z;
}

AKFBXDEF void AK_FBX_M4x3_Identity(ak_fbx_m4x3* M) {
    AK_FBX_MEMSET(M->Data, 0, sizeof(M->Data));
    M->Data[0] = 1.0;
    M->Data[4] = 1.0;
    M->Data[8] = 1.0;
}

//~STB Image ZLib implementation (should never allocate memory!)

#define AK_FBX__STBI_ASSERT AK_FBX_ASSERT
#define ak_fbx__stbi__err(x, y) AK_FBX__Error(x, y)

#define AK_FBX__STBI_MEMSET AK_FBX_MEMSET
#define AK_FBX__STBI_MEMCPY AK_FBX_MEMCPY 

typedef ak_fbx_u8  ak_fbx__stbi_uc;
typedef ak_fbx_u16 ak_fbx__stbi__uint16;
typedef ak_fbx_u32 ak_fbx__stbi__uint32;

// fast-way is faster to check than jpeg huffman, but slow way is slower
#define AK_FBX__STBI__ZFAST_BITS  9 // accelerate all cases in default tables
#define AK_FBX__STBI__ZFAST_MASK  ((1 << AK_FBX__STBI__ZFAST_BITS) - 1)
#define AK_FBX__STBI__ZNSYMS 288 // number of symbols in literal/length alphabet

// zlib-style huffman encoding
// (jpegs packs from left, zlib from right, so can't share code)
typedef struct
{
   ak_fbx__stbi__uint16 fast[1 << AK_FBX__STBI__ZFAST_BITS];
   ak_fbx__stbi__uint16 firstcode[16];
   int maxcode[17];
   ak_fbx__stbi__uint16 firstsymbol[16];
   ak_fbx__stbi_uc  size[AK_FBX__STBI__ZNSYMS];
   ak_fbx__stbi__uint16 value[AK_FBX__STBI__ZNSYMS];
} ak_fbx__stbi__zhuffman;

static int ak_fbx__stbi__bitreverse16(int n)
{
  n = ((n & 0xAAAA) >>  1) | ((n & 0x5555) << 1);
  n = ((n & 0xCCCC) >>  2) | ((n & 0x3333) << 2);
  n = ((n & 0xF0F0) >>  4) | ((n & 0x0F0F) << 4);
  n = ((n & 0xFF00) >>  8) | ((n & 0x00FF) << 8);
  return n;
}

static int ak_fbx__stbi__bit_reverse(int v, int bits)
{
   AK_FBX__STBI_ASSERT(bits <= 16);
   // to bit reverse n bits, reverse 16 and shift
   // e.g. 11 bits, bit reverse and shift away 5
   return ak_fbx__stbi__bitreverse16(v) >> (16-bits);
}

static int ak_fbx__stbi__zbuild_huffman(ak_fbx__stbi__zhuffman *z, const ak_fbx__stbi_uc *sizelist, int num)
{
   int i,k=0;
   int code, next_code[16], sizes[17];

   // DEFLATE spec for generating codes
   AK_FBX__STBI_MEMSET(sizes, 0, sizeof(sizes));
   AK_FBX__STBI_MEMSET(z->fast, 0, sizeof(z->fast));
   for (i=0; i < num; ++i)
      ++sizes[sizelist[i]];
   sizes[0] = 0;
   for (i=1; i < 16; ++i)
      if (sizes[i] > (1 << i))
         return ak_fbx__stbi__err("bad sizes", "Corrupt ZLIB");
   code = 0;
   for (i=1; i < 16; ++i) {
      next_code[i] = code;
      z->firstcode[i] = (ak_fbx__stbi__uint16) code;
      z->firstsymbol[i] = (ak_fbx__stbi__uint16) k;
      code = (code + sizes[i]);
      if (sizes[i])
         if (code-1 >= (1 << i)) return ak_fbx__stbi__err("bad codelengths","Corrupt ZLIB");
      z->maxcode[i] = code << (16-i); // preshift for inner loop
      code <<= 1;
      k += sizes[i];
   }
   z->maxcode[16] = 0x10000; // sentinel
   for (i=0; i < num; ++i) {
      int s = sizelist[i];
      if (s) {
         int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
         ak_fbx__stbi__uint16 fastv = (ak_fbx__stbi__uint16) ((s << 9) | i);
         z->size [c] = (ak_fbx__stbi_uc     ) s;
         z->value[c] = (ak_fbx__stbi__uint16) i;
         if (s <= AK_FBX__STBI__ZFAST_BITS) {
            int j = ak_fbx__stbi__bit_reverse(next_code[s],s);
            while (j < (1 << AK_FBX__STBI__ZFAST_BITS)) {
               z->fast[j] = fastv;
               j += (1 << s);
            }
         }
         ++next_code[s];
      }
   }
   return 1;
}

// zlib-from-memory implementation for PNG reading
//    because PNG allows splitting the zlib stream arbitrarily,
//    and it's annoying structurally to have PNG call ZLIB call PNG,
//    we require PNG read all the IDATs and combine them into a single
//    memory buffer

typedef struct
{
   ak_fbx__stbi_uc *zbuffer, *zbuffer_end;
   int num_bits;
   ak_fbx__stbi__uint32 code_buffer;

   char *zout;
   char *zout_start;
   char *zout_end;
   int   z_expandable;

   ak_fbx__stbi__zhuffman z_length, z_distance;
} ak_fbx__stbi__zbuf;

static int ak_fbx__stbi__zeof(ak_fbx__stbi__zbuf *z)
{
   return (z->zbuffer >= z->zbuffer_end);
}

static ak_fbx__stbi_uc ak_fbx__stbi__zget8(ak_fbx__stbi__zbuf *z)
{
   return ak_fbx__stbi__zeof(z) ? 0 : *z->zbuffer++;
}

static void ak_fbx__stbi__fill_bits(ak_fbx__stbi__zbuf *z)
{
   do {
      if (z->code_buffer >= (1U << z->num_bits)) {
        z->zbuffer = z->zbuffer_end;  /* treat this as EOF so we fail. */
        return;
      }
      z->code_buffer |= (unsigned int) ak_fbx__stbi__zget8(z) << z->num_bits;
      z->num_bits += 8;
   } while (z->num_bits <= 24);
}

static unsigned int ak_fbx__stbi__zreceive(ak_fbx__stbi__zbuf *z, int n)
{
   unsigned int k;
   if (z->num_bits < n) ak_fbx__stbi__fill_bits(z);
   k = z->code_buffer & ((1 << n) - 1);
   z->code_buffer >>= n;
   z->num_bits -= n;
   return k;
}

static int ak_fbx__stbi__zhuffman_decode_slowpath(ak_fbx__stbi__zbuf *a, ak_fbx__stbi__zhuffman *z)
{
   int b,s,k;
   // not resolved by fast table, so compute it the slow way
   // use jpeg approach, which requires MSbits at top
   k = ak_fbx__stbi__bit_reverse(a->code_buffer, 16);
   for (s=AK_FBX__STBI__ZFAST_BITS+1; ; ++s)
      if (k < z->maxcode[s])
         break;
   if (s >= 16) return -1; // invalid code!
   // code size is s, so:
   b = (k >> (16-s)) - z->firstcode[s] + z->firstsymbol[s];
   if (b >= AK_FBX__STBI__ZNSYMS) return -1; // some data was corrupt somewhere!
   if (z->size[b] != s) return -1;  // was originally an assert, but report failure instead.
   a->code_buffer >>= s;
   a->num_bits -= s;
   return z->value[b];
}

static int ak_fbx__stbi__zhuffman_decode(ak_fbx__stbi__zbuf *a, ak_fbx__stbi__zhuffman *z)
{
   int b,s;
   if (a->num_bits < 16) {
      if (ak_fbx__stbi__zeof(a)) {
         return -1;   /* report error for unexpected end of data. */
      }
      ak_fbx__stbi__fill_bits(a);
   }
   b = z->fast[a->code_buffer & AK_FBX__STBI__ZFAST_MASK];
   if (b) {
      s = b >> 9;
      a->code_buffer >>= s;
      a->num_bits -= s;
      return b & 511;
   }
   return ak_fbx__stbi__zhuffman_decode_slowpath(a, z);
}

static const int ak_fbx__stbi__zlength_base[31] = {
   3,4,5,6,7,8,9,10,11,13,
   15,17,19,23,27,31,35,43,51,59,
   67,83,99,115,131,163,195,227,258,0,0 };

static const int ak_fbx__stbi__zlength_extra[31]=
{ 0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0,0,0 };

static const int ak_fbx__stbi__zdist_base[32] = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,
257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577,0,0};

static const int ak_fbx__stbi__zdist_extra[32] =
{ 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static int ak_fbx__stbi__parse_huffman_block(ak_fbx__stbi__zbuf *a)
{
   char *zout = a->zout;
   for(;;) {
      int z = ak_fbx__stbi__zhuffman_decode(a, &a->z_length);
      if (z < 256) {
         if (z < 0) return ak_fbx__stbi__err("bad huffman code","Corrupt PNG"); // error in huffman codes
         AK_FBX__STBI_ASSERT(zout < a->zout_end);
         *zout++ = (char) z;
      } else {
         ak_fbx__stbi_uc *p;
         int len,dist;
         if (z == 256) {
            a->zout = zout;
            return 1;
         }
         if (z >= 286) return ak_fbx__stbi__err("bad huffman code","Corrupt PNG"); // per DEFLATE, length codes 286 and 287 must not appear in compressed data
         z -= 257;
         len = ak_fbx__stbi__zlength_base[z];
         if (ak_fbx__stbi__zlength_extra[z]) len += ak_fbx__stbi__zreceive(a, ak_fbx__stbi__zlength_extra[z]);
         z = ak_fbx__stbi__zhuffman_decode(a, &a->z_distance);
         if (z < 0 || z >= 30) return ak_fbx__stbi__err("bad huffman code","Corrupt PNG"); // per DEFLATE, distance codes 30 and 31 must not appear in compressed data
         dist = ak_fbx__stbi__zdist_base[z];
         if (ak_fbx__stbi__zdist_extra[z]) dist += ak_fbx__stbi__zreceive(a, ak_fbx__stbi__zdist_extra[z]);
         if (zout - a->zout_start < dist) return ak_fbx__stbi__err("bad dist","Corrupt PNG");
         
         AK_FBX__STBI_ASSERT(zout + len <= a->zout_end);
         
         p = (ak_fbx__stbi_uc *) (zout - dist);
         if (dist == 1) { // run of one byte; common in images.
            ak_fbx__stbi_uc v = *p;
            if (len) { do *zout++ = v; while (--len); }
         } else {
            if (len) { do *zout++ = *p++; while (--len); }
         }
      }
   }
}

static int ak_fbx__stbi__compute_huffman_codes(ak_fbx__stbi__zbuf *a)
{
   static const ak_fbx__stbi_uc length_dezigzag[19] = { 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15 };
   ak_fbx__stbi__zhuffman z_codelength;
   ak_fbx__stbi_uc lencodes[286+32+137];//padding for maximum single op
   ak_fbx__stbi_uc codelength_sizes[19];
   int i,n;

   int hlit  = ak_fbx__stbi__zreceive(a,5) + 257;
   int hdist = ak_fbx__stbi__zreceive(a,5) + 1;
   int hclen = ak_fbx__stbi__zreceive(a,4) + 4;
   int ntot  = hlit + hdist;

   AK_FBX__STBI_MEMSET(codelength_sizes, 0, sizeof(codelength_sizes));
   for (i=0; i < hclen; ++i) {
      int s = ak_fbx__stbi__zreceive(a,3);
      codelength_sizes[length_dezigzag[i]] = (ak_fbx__stbi_uc) s;
   }
   if (!ak_fbx__stbi__zbuild_huffman(&z_codelength, codelength_sizes, 19)) return 0;

   n = 0;
   while (n < ntot) {
      int c = ak_fbx__stbi__zhuffman_decode(a, &z_codelength);
      if (c < 0 || c >= 19) return ak_fbx__stbi__err("bad codelengths", "Corrupt PNG");
      if (c < 16)
         lencodes[n++] = (ak_fbx__stbi_uc) c;
      else {
         ak_fbx__stbi_uc fill = 0;
         if (c == 16) {
            c = ak_fbx__stbi__zreceive(a,2)+3;
            if (n == 0) return ak_fbx__stbi__err("bad codelengths", "Corrupt PNG");
            fill = lencodes[n-1];
         } else if (c == 17) {
            c = ak_fbx__stbi__zreceive(a,3)+3;
         } else if (c == 18) {
            c = ak_fbx__stbi__zreceive(a,7)+11;
         } else {
            return ak_fbx__stbi__err("bad codelengths", "Corrupt PNG");
         }
         if (ntot - n < c) return ak_fbx__stbi__err("bad codelengths", "Corrupt PNG");
         AK_FBX__STBI_MEMSET(lencodes+n, fill, c);
         n += c;
      }
   }
   if (n != ntot) return ak_fbx__stbi__err("bad codelengths","Corrupt PNG");
   if (!ak_fbx__stbi__zbuild_huffman(&a->z_length, lencodes, hlit)) return 0;
   if (!ak_fbx__stbi__zbuild_huffman(&a->z_distance, lencodes+hlit, hdist)) return 0;
   return 1;
}

static int ak_fbx__stbi__parse_uncompressed_block(ak_fbx__stbi__zbuf *a)
{
   ak_fbx__stbi_uc header[4];
   int len,nlen,k;
   if (a->num_bits & 7)
      ak_fbx__stbi__zreceive(a, a->num_bits & 7); // discard
   // drain the bit-packed data into header
   k = 0;
   while (a->num_bits > 0) {
      header[k++] = (ak_fbx__stbi_uc) (a->code_buffer & 255); // suppress MSVC run-time check
      a->code_buffer >>= 8;
      a->num_bits -= 8;
   }
   if (a->num_bits < 0) return ak_fbx__stbi__err("zlib corrupt","Corrupt PNG");
   // now fill header the normal way
   while (k < 4)
      header[k++] = ak_fbx__stbi__zget8(a);
   len  = header[1] * 256 + header[0];
   nlen = header[3] * 256 + header[2];
   if (nlen != (len ^ 0xffff)) return ak_fbx__stbi__err("zlib corrupt","Corrupt PNG");
   if (a->zbuffer + len > a->zbuffer_end) return ak_fbx__stbi__err("read past buffer","Corrupt PNG");
   AK_FBX__STBI_ASSERT(a->zout + len <= a->zout_end);
   AK_FBX__STBI_MEMCPY(a->zout, a->zbuffer, len);
   a->zbuffer += len;
   a->zout += len;
   return 1;
}

static int ak_fbx__stbi__parse_zlib_header(ak_fbx__stbi__zbuf *a)
{
   int cmf   = ak_fbx__stbi__zget8(a);
   int cm    = cmf & 15;
   /* int cinfo = cmf >> 4; */
   int flg   = ak_fbx__stbi__zget8(a);
   if (ak_fbx__stbi__zeof(a)) return ak_fbx__stbi__err("bad zlib header","Corrupt PNG"); // zlib spec
   if ((cmf*256+flg) % 31 != 0) return ak_fbx__stbi__err("bad zlib header","Corrupt PNG"); // zlib spec
   if (flg & 32) return ak_fbx__stbi__err("no preset dict","Corrupt PNG"); // preset dictionary not allowed in png
   if (cm != 8) return ak_fbx__stbi__err("bad compression","Corrupt PNG"); // DEFLATE required for png
   // window = 1 << (8 + cinfo)... but who cares, we fully buffer output
   return 1;
}

static const ak_fbx__stbi_uc ak_fbx__stbi__zdefault_length[AK_FBX__STBI__ZNSYMS] =
{
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, 9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, 7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8
};
static const ak_fbx__stbi_uc ak_fbx__stbi__zdefault_distance[32] =
{
   5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};
/*
Init algorithm:
{
   int i;   // use <= to match clearly with spec
   for (i=0; i <= 143; ++i)     stbi__zdefault_length[i]   = 8;
   for (   ; i <= 255; ++i)     stbi__zdefault_length[i]   = 9;
   for (   ; i <= 279; ++i)     stbi__zdefault_length[i]   = 7;
   for (   ; i <= 287; ++i)     stbi__zdefault_length[i]   = 8;

   for (i=0; i <=  31; ++i)     stbi__zdefault_distance[i] = 5;
}
*/

static int ak_fbx__stbi__parse_zlib(ak_fbx__stbi__zbuf *a, int parse_header)
{
   int final, type;
   if (parse_header)
      if (!ak_fbx__stbi__parse_zlib_header(a)) return 0;
   a->num_bits = 0;
   a->code_buffer = 0;
   do {
      final = ak_fbx__stbi__zreceive(a,1);
      type = ak_fbx__stbi__zreceive(a,2);
      if (type == 0) {
         if (!ak_fbx__stbi__parse_uncompressed_block(a)) return 0;
      } else if (type == 3) {
         return 0;
      } else {
         if (type == 1) {
            // use fixed code lengths
            if (!ak_fbx__stbi__zbuild_huffman(&a->z_length  , ak_fbx__stbi__zdefault_length  , AK_FBX__STBI__ZNSYMS)) return 0;
            if (!ak_fbx__stbi__zbuild_huffman(&a->z_distance, ak_fbx__stbi__zdefault_distance,  32)) return 0;
         } else {
            if (!ak_fbx__stbi__compute_huffman_codes(a)) return 0;
         }
         if (!ak_fbx__stbi__parse_huffman_block(a)) return 0;
      }
   } while (!final);
   return 1;
}

static int ak_fbx__stbi__do_zlib(ak_fbx__stbi__zbuf *a, char *obuf, int olen, int exp, int parse_header)
{
   a->zout_start = obuf;
   a->zout       = obuf;
   a->zout_end   = obuf + olen;
   a->z_expandable = exp;

   return ak_fbx__stbi__parse_zlib(a, parse_header);
}

static int ak_fbx__stbi_zlib_decode_buffer(char *obuffer, int olen, char const *ibuffer, int ilen)
{
   ak_fbx__stbi__zbuf a;
   a.zbuffer = (ak_fbx__stbi_uc *) ibuffer;
   a.zbuffer_end = (ak_fbx__stbi_uc *) ibuffer + ilen;
   if (ak_fbx__stbi__do_zlib(&a, obuffer, olen, 0, 1))
      return (int) (a.zout - a.zout_start);
   else
      return -1;
}

#ifdef __cplusplus
}
#endif

#pragma warning(pop)

#endif //AK_FBX_IMPLEMENTATION