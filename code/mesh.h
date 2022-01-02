/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

struct Mesh
{
    list(v3) p;
    list(v2) uv;
    list(v3) n;

    list(int) indices;

    b32x dynamic;
    b32x dont_use_uvs;
};

r3 bounding_box(Mesh *mesh)
{
    v3 point = get_by_index(&mesh->p, 0);

    r3 result = {point, point};
        
    foreach_by_value_starting_index(int index, mesh->indices, 1)
    {
        v3 point = get_by_index(&mesh->p, index);
        result.min.x = Min(result.min.x, point.x);
        result.min.y = Min(result.min.y, point.y);
        result.min.z = Min(result.min.z, point.z);
        result.max.x = Max(result.max.x, point.x);
        result.max.y = Max(result.max.y, point.y);
        result.max.z = Max(result.max.z, point.z);
    }

    return result;
}

// NOTE(lubo): full circle
internal Mesh create_disc(int slices=32)
{
    Mesh mesh = {};

    append_to_list(&mesh.p, V3(0));
    append_to_list(&mesh.uv, V2(0.5f));
    append_to_list(&mesh.n, V3(0,0,1));

    v2 currently_at = V2(1,0);
    v2 rotate_by = unit_angle(Tau32/slices);
    
    for(int i=0; i<slices; ++i)
    {
        append_to_list(&mesh.p, V3(currently_at, 0));
        append_to_list(&mesh.uv, to_range_01(currently_at));
        append_to_list(&mesh.n, V3(0,0,1));

        append_to_list(&mesh.indices, 0);
        append_to_list(&mesh.indices, 1 + i);
        append_to_list(&mesh.indices, 1 + (i+1)%slices);
        
        currently_at = complex_multiply(currently_at, rotate_by);
    }

    return mesh;
}

// NOTE(lubo): hollow circle, inner radius must be from (0, 1)
internal Mesh create_annulus(float inner_radius=0.8f, int slices=32)
{
    Mesh mesh = {};

    v2 currently_at = V2(1,0);
    v2 rotate_by = unit_angle(Tau32/slices);
    
    for(int i=0; i<slices; ++i)
    {
        v2 inner_at = inner_radius*currently_at;
        append_to_list(&mesh.p, V3(inner_at, 0));
        append_to_list(&mesh.uv, to_range_01(inner_at));
        append_to_list(&mesh.n, V3(0,0,1));

        append_to_list(&mesh.p, V3(currently_at, 0));
        append_to_list(&mesh.uv, to_range_01(currently_at));
        append_to_list(&mesh.n, V3(0,0,1));

        append_to_list(&mesh.indices, 2*i);
        append_to_list(&mesh.indices, 2*i + 1);
        append_to_list(&mesh.indices, (2*i + 3)%(2*slices));
        
        append_to_list(&mesh.indices, 2*i);
        append_to_list(&mesh.indices, (2*i + 3)%(2*slices));
        append_to_list(&mesh.indices, (2*i + 2)%(2*slices));
        
        currently_at = complex_multiply(currently_at, rotate_by);
    }

    return mesh;
}

internal Mesh create_plane(int width, int height)
{
    list(v3) p_list = {};
    list(v2) uv_list = {};
    list(v3) n_list = {};
    list(int) vertex_indices = {};

    // Create vertices
    for(int y=0; y<height + 1; ++y)
    {
        for(int x=0; x<width + 1; ++x)
        {
            append_to_list(&p_list, V3((float)x, (float)y, 0));
            append_to_list(&uv_list, V2(ratio(x, width), ratio(y, height)));
            append_to_list(&n_list, V3(0, 0, 1));
        }
    }

    // Create triangles
    for(int y=0; y<height; ++y)
    {
        for(int x=0; x<width; ++x)
        {
            append_to_list(&vertex_indices, (y)*(width+1) + (x));
            append_to_list(&vertex_indices, (y)*(width+1) + (x+1));
            append_to_list(&vertex_indices, (y+1)*(width+1) + (x+1));

            append_to_list(&vertex_indices, (y)*(width+1) + (x));
            append_to_list(&vertex_indices, (y+1)*(width+1) + (x+1));
            append_to_list(&vertex_indices, (y+1)*(width+1) + (x));
        }
    }

    Mesh mesh = {};
    mesh.p = p_list;
    mesh.uv = uv_list;
    mesh.n = n_list;
    mesh.indices = vertex_indices;

    return mesh;
}

