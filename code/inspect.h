/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */


// TODO(lubo): This is awful. It's super hard to add an extra lines to e.g. display composed transform matrix.
// TODO(lubo): Split into UI code and usage code.

struct Inspect_Result
{
    r2 bounds;
    b32x clicked;
};

internal Inspect_Result inspect(Game_State *state, Game_Input *input, Baked_Font *font, r2 inspect_region, buffer format_buffer, Any any, string name={}, float offset_y=0, int depth=0, b32x is_expanded=true)
{
    Inspect_Result result = {};

    UI_State *ui_state = &state->ui_state;
    
    // TODO(lubo): Store more upper bytes? Some lower bytes are likely zeroes.
    UI_ID uiid = {};
    uiid.owner = (smm)inspect;
    uiid.item = (smm)any.memory;
                        
    r2 total_bounds = rect_union_identity();

    Struct_Member *members = StructMembersOf(any.type);

    b32x HANDLED = false;
    if(depth == 0 && is_null_or_empty(name))
    {
        HANDLED = true;
        r2 bounds = draw_text(font, String(TypeName(any.type)), inspect_region, {0,1}, {0,1}, {0, offset_y}, {10, 0}, {1,1,1,1}, {0.4,0.6,0.5,1});
        total_bounds = rect_union(total_bounds, bounds);
    }

    v2 line_padding = {16,0};

    if(any.count)
    {
        r2 key_bounds = get_text_bounds(font, name, inspect_region, {0,1}, {0,1}, {0, offset_y}, line_padding);
        key_bounds.max.x = inspect_region.max.x;
        b32x moused_over = false;
        if(collides(input->mouse_pixel, key_bounds))
        {
            set_hot(ui_state, uiid);
            moused_over = true;
            if(was_pressed(&input->left_mouse))
            {
                click_value(any);
                result.clicked = true;
            }
        }

        v4 key_color = {1,1,1,1};
        v4 key_background_color = lerp(V4(0,0,0,1), V4(1,1,1,1), depth/10.0f);
        if(moused_over)
        {
            key_background_color = {0.5,0.8,0.65,1};
        }
        if(members && !is_expanded) key_color = {0.5,0.5,1,1};
        key_bounds = draw_text(font, name, key_bounds, line_padding, key_color, key_background_color);
        total_bounds = rect_union(total_bounds, key_bounds);
                
        for(int element_index=0; element_index<any.count; ++element_index)
        {
            void *element_pointer = (u8 *)any.memory + element_index*SizeOf(any.type);

            string before = String("[");
            string after = String("]");

            Any any_before = {TypeOf(string), &before};
            Any any_after = {TypeOf(string), &after};
            Any num = {TypeOf(int), &element_index};
            
            string index_string = {};
            index_string = format_value(format_buffer, any_before, index_string.size);
            index_string = format_value(format_buffer, num, index_string.size);
            index_string = format_value(format_buffer, any_after, index_string.size);
            
            Any sub = {};
            sub.type = any.type;
            sub.memory = element_pointer;

            
            offset_y = (total_bounds.min.y - inspect_region.max.y);
            Inspect_Result subresult = inspect(state, input, font, inspect_region, format_buffer, sub, index_string, offset_y, depth+1, true);
            //if(subresult.clicked) member->expanded = !member->expanded;
            r2 bounds = subresult.bounds;
            total_bounds = rect_union(total_bounds, bounds);
        }
    }
    else
    {
        if(members)
        {
            if(!HANDLED)
            {
                r2 key_bounds = get_text_bounds(font, name, inspect_region, {0,1}, {0,1}, {0, offset_y}, line_padding);
                b32x moused_over = false;
                key_bounds.max.x = inspect_region.max.x;
                if(collides(input->mouse_pixel, key_bounds))
                {
                    set_hot(ui_state, uiid);
                    moused_over = true;
                    if(was_pressed(&input->left_mouse))
                    {
                        click_value(any);
                        result.clicked = true;
                    }
                }

                v4 key_color = {0.5,0.5,0.5,1};
                v4 key_background_color = lerp(V4(0,0,0,1), V4(1,1,1,1), depth/10.0f);
                if(moused_over)
                {
                    key_background_color = {0.5,0.8,0.65,1};
                }
                if(members && !is_expanded) key_color = {0.5,0.7,1,1};
                key_bounds = draw_text(font, name, key_bounds, line_padding, key_color, key_background_color);
                total_bounds = rect_union(total_bounds, key_bounds);
            }
            
            if(is_expanded)
            {
                for(int member_index=0; member_index<StructMemberCount(any.type); ++member_index)
                {
                    Struct_Member *member = members + member_index;

                    Any sub = {};
                    sub.type = member->type;
                    sub.memory = (void *)((u8 *)any.memory + member->offset);

                    if(member->int_count_offset != -1)
                    {
                        Assert(member->deref_count == 1);
                        sub.memory = *(void **)sub.memory;
                        sub.count = *(int *)((u8 *)any.memory + member->int_count_offset);
                    }

                    offset_y = (total_bounds.min.y - inspect_region.max.y);
                    Inspect_Result subresult = inspect(state, input, font, inspect_region, format_buffer, sub, String(member->name), offset_y, depth+1, member->expanded);
                    if(subresult.clicked) member->expanded = !member->expanded;
                    r2 bounds = subresult.bounds;
                    total_bounds = rect_union(total_bounds, bounds);
                }
            }
        }
        else
        {
            r2 key_bounds = get_text_bounds(font, name, inspect_region, {0,1}, {0,1}, {0, offset_y}, line_padding);
            float offset_x = key_bounds.max.x - inspect_region.min.x;
            r2 value_bounds;
            switch(any.type)
            {
                case TypeOf(b32):
                case TypeOf(b32x):
                {
                    value_bounds = get_checkbox_bounds(get_height(key_bounds), inspect_region, {0,1}, {0,1}, {offset_x, offset_y});
                } break;
                
                default:
                {
                    value_bounds = get_text_bounds(font, format_value(format_buffer, any), inspect_region, {0,1}, {0,1}, {offset_x, offset_y}, line_padding);
                } break;
            }
            
            r2 field_bounds = rect_union(key_bounds, value_bounds);

            field_bounds.max.x = inspect_region.max.x;

            v4 key_color = {1,1,1,1};
            v4 key_background_color = lerp(V4(0,0,0,1), V4(1,1,1,1), depth/10.0f);
            v4 value_color = {1,1,0.7,1};
            v4 value_background_color = {0,0,0,0};

            string value = format_value(format_buffer, any);
            
            if(collides(input->mouse_pixel, field_bounds))
            {
                if(set_hot(ui_state, uiid))
                {
                    key_background_color = value_background_color = {0.5,0.8,0.65,1};
                    if(was_pressed(&input->left_mouse))
                    {
                        set_active(ui_state, uiid);
                        // ui_state->dragging_start_x = input->mouse_x_pixel;
                        // ui_state->dragging_start_y = input->mouse_y_pixel;
                        ui_state->dragging_start = input->mouse_pixel;
                    }
                }
            }

            b32x show_cursor = false;
            if(is_active(ui_state, uiid))
            {
                if(ui_state->editting)
                {
                    key_background_color = value_background_color = {0.365,0.2,0.45,1};
                    Textfield *field = &state->textfield;

                    value = get_string(field);
                    show_cursor = true;

                    if(!parse_value(any, value))
                    {
                        key_background_color = value_background_color = {1,0,0,1};
                    }
                    
                    if(!state->textfield_active)
                    {
                        set_active(ui_state, {});
                        ui_state->editting = false;
                        show_cursor = false;
                
                        if(state->textfield_entered)
                        {
                            state->textfield_entered = false;
                        }
                        else
                        {
                            // TODO(lubo): Undo
                        }
                    }
                }
                else
                {
                    if(is_pressed(&input->left_mouse))
                    {
                        key_background_color = value_background_color = {1,0.8,0.65,1};
                        b32x dragging = ui_state->dragging ? true : (quadrance(input->mouse_pixel - ui_state->dragging_start) > square(5));
                        if(dragging)
                        {
                            ui_state->dragging = true;
                            float sensitivity = 0.01f;
                            if(is_pressed(&input->shift))
                            {
                                sensitivity = 0.001f;
                                
                                if(is_pressed(&input->control))
                                {
                                    sensitivity = 0.0001f;
                                }
                            }
                            else if(is_pressed(&input->control))
                            {
                                sensitivity = 0.1f;
                            }
                            ui_state->dragging_accumulator = hold_value(any, ui_state->dragging_accumulator + sensitivity*input->mouse_delta_x_raw, is_pressed(&input->alt));
                            key_background_color = value_background_color = {1,0,0.65,1};
                        }
                    }
                    else
                    {
                        if(!ui_state->dragging && was_released(&input->left_mouse) && is_hot(ui_state, uiid))
                        {
                            result.clicked = true;
                            b32x action_needed = click_value(any);
                        
                            if(action_needed)
                            {
                                ui_state->editting = true;
                                state->textfield_active = true;
                                string initial_text_value = format_value(format_buffer, any);

                                Textfield *field = &state->textfield;
                                clear_text(field);
                                insert_text(field, initial_text_value);
                            }
                            else
                            {
                                set_active(ui_state, {});
                            }
                        }
                        else
                        {
                            ui_state->dragging = false;
                            set_active(ui_state, {});
                        }
                    }
                }
            }

            //if(members && !is_expanded) key_color = {0.5,0.5,1,1};
            
            key_bounds = draw_text(font, name, key_bounds, line_padding, key_color, key_background_color);
            r2 highlight_rect = {};

            switch(any.type)
            {
                case TypeOf(b32):
                {
                    show_cursor = false;
                    value_color = {1,1,1,1};
                    value_bounds = draw_checkbox(*(b32 *)any.memory, value_bounds, value_color, value_background_color);
                } break;
                
                case TypeOf(b32x):
                {
                    show_cursor = false;
                    value_color = {1,1,1,1};
                    value_bounds = draw_checkbox(*(b32x *)any.memory, value_bounds, value_color, value_background_color);
                } break;
                
                default:
                {
                    value_bounds = draw_text(font, value, value_bounds, line_padding, value_color, value_background_color, 0,
                                             state->textfield.cursor_pos, state->textfield.cursor_other,
                                             &highlight_rect.min, &highlight_rect.max);
                } break;
            }

            if(show_cursor)
            {
                if(state->textfield.cursor_other < state->textfield.cursor_pos)
                {
                    v2 swap = highlight_rect.max;
                    highlight_rect.max = highlight_rect.min;
                    highlight_rect.min = swap;
                }
                set_immediate_texture(global_textures[Texture_White]);

                highlight_rect.min.y += font->line_descent;//bounding_box.min.y;
                highlight_rect.max.y += font->line_ascent;//bounding_box.max.y;
                highlight_rect.min.x -= 1;
                highlight_rect.max.x += 1;
                debug_immediate_rect_ui(highlight_rect, V4(0.5,0.5,0.5,0));
            }
            
            field_bounds = rect_union(key_bounds, value_bounds);

            total_bounds = rect_union(total_bounds, field_bounds);

            //total_bounds.max.x = inspect_region.max.x;
        }
    }

    
    
//    return total_bounds;
    result.bounds = total_bounds;
    return result;
}
