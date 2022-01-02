/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

#include "_meta.h"
#include "types.h"
#include "intrinsics.h"
#include "math_types.h"
#include "math_operators.h"
//#include "meta_types.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

inline b32x strings_equal(char *a, char *b)
{
    while(*a && *b && *a == *b)
    {
        a += 1;
        b += 1;
    }

    b32x result = *a == *b;
    return result;
}

inline b32x strings_equal(char *a, char *b, umm count)
{
    while(count && *a && *b && *a == *b)
    {
        count -= 1;
        a += 1;
        b += 1;
    }

    b32x result = *a == *b;
    return result;
}

#if 0
inline b32x array_bounds_check(void *array_start, void *array_end, void *access_start, void *access_end)
{
    Assert(array_start <= array_end);
    Assert(access_start <= access_end);
    
    b32x is_ok = access_start >= array_start && access_end < array_end;
    return is_ok;
}
inline b32x array_bounds_check(void *array_start, smm array_size, void *access_start, smm access_size)
{
    Assert(array_size >= 0);
    Assert(access_size >= 0);
    
    void *array_end = (u8 *)array_start + array_size;
    void *access_end = (u8 *)access_start + access_size;
    b32x is_ok = array_bounds_check(array_start, array_end, access_start, access_end);
    return is_ok;
}
inline b32x array_bounds_check(buffer array, buffer access)
{
    
}
#endif

#include <string.h>
inline void block_copy(void *destination, const void *source, umm size)
{
    memcpy(destination, source, size);
}

inline void block_copy_from_back(void *destination, const void *source, umm size)
{
    u8 *read = (u8 *)source + (size-1);
    u8 *write = (u8 *)destination + (size-1);

    for(umm index=0; index<size; ++index)
    {
        *write-- = *read--;
    }
}

internal s32
casey_crazy_function(char **AtInit)
{
    s32 Result = 0;
    
    char *At = *AtInit;
    while((*At >= '0') &&
          (*At <= '9'))
    {
        Result *= 10;
        Result += (*At - '0');
        ++At;
    }
    
    *AtInit = At;
    
    return(Result);
}

internal int
c_string_to_int(char *at)
{
    char *ignored = at;
    int result = casey_crazy_function(&ignored);
    return(result);
}

#include "logger.h"
#include "memory.h"
#include "my_string.h"
#include "my_string.cpp"
#include "value_parsing.h"
#include "my_math.h"
#include "color.h"
#include "ringbuffer.h"
#include "random.h"
#include "animation.h" // TODO(lubo): Is this crossing the point of which code should be sharable?

// NOTE(lubo): Not only does this sound pretty specific, it also is
// Only console history is using this, but I though it could be reused sometime in the future.
#include "string_ringbuffer.h"

#define SAFECAST(FROM, TO)                      \
    inline TO safecast_to_##TO(FROM value)      \
    {                                           \
        Assert(value >= TO##_min);              \
        Assert(value <= TO##_max);              \
        TO result = (TO)value;                  \
        return result;                          \
    };

SAFECAST(smm, int);
SAFECAST(int, u16);
SAFECAST(int, u8);

#define CLAMPCAST(FROM, TO)                             \
    inline TO clamp_to_##TO(FROM value)                 \
    {                                                   \
        TO result = (TO)value;                          \
        if(value < TO##_min) result = TO##_min;         \
        else if(value > TO##_max) result = TO##_max;    \
        return result;                                  \
    };

CLAMPCAST(float, u8);
CLAMPCAST(int, u8);

inline int simple_hash(string a)
{
    int result=0;
    for(smm index=0; index<a.size; ++index)
    {
        result = result*7 + a.memory[index]*11;
    }
    return result;
}

inline int simple_hash(char *a)
{
    int result = simple_hash(string_from_charstar(a));
    return result;
}

inline void clear_buffer(buffer A)
{
    memset(A.memory, 0, A.size);
}

#if 0
internal buffer read_entire_file_to_buffer(buffer write_to, string filename)
{
    buffer result = {};
    
    char *tmp = (char *)malloc(filename.size + 1);
    block_copy(tmp, filename.memory, filename.size + 1);
    tmp[filename.size] = 0;
    FILE *f = fopen(tmp, "rb");
    if(f)
    {
        fseek(f, 0, SEEK_END);
        smm file_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        Assert(write_to.size >= file_size);

        fread(write_to.memory, 1, file_size, f);
        fclose(f);

        result.memory = write_to.memory;
        result.size = file_size;

        if(write_to.size >= file_size + 1)
        {
            buf.memory[buf.size] = 0;
        }
    }
    free(tmp);

    return result;
}
#endif

internal buffer read_entire_file(Memory_Arena *arena, string filename)
{
    buffer buf = {};
    char *tmp = (char *)malloc(filename.size + 1);
    block_copy(tmp, filename.memory, filename.size + 1);
    tmp[filename.size] = 0;
    FILE *f = fopen(tmp, "rb");
    if(f)
    {
        fseek(f, 0, SEEK_END);
        buf.size = ftell(f);
        fseek(f, 0, SEEK_SET);
        buf.memory = (u8 *)push_array(arena, u8, buf.size + 1);
        fread(buf.memory, 1, buf.size, f);
        fclose(f);
        buf.memory[buf.size] = 0;
    }
    free(tmp);
    return buf;
}

internal buffer read_entire_file(string filename)
{
    buffer buf = {};
    char *tmp = (char *)malloc(filename.size + 1);
    block_copy(tmp, filename.memory, filename.size + 1);
    tmp[filename.size] = 0;
    FILE *f = fopen(tmp, "rb");
    if(f)
    {
        fseek(f, 0, SEEK_END);
        buf.size = ftell(f);
        fseek(f, 0, SEEK_SET);
        buf.memory = (u8 *)malloc(buf.size + 1); // TODO(lubo): Is there a reason why we don't use general realloc? It's not dependence since we're totally depending on CRT for this function.
        fread(buf.memory, 1, buf.size, f);
        fclose(f);
        buf.memory[buf.size] = 0;
    }
    free(tmp);
    return buf;
}

internal void *
open_file_for_writing(string filename)
{
    char *tmp = (char *)malloc(filename.size + 1);
    block_copy(tmp, filename.memory, filename.size + 1);
    tmp[filename.size] = 0;
    FILE *output_file = fopen(tmp, "wb");
    free(tmp);
    return output_file;
}

inline void write_to_file(void *output_file, buffer data)
{
    fwrite(data.memory, 1, data.size, (FILE *)output_file);
}

inline void write_string_to_file(void *output_file, string a)
{
    write_to_file(output_file, (buffer)a);
}

internal void
close_file_for_writing(void *output_file)
{
    fclose((FILE *)output_file);
}

// NOTE(lubo): Dynamic list types
#define DL_TYPE int
#define DL_COMPARISON
#include "dynamic_list_template.h"
#define DL_TYPE v2
#define DL_EQUALITY
#include "dynamic_list_template.h"
#define DL_TYPE v3
#define DL_EQUALITY
#include "dynamic_list_template.h"
#define DL_TYPE buffer
#include "dynamic_list_template.h"
#define DL_TYPE string
#define DL_COMPARISON
#include "dynamic_list_template.h"

// TODO in "dynamic_list_template.h": This should create functions like insert_ordered, is_in_ordererd
#define DL_TYPE u64
#define DL_COMPARISON
#include "dynamic_list_template.h"