internal void create_tile_plane(Mesh *mesh, int width, int height, int tilemap_width, int tilemap_height, int *tiles, float padding=0)
{
    list(v3) *p_list = &mesh->p;
    list(v2) *uv_list = &mesh->uv;
    list(v3) *n_list = &mesh->n;
    list(int) *vertex_indices = &mesh->indices;

    clear_list(p_list);
    clear_list(uv_list);
    clear_list(n_list);
    clear_list(vertex_indices);

    // Create quads
    for(int y=0; y<height; ++y)
    {
        for(int x=0; x<width; ++x)
        {
            int tile = tiles[y*width + x];
            r2 uvs = rect_atlas(tilemap_width, tilemap_height, tile, padding);

            int a = append_to_list(p_list, V3((float)x, (float)y, 0));
            append_to_list(uv_list, get_bottom_left(uvs));
            append_to_list(n_list, V3(0, 0, 1));

            int b = append_to_list(p_list, V3((float)(x+1), (float)y, 0));
            append_to_list(uv_list, get_bottom_right(uvs));
            append_to_list(n_list, V3(0, 0, 1));

            int c = append_to_list(p_list, V3((float)(x+1), (float)(y+1), 0));
            append_to_list(uv_list, get_top_right(uvs));
            append_to_list(n_list, V3(0, 0, 1));

            int d = append_to_list(p_list, V3((float)x, (float)(y+1), 0));
            append_to_list(uv_list, get_top_left(uvs));
            append_to_list(n_list, V3(0, 0, 1));

            
            append_to_list(vertex_indices, a);
            append_to_list(vertex_indices, b);
            append_to_list(vertex_indices, c);

            append_to_list(vertex_indices, a);
            append_to_list(vertex_indices, c);
            append_to_list(vertex_indices, d);
        }
    }
}

