/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2020 All Rights Reserved. $
   ======================================================================== */

// NOTE(lubo): Renamed from IDX_File because there was a collision with python C interface.
struct LK_IDX_File
{
    int idx_data_type;
    int type_size;
    int dimensions;
    int *raw_big_endian_dimension_sizes;
    void *data;

    int element_size;
    int elements;

    int dimension_sizes[8];
};

inline void *idx_get_element(LK_IDX_File file, int element_index)
{
    void *result = (u8 *)file.data + file.element_size*element_index;
    return result;
}

internal LK_IDX_File process_idx_file(buffer contents)
{
    LK_IDX_File result = {};
    u8 *at = contents.memory;

    Assert(at[0] == 0);
    Assert(at[1] == 0);

    char *data_type_name;
    
    result.idx_data_type = at[2];
    switch(result.idx_data_type)
    {
        default: result.type_size = 0; data_type_name = "unknown"; break;
        case 0x08: result.type_size = 1; data_type_name = "u8"; break;
        case 0x09: result.type_size = 1; data_type_name = "s8"; break;
        case 0x0B: result.type_size = 2; data_type_name = "s16"; break;
        case 0x0C: result.type_size = 4; data_type_name = "s32"; break;
        case 0x0D: result.type_size = 4; data_type_name = "float"; break;
        case 0x0E: result.type_size = 8; data_type_name = "float64"; break;
    }

    result.dimensions = at[3];
    Assert(result.dimensions <= ArrayCount(result.dimension_sizes));

    result.raw_big_endian_dimension_sizes = (int *)(at + 4);
    result.data = result.raw_big_endian_dimension_sizes + result.dimensions;

    for(int dim=0; dim<result.dimensions; ++dim)
    {
        result.dimension_sizes[dim] = result.raw_big_endian_dimension_sizes[dim];
        if(LOCAL_ENDIANNESS == LITTLE_ENDIAN)
        {
            result.dimension_sizes[dim] = change_endianness(result.dimension_sizes[dim]);
        }
    }
    
    result.element_size = result.type_size;
    for(int dim=1; dim<result.dimensions; ++dim)
    {
        result.element_size *= result.dimension_sizes[dim];
    }
    
    size_t data_size = PointerDifference(result.data, (u8 *)contents.memory + contents.size);
    result.elements = (int)(data_size/result.element_size);
    Assert(result.elements * result.element_size == data_size);
    Assert(result.elements == result.dimension_sizes[0]);

    return result;
}
