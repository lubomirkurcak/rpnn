/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

// NOTE(lubo): Could be humourously called 'Stringbuffer'
struct String_Ringbuffer
{
    Ringbuffer ringbuffer;
    smm item_count;
};

internal void clear(String_Ringbuffer *sring)
{
    clear(&sring->ringbuffer);
    sring->item_count = 0;
}

internal void init_string_ringbuffer(String_Ringbuffer *sring, buffer buf)
{
    ringbuffer_init(&sring->ringbuffer, buf);

    sring->item_count = 0;
}

internal void remove_first(String_Ringbuffer *sring)
{
    string first;
    buffer struct_buffer;
    struct_buffer.memory = (u8 *)&first;
    struct_buffer.size = sizeof(first);
    read_to_buffer(&sring->ringbuffer, 0, struct_buffer);

    smm total_size_to_free = sizeof(first) + first.size;
    deallocate(&sring->ringbuffer, total_size_to_free);

    sring->item_count -= 1;
}

internal b32x insert_string(String_Ringbuffer *sring, string a)
{
    b32x success = false;
    smm total_needed_allocation_size = a.size + sizeof(a);
    if(total_needed_allocation_size <= sring->ringbuffer.buffer.size)
    {
        while(total_needed_allocation_size > sring->ringbuffer.free_size)
        {
            remove_first(sring);
        }

        buffer struct_buffer;
        struct_buffer.memory = (u8 *)&a;
        struct_buffer.size = sizeof(a);

        buffer contents_buffer = (buffer)a;
    
        store(&sring->ringbuffer, struct_buffer);
        store(&sring->ringbuffer, contents_buffer);

        sring->item_count += 1;
        success = true;
    }
    
    return success;
}

internal smm get_string_at_offset(String_Ringbuffer *sring, smm offset, string *first_part, string *other_part)
{
    string first;
    buffer struct_buffer;
    struct_buffer.memory = (u8 *)&first;
    struct_buffer.size = sizeof(first);
    read_to_buffer(&sring->ringbuffer, offset, struct_buffer);
    offset += struct_buffer.size;

    buffer region1 = {};
    buffer region2 = {};
    read_in_place(&sring->ringbuffer, offset, first.size, &region1, &region2);
    string part1 = (string)region1;
    string part2 = (string)region2;
    offset += region1.size + region2.size;

    *first_part = region1;
    *other_part = region2;
    
    return offset;
}