internal Mesh load_obj(string contents, quaternion rotation={0,0,0,1})
{
    string remaining = skip_whitespace(contents);

    list(v3) p_list_pure = {};
    list(v2) uv_list_pure = {};
    list(v3) n_list_pure = {};

    int unique_vertex_count = 0;
    list(u64, "compare") unique_p_uv_n_combinations = {};

    // NOTE(lubo): Out params
    list(v3) p_list = {};
    list(v2) uv_list = {};
    list(v3) n_list = {};
    list(int) vertex_indices = {};
    
    int line_number = 0;
    while(remaining.size)
    {
        ++line_number;
        string_split split = split_at_end_of_line(remaining);
        remaining = skip_whitespace(split.remaining);
        string line = split.first;
        line = skip_whitespace_from_back(split_at_char(line, '#').first);

        if(line.size == 0) continue;

        split = split_at_whitespace(line);
        string key = split.first;
        string value = skip_whitespace(split.remaining);
        
        if(key == String("mtllib"))
        {
        }
        else if(key == String("usemtl"))
        {
        }
        else if(key == String("o"))
        {
        }
        else if(key == String("v"))
        {
            v3 p = {};
            Any any = {TypeOf(v3), &p};
            if(!parse_value(any, value))
            {
                logprint(".obj loader", "Failed to load something in .obj at line %d. Error detected ", line_number);
            }

            append_to_list(&p_list_pure, rotation*p);
        }
        else if(key == String("vt"))
        {
            v2 uv = {};
            Any any = {TypeOf(v2), &uv};
            if(!parse_value(any, value))
            {
                logprint(".obj loader", "Failed to load something in .obj at line %d. Error detected ", line_number);
            }

            append_to_list(&uv_list_pure, uv);
        }
        else if(key == String("vn"))
        {
            v3 n = {};
            Any any = {TypeOf(v3), &n};
            if(!parse_value(any, value))
            {
                logprint(".obj loader", "Failed to load something in .obj at line %d. Error detected ", line_number);
            }

            append_to_list(&n_list_pure, rotation*n);
        }
        else if(key == String("s"))
        {
            // smooth shading
        }
        else if(key == String("f"))
        {
            string_split split = split_at_whitespace(value);
            
            string vertex_0 = split.first;
            split = split_at_whitespace(skip_whitespace(split.remaining));
            string vertex_1 = split.first;
            string vertex_2 = skip_whitespace(split.remaining);

            int v0_p;
            int v0_uv;
            int v0_n;
            int v1_p;
            int v1_uv;
            int v1_n;
            int v2_p;
            int v2_uv;
            int v2_n;

            {
                split = split_at_char(vertex_0, '/');
                string p = split.first;
                split = split_at_char(substring(split.remaining, 1), '/');
                string uv = split.first;
                string n = substring(split.remaining, 1);

                Any p_any = {TypeOf(int), &v0_p};
                Any uv_any = {TypeOf(int), &v0_uv};
                Any n_any = {TypeOf(int), &v0_n};
                parse_value(p_any, p);
                parse_value(uv_any, uv);
                parse_value(n_any, n);
            }
            {
                split = split_at_char(vertex_1, '/');
                string p = split.first;
                split = split_at_char(substring(split.remaining, 1), '/');
                string uv = split.first;
                string n = substring(split.remaining, 1);

                Any p_any = {TypeOf(int), &v1_p};
                Any uv_any = {TypeOf(int), &v1_uv};
                Any n_any = {TypeOf(int), &v1_n};
                parse_value(p_any, p);
                parse_value(uv_any, uv);
                parse_value(n_any, n);
            }
            {
                split = split_at_char(vertex_2, '/');
                string p = split.first;
                split = split_at_char(substring(split.remaining, 1), '/');
                string uv = split.first;
                string n = substring(split.remaining, 1);

                Any p_any = {TypeOf(int), &v2_p};
                Any uv_any = {TypeOf(int), &v2_uv};
                Any n_any = {TypeOf(int), &v2_n};
                parse_value(p_any, p);
                parse_value(uv_any, uv);
                parse_value(n_any, n);
            }

            // NOTE(lubo): OBJ indexes from 1
            v0_p -= 1;
            v0_uv -= 1;
            v0_n -= 1;
            v1_p -= 1;
            v1_uv -= 1;
            v1_n -= 1;
            v2_p -= 1;
            v2_uv -= 1;
            v2_n -= 1;

            u16 v0_p16 = safecast_to_u16(v0_p);
            u16 v0_uv16 = safecast_to_u16(v0_uv);
            u16 v0_n16 = safecast_to_u16(v0_n);
            u16 v1_p16 = safecast_to_u16(v1_p);
            u16 v1_uv16 = safecast_to_u16(v1_uv);
            u16 v1_n16 = safecast_to_u16(v1_n);
            u16 v2_p16 = safecast_to_u16(v2_p);
            u16 v2_uv16 = safecast_to_u16(v2_uv);
            u16 v2_n16 = safecast_to_u16(v2_n);

            u64 v0_combined_key = (u64)v0_p | (u64)v0_uv << 16 | (u64)v0_n << 32;
            u64 v1_combined_key = (u64)v1_p | (u64)v1_uv << 16 | (u64)v1_n << 32;
            u64 v2_combined_key = (u64)v2_p | (u64)v2_uv << 16 | (u64)v2_n << 32;

            int unique_vertex_index = get_index_of(&unique_p_uv_n_combinations, v0_combined_key);
            if(unique_vertex_index == -1)
            {
                // TODO(lubomir): ordered append?
                append_to_list(&unique_p_uv_n_combinations, v0_combined_key);

                unique_vertex_index = unique_vertex_count;
                unique_vertex_count += 1;
                
                append_to_list(&vertex_indices, unique_vertex_index);
                append_to_list(&p_list, get_by_index(&p_list_pure, v0_p));
                append_to_list(&uv_list, get_by_index(&uv_list_pure, v0_uv));
                append_to_list(&n_list, get_by_index(&n_list_pure, v0_n));
            }
            else
            {
                append_to_list(&vertex_indices, unique_vertex_index);
            }

            unique_vertex_index = get_index_of(&unique_p_uv_n_combinations, v1_combined_key);
            if(unique_vertex_index == -1)
            {
                // TODO(lubomir): ordered append?
                append_to_list(&unique_p_uv_n_combinations, v1_combined_key);

                unique_vertex_index = unique_vertex_count;
                unique_vertex_count += 1;
                
                append_to_list(&vertex_indices, unique_vertex_index);
                append_to_list(&p_list, get_by_index(&p_list_pure, v1_p));
                append_to_list(&uv_list, get_by_index(&uv_list_pure, v1_uv));
                append_to_list(&n_list, get_by_index(&n_list_pure, v1_n));
            }
            else
            {
                append_to_list(&vertex_indices, unique_vertex_index);
            }

            unique_vertex_index = get_index_of(&unique_p_uv_n_combinations, v2_combined_key);
            if(unique_vertex_index == -1)
            {
                // TODO(lubomir): ordered append?
                append_to_list(&unique_p_uv_n_combinations, v2_combined_key);

                unique_vertex_index = unique_vertex_count;
                unique_vertex_count += 1;
                
                append_to_list(&vertex_indices, unique_vertex_index);
                append_to_list(&p_list, get_by_index(&p_list_pure, v2_p));
                append_to_list(&uv_list, get_by_index(&uv_list_pure, v2_uv));
                append_to_list(&n_list, get_by_index(&n_list_pure, v2_n));
            }
            else
            {
                append_to_list(&vertex_indices, unique_vertex_index);
            }
        }
    }

    Mesh mesh = {};
    mesh.p = p_list;
    mesh.uv = uv_list;
    mesh.n = n_list;
    mesh.indices = vertex_indices;

    return mesh;
}
