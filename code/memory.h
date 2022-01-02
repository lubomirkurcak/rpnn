/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

#include <malloc.h>

// TODO(lubomir): This is not multithread safe (add volatile modifier and use atomic functions)
global smm global_allocations_size;
global smm global_allocations_count;
global smm global_allocations_live;
global smm global_allocations_failed;
global smm global_allocations_resizes;
global smm global_allocations_null_frees;

#define general_allocate_struct(Type) general_allocate(sizeof(Type))
#define general_allocate_array(Type, Count) general_allocate(sizeof(Type)*(Count))
#define general_reallocate_array(Buffer, Type, Count) general_reallocate(Buffer, sizeof(Type)*(Count))

internal buffer general_allocate(smm size)
{
    buffer result = {};
    if(size <= 0)
    {
        loginfo("allocate", "Invalid allocation size: %zd", size);
        return result;
    }
    
    void *memory = malloc(size);

    if(memory)
    {
        global_allocations_count += 1;
        global_allocations_live += 1;
        global_allocations_size += size;

        result.size = size;
        result.memory = (u8 *)memory;
    }
    else
    {
        global_allocations_failed += 1;
        loginfo("allocate", "Allocation of size %zd bytes failed", size);
    }
    
    return result;
}

inline void *simple_alloc(smm size)
{
    buffer alloc = general_allocate(size);
    return alloc.memory;
}

internal void general_deallocate(buffer buf)
{
    if(buf.memory)
    {
        global_allocations_size -= buf.size;
        global_allocations_live -= 1;
        free(buf.memory);
    }
    else
    {
        global_allocations_null_frees += 1;
        loginfo("deallocate", "Someone tried to free null pointer! Supposed size was %zd", buf.size);
        Assert(false);
    }
}

buffer general_reallocate(buffer buf, smm new_size)
{
    buffer result = {};

    global_allocations_resizes += 1;
    global_allocations_size -= buf.size;
    
    void *memory = realloc(buf.memory, new_size);

    if(memory)
    {
        result.memory = (u8 *)memory;
        result.size = new_size;

        global_allocations_size += new_size;
    }
    else
    {
        global_allocations_live -= 1;
        global_allocations_failed += 1;
        loginfo("reallocate", "Reallocation from %zd bytes to %zd bytes failed", buf.size, new_size);
        Assert(false);
    }

    return result;
}


struct Memory_Block
{
    buffer memory;
    smm used;
    Memory_Block *previous_block;
};

struct Memory_Arena
{
    Memory_Block *current_block;
    smm block_size;
    int temp_count;
};

inline int count_blocks(Memory_Arena *arena)
{
    Memory_Block *block = arena->current_block;
    int result = 0;
    while(block)
    {
        result += 1;
        block = block->previous_block;
    }
    return result;
}

inline void remove_last_block(Memory_Arena *arena)
{
    Memory_Block *last = arena->current_block;
    arena->current_block = last->previous_block;
    general_deallocate(last->memory);
}

inline Memory_Arena create_memory_arena()
{
    Memory_Arena arena = {};
    arena.block_size = Megabytes(64);
    
    return arena;
}

inline void *find_next_aligned(void *memory, smm alignment)
{
    Assert(alignment <= 128);
    Assert(is_pow2(alignment));

    smm address = (smm)memory;
    smm mask = ~(alignment-1);
    void *result = (void *)((address + alignment-1) & mask);
    return result;
}

inline umm get_alignment_offset(void *memory, umm alignment)
{
    void *aligned = find_next_aligned(memory, alignment);
    umm offset = PointerDifference(memory, aligned);
    return offset;
}

#define push_struct(Arena, Type, ...) (Type *) _push_size(Arena, sizeof(Type), ## __VA_ARGS__)
#define push_array(Arena, Type, Count, ...)  (Type *) _push_size(Arena, (Count)*sizeof(Type), ## __VA_ARGS__)
internal void *
_push_size(Memory_Arena *arena, umm size, umm alignment=16)
{
    Assert(alignment <= 128);
    Assert(is_pow2(alignment));

    b32x need_to_alloc_block = false;
    if(arena->current_block)
    {
        Memory_Block *block = arena->current_block;
        smm alignment_offset = get_alignment_offset(block->memory.memory + block->used, alignment);
        smm actual_size = size + alignment_offset;
        if(block->used + actual_size > block->memory.size)
        {
            need_to_alloc_block = true;
        }
    }
    else
    {
        need_to_alloc_block = true;
    }

    if(need_to_alloc_block)
    {
        buffer buf = general_allocate(arena->block_size);
        Memory_Block *block = (Memory_Block *)buf.memory;
        
        Assert(sizeof(*block) < arena->block_size);
        
        block->memory = buf;
        block->used = sizeof(*block);
        block->previous_block = arena->current_block;
        arena->current_block = block;
    }

    Assert(arena->current_block);

    Memory_Block *block = arena->current_block;
    smm alignment_offset = get_alignment_offset(block->memory.memory + block->used, alignment);
    smm actual_size = size + alignment_offset;
    
    Assert(block->used + actual_size <= block->memory.size);

    void *result = block->memory.memory + block->used + alignment_offset;
    block->used += actual_size;
    
    return result;
}

internal void
clear_arena(Memory_Arena *arena, b32x leave_one_block=true)
{
    Memory_Block *block = arena->current_block;
    arena->current_block = 0;
    int cleared_blocks = 0;
    while(block)
    {
        Memory_Block *previous = block->previous_block;
        if(leave_one_block && previous == 0)
        {
            block->used = sizeof(*block);
            arena->current_block = block;
            break;
        }
        
        general_deallocate(block->memory);

        block = previous;
        cleared_blocks += 1;
    }
    
    loginfo("clear_arena", "Cleared %d memory blocks", cleared_blocks);
}

struct Temporary_Memory
{
    Memory_Arena *arena;
    Memory_Block *block;
    smm used;
};

inline Temporary_Memory
begin_temporary_memory(Memory_Arena *arena)
{
    Temporary_Memory result;
    
    result.arena = arena;
    result.block = arena->current_block;
    result.used = arena->current_block ? arena->current_block->used : 0;
    
    arena->temp_count += 1;
    
    return result;
}

inline void
end_temporary_memory(Temporary_Memory temp_mem)
{
    Memory_Arena *arena = temp_mem.arena;
    while(arena->current_block != temp_mem.block)
    {
        remove_last_block(arena);
    }
    
    if(arena->current_block)
    {
        Assert(arena->current_block->used >= temp_mem.used);
        arena->current_block->used = temp_mem.used;
    }
    
    Assert(arena->temp_count > 0);
    arena->temp_count -= 1;
}
