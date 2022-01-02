/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

struct Ringbuffer
{
    buffer buffer;
    smm read;
    smm write;
    smm free_size;

    smm granularity;
};

/*inline smm get_free_size(Ringbuffer *ring)
{
    smm free = ring->read - ring->write;
    if(free < 0) free += ring->buffer.size
    return free;
}*/

internal void clear(Ringbuffer *ring)
{
    ring->read = 0;
    ring->write = 0;
    ring->free_size = ring->buffer.size;
}

internal void ringbuffer_init(Ringbuffer *ring, buffer buf)
{
    ring->buffer = buf;

    clear(ring);

    // TODO(lubo): Align memory to number of bytes
    // Currently 1 is baked.
    ring->granularity = 1;
}

internal b32x allocate(Ringbuffer *ring, smm size, buffer *first_region, buffer *second_region)
{
    b32x success = false;
    buffer region1 = {};
    buffer region2 = {};

    if(size <= ring->free_size)
    {
        success = true;
        ring->free_size -= size;
        
        if(ring->write >= ring->read)
        {
            smm free_until_end = ring->buffer.size - ring->write;
            if(size <= free_until_end)
            {
                region1.memory = ring->buffer.memory + ring->write;
                region1.size = size;
            
                ring->write += size;
            }
            else
            {
                smm free_at_start = ring->read;
                region1.memory = ring->buffer.memory + ring->write;
                region1.size = free_until_end;

                smm remaining_size = size - free_until_end;
                region2.memory = ring->buffer.memory;
                region2.size = remaining_size;
            
                ring->write = remaining_size;
            }

        }
        else
        {
            smm free_total = ring->read - ring->write;
            region1.memory = ring->buffer.memory + ring->write;
            region1.size = size;
            ring->write += size;
        }
    }

    *first_region = region1;
    *second_region = region2;
    return success;
}

internal b32x store(Ringbuffer *ring, buffer data)
{
    b32x success = false;

    buffer region1 = {};
    buffer region2 = {};
    
    if(allocate(ring, data.size, &region1, &region2))
    {
        block_copy(region1.memory, data.memory,                region1.size);
        block_copy(region2.memory, data.memory + region1.size, region2.size);
    }

    return success;
}

internal void deallocate(Ringbuffer *ring, smm size)
{
    Assert(ring->free_size + size <= ring->buffer.size);
    ring->free_size += size;
    
    if(ring->write > ring->read)
    {
        smm allocated_total = ring->write - ring->read;
        ring->read += size;
    }
    else
    {
        smm allocated_until_end = ring->buffer.size - ring->read;
        if(size <= allocated_until_end)
        {
            ring->read += size;
        }
        else
        {
            smm allocated_at_start = ring->write;
            smm remaining_size = size - allocated_until_end;
            ring->read = remaining_size;
        }
    }
}

internal b32x read_in_place(Ringbuffer *ring, smm offset, smm size, buffer *first_region, buffer *second_region)
{
    b32x success = false;
    buffer region1 = {};
    buffer region2 = {};
    
    if(size <= ring->buffer.size)
    {
        success = true;

        smm effective_read = ring->read + offset;
        if(effective_read >= ring->buffer.size) effective_read -= ring->buffer.size;
        
        if(ring->write > effective_read)
        {
            smm allocated_total_after_offset = ring->write - effective_read;
            region1.memory = ring->buffer.memory + effective_read;
            region1.size = size;
        }
        else
        {
            smm allocated_until_end = ring->buffer.size - effective_read;
            if(size <= allocated_until_end)
            {
                region1.memory = ring->buffer.memory + effective_read;
                region1.size = size;
            }
            else
            {
                smm allocated_at_start = ring->write;
                region1.memory = ring->buffer.memory + effective_read;
                region1.size = allocated_until_end;

                smm remaining_size = size - allocated_until_end;
                region2.memory = ring->buffer.memory;
                region2.size = remaining_size;
            }
        }
    }

    *first_region = region1;
    *second_region = region2;
    return success;
}

internal b32x read_to_buffer(Ringbuffer *ring, smm offset, buffer buf)
{
    b32x success = false;

    buffer region1 = {};
    buffer region2 = {};
    
    if(read_in_place(ring, offset, buf.size, &region1, &region2))
    {
        block_copy(buf.memory,                region1.memory, region1.size);
        block_copy(buf.memory + region1.size, region2.memory, region2.size);
    }

    return success;
}
