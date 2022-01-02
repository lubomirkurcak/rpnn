/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
// TODO(casey): Moar compilerz!!!
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#elif COMPILER_LLVM
#include <x86intrin.h>
#else
#error SEE/NEON optimizations are not available for this compiler yet!!!!
#endif

////////////////

#include <stdint.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>

#define internal static
#define global static
#define local_persist static

Serializable; typedef int8_t s8;
Serializable; typedef int16_t s16;
Serializable; typedef int32_t s32;
Serializable; typedef int64_t s64;
Serializable; typedef s32 b32;
Serializable; typedef b32 b32x;

Serializable; typedef uint8_t u8;
Serializable; typedef uint16_t u16;
Serializable; typedef uint32_t u32;
Serializable; typedef uint64_t u64;
Serializable; typedef u32 u32x;

Serializable; typedef double float64;

Serializable; typedef uintptr_t umm;
Serializable; typedef intptr_t smm;

enum {LITTLE_ENDIAN = 0x03020100ul, BIG_ENDIAN = 0x00010203};
global union {u8 bytes[4]; u32 value;} global_endianness_check = {{0,1,2,3}};
#define LOCAL_ENDIANNESS (global_endianness_check.value)

#define true 1
#define false 0
#define s8_min  (-0x80)
#define s16_min (-0x8000)
#define s32_min (-0x80000000LL)
#define s64_min (-0x8000000000000000LL)
#define s8_max  (0x7f)
#define s16_max (0x7fff)
#define s32_max (0x7fffffffLL)
#define s64_max (0x7fffffffffffffffLL)
#define u8_min  (0)
#define u16_min (0)
#define u32_min (0)
#define u64_min (0)
#define u8_max  (0xff)
#define u16_max (0xffff)
#define u32_max (0xffffffffLL)
#define u64_max (0xffffffffffffffffLL)
#define int_max INT_MAX
#define int_min INT_MIN
#define float_max FLT_MAX
#define float_tiny FLT_MIN
#define float64_max DBL_MAX
#define float64_min DBL_MIN

#define flag8(type) u8
#define flag16(type) u16
#define flag32(type) u32
#define flag64(type) u64
#define enum8(type) u8
#define enum16(type) u16
#define enum32(type) u32
#define enum64(type) u64

#define STRINGIZE2(A) #A
#define STRINGIZE(A) STRINGIZE2(A)
#define CONCAT(A,B) A##B
#define PointerDifference(A,B) (((u8 *)B)-((u8 *)A))
//#define ArrayOffset(Array, Pointer) ((PointerDifference(Array,Pointer))/sizeof(Array[0]))
#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))
#define InRangeOfArray(Array, Pointer) ((PointerDifference(Array,Pointer)>=0)&&(PointerDifference(Array,Pointer)<sizeof(Array)))
#define Swap(A,B) {auto _SWAP_VAR = (A); (A) = (B); (B) = _SWAP_VAR;}

#define Kilobytes(Value) (1024LL*(Value))
#define Megabytes(Value) (1024LL*Kilobytes(Value))
#define Gigabytes(Value) (1024LL*Megabytes(Value))
#define Terabytes(Value) (1024LL*Gigabytes(Value))

#define GetBit(Value, Bit) ((Value)&(1<<(Bit)))
#define SetBit(Value, Bit) ((Value)|(1<<(Bit)))
#define ClearBit(Value, Bit) ((Value)&(~(1<<(Bit))))

#define AlignPow2(Value, Alignment) ((Value + ((Alignment) - 1)) & ~((Value - Value) + (Alignment) - 1))
inline b32x is_pow2(umm value)
{
    b32x result = (value & ~(value - 1)) == value;
    return result;
}


//#define loop_forever for(;;)
#define loop_times(A) for(int item_index=0; item_index<(A); ++item_index)

// NOTE(lubo): Inspired from macros.h from google. Thanks!
#define foreach_static(i, A) for(int ___KEEP=1, item_index=0, ___SIZE=ArrayCount(A); \
                                 ___KEEP && item_index != ___SIZE;      \
                                 ___KEEP = !___KEEP, ++item_index)      \
        for(i = (A)+item_index; ___KEEP; ___KEEP = !___KEEP)

#define Min(A,B) (((A)<(B))?(A):(B))
#define Max(A,B) (((A)>(B))?(A):(B))
#define Abs(A) (((A)<0)?(-(A)):(A))
#define Clamp(Value,A,B) Min(Max(Value,A),B)
//#define Clamp01(Value) Clamp(Value,0,1)
#define RoundUp(A,B) (((A)+(B)-1)/(B))

#define RGBA(R,G,B,A) (u32)(            \
    (u32)(255.0f*((float)(R))) <<  0 |  \
    (u32)(255.0f*((float)(G))) <<  8 |  \
    (u32)(255.0f*((float)(B))) << 16 |  \
    (u32)(255.0f*((float)(A))) << 24)

#define RGB_HEX(A) (u32)(                  \
    (u32)(16.0f*((float)hex_to_int((A)[1])) + ((float)hex_to_int((A)[2]))) <<  0 | \
    (u32)(16.0f*((float)hex_to_int((A)[3])) + ((float)hex_to_int((A)[4]))) <<  8 | \
    (u32)(16.0f*((float)hex_to_int((A)[5])) + ((float)hex_to_int((A)[6]))) << 16)

struct Loaded_Bitmap
{
    void *data;
    int width;
    int height;
    int channels;
};

Serializable;
struct buffer
{
    smm size;
    u8 *memory;

    // NOTE(lubo): I think this is the only non-pure-C thing in this file
    DontSerialize; inline u8& operator[](smm index);
    DontSerialize; inline buffer& operator+=(const buffer& b);
};
#define StaticBuffer(A) {ArrayCount(A),A}

struct Buffer_Pair
{
    buffer first;
    buffer second;
};

inline Buffer_Pair split_buffer(buffer buffer, smm size_of_first)
{
//    Assert(size_of_first > 0);
//    Assert(size_of_first <= buffer.size);
    
    Buffer_Pair result = {};
    result.first.memory = buffer.memory;
    result.first.size = size_of_first;
    result.second.memory = buffer.memory + size_of_first;
    result.second.size = buffer.size - size_of_first;
    return result;
}

Serializable; typedef buffer string;
#define String(A) string_from_charstar(A)
#define EmptyString {0,0}
inline smm string_length(char *a)
{
    smm result = 0;
    while(*a++) result += 1;
    return result;
}
inline string string_from_charstar(char *a)
{
    string result = {};
    if(a)
    {
        result.size = string_length(a);
        result.memory = (u8 *)a;
    }
    return result;
}

inline int hex_to_int(char c)
{
    int result = 0;
    if(c >= 'a' && c <= 'z')
    {
        result = c - 'a' + 10;
    }
    else if(c >= 'A' && c <= 'Z')
    {
        result = c - 'A' + 10;
    }
    else if(c >= '0' && c <= '9')
    {
        result = c - '0';
    }
    
    return result;
}
