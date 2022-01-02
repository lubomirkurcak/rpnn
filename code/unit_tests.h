/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2019 All Rights Reserved. $
   ======================================================================== */

u8 string_ringbuffer_test_buffer[80];

internal void string_ringbuffer_test()
{
    String_Ringbuffer _sring;
    String_Ringbuffer *sring = &_sring;
    init_string_ringbuffer(sring, StaticBuffer(string_ringbuffer_test_buffer));

    Assert(insert_string(sring, String("Test")));
    Assert(insert_string(sring, String("Another")));
    Assert(insert_string(sring, String("Please work?")));
    Assert(insert_string(sring, String("At some point we should fail.")));
    Assert(insert_string(sring, String("What is the point though?")));
    Assert(insert_string(sring, String("Who knows...")));
    Assert(insert_string(sring, String("I don't.")));
    Assert(insert_string(sring, String("But I do!")));

    smm offset = 0;
    for(int item_index=0; item_index<sring->item_count; ++item_index)
    {
        string first;
        string second;
        offset = get_string_at_offset(sring, offset, &first, &second);

        string whole = first;
        if(second.size)
        {
            whole = concatenate(first, second);
        }

        switch(item_index)
        {
            case 0: Assert(whole == String("Who knows...")); break;
            case 1: Assert(whole == String("I don't.")); break;
            case 2: Assert(whole == String("But I do!")); break;
        }

        if(second.size)
        {
            general_deallocate(whole);
        }
    }
}

internal void small_gjk_test()
{
    v3 vertices[] =
    {
        {0,0,0},
        {0,0,1},
        {0,1,0},
        {0,1,1},
        {1,0,0},
        {1,0,1},
        {1,1,0},
        {1,1,1},
    };

    Polyhedron poly;
    poly.vertices = vertices;
    poly.vertex_count = ArrayCount(vertices);

    Entity entity_a = {};
    Collider collider_a = {};
    {
        Entity *entity = &entity_a;
        entity->active = true;
        entity->transform = default_transform();
        
        Collider *collider = &collider_a;
        collider->radius = 0;
        collider->poly = poly;
    }
    
    Entity entity_b = {};
    Collider collider_b = {};
    {
        Entity *entity = &entity_b;
        entity->active = true;
        entity->transform = default_transform();
        
        Collider *collider = &collider_b;
        collider->radius = 0;
        collider->poly = poly;
    }
    
    Entity entity_point = {};
    Collider collider_point = {};
    {
        Entity *entity = &entity_point;
        entity->active = true;
        entity->transform = default_transform();
        
        Collider *collider = &collider_point;
        collider->radius = 0;
        collider->poly = {};
    }

    entity_a.transform.pos = {0,0,0};
    entity_b.transform.pos = {0,0,0};

    // NOTE(lubo): I don't know if collider_a.poly is used twice here on purpose, probably should have noted that XD
    // TODO(lubo): Re-enable early collision detection for this to work again.
    Assert(gjk(&entity_a.transform, collider_a.poly, collider_a.radius,
               &entity_b.transform, collider_a.poly, collider_b.radius,
               false, false).collides == true);

    // NOTE(lubo): This situation:
    // Y    
    // ^   Z    *--*   *--*
    // |  ^    /  /|  /  /|
    // | /    *--* * *--* *
    // |/     |A |/  |B |/ 
    //        *--*   *--*  
    //        0  1   2  3   4  5  ----> X
    entity_a.transform.pos = {0,0,0};
    entity_b.transform.pos = {2,0,0};
    Assert(gjk(&entity_a.transform, collider_a.poly, collider_a.radius,
               &entity_b.transform, collider_b.poly, collider_b.radius,
               false, false).collides == false);

    #if 0
    // NOTE(lubo): This is testing an extreme case: single point at origin
    // colliding with a cube surface (its corner) at (0,0,0).
    // We don't actually care if we detect a collision here or not.
    // So, if this test ever fails, it's probably OK to just remove it.
    entity_point.transform.pos = {0,0,0};
    entity_a.transform.pos = {0,0,0};
    Assert(gjk(&entity_a.transform, entity_a.collider.poly,
               &entity_b.transform, entity_b.collider.poly,
               false, false).collides == true);
    #endif

    //FinishThisTest;
}

internal void barycentric_test()
{
    v2 a = {0,1};
    v2 b = {1,0};
    v2 c = {2,1};
    v2 d = {1,1};
    v2 lambda = get_barycentric_coordinates(a, b, c, d);
    Assert(lambda.x == 0);
    Assert(lambda.y == 0.5);
}

internal void quaternion_test()
{
    quaternion q = {0,0,0,1};
    v3 v = {1,2,3};

    v3 _v = q*v;

    Assert(v.x == _v.x && v.y == _v.y && v.z == _v.z);

    v3 axis_of_rotation = normalize(V3(0,0,1));
    quaternion a = quaternion_axis_theta(axis_of_rotation, 0);
    quaternion b = quaternion_axis_theta(axis_of_rotation, 0.001f);

    quaternion c = a*b;
}

#if 0
// TODO(lubo): This is not very clean!
global buffer global_debug_mnist_test_bitmap_buffer;
global Loaded_Bitmap global_debug_mnist_test_bitmap;

// NOTE(lubo): This is outdated usage, see neural_net_test() for API
internal void neural_net_test5()
{
    int layer_sizes[] = {1, 3, 2, 1};
    Neural_Network _net = neural_net_parametrized(ArrayCount(layer_sizes), layer_sizes, 0.01f);
    Neural_Network *net = &_net;
    
    matrix input = get_by_index(&net->activations, 0);
    matrix output = net->expected_output;

    Random_Series random = {};

    float best_cost = float_max;
    int runs_without_display=0;
    for(;;)
    {
        float a = get_random_float(&random);
        float b = get_random_float(&random);
        float c = get_random_float(&random);
        
        *(float *)matrix_access(input, 0) = a;
        
        *(float *)matrix_access(output, 0) = a*a;
        
        backprop(net);

        float cost = calculate_cost(net);
        
        // float learning_rate = 0.01f;

        if(runs_without_display > 100000)
        {
            runs_without_display = 0;
            //best_cost = cost;
            loginfo("neural_net", "Cost %f LR %f", cost, net->learning_rate);
        }
        runs_without_display += 1;

        for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
        {
            matrix weights = get_by_index(&net->weights, layer_index);
            matrix biases = get_by_index(&net->biases, layer_index);
            matrix del_weights = get_by_index(&net->del_weights, layer_index);
            matrix del_biases = get_by_index(&net->del_biases, layer_index);
        
            multiply(del_weights, -net->learning_rate);
            multiply(del_biases, -net->learning_rate);
            add(weights, del_weights);
            add(biases, del_biases);
        }
    }
}

internal void neural_net_test4()
{
    int layer_sizes[] = {3, 3, 2};
    float learning_rate = 0.01f;
    Neural_Network _net = neural_net_parametrized(ArrayCount(layer_sizes), layer_sizes, learning_rate);
    Neural_Network *net = &_net;
    
    matrix input = get_by_index(&net->activations, 0);
    matrix output = net->expected_output;

    Random_Series random = {};

    int runs_without_display=0;
    for(;;)
    {
        float a = get_random_float(&random);
        float b = get_random_float(&random);
        float c = get_random_float(&random);
        
        *(float *)matrix_access(input, 0) = a;
        *(float *)matrix_access(input, 1) = b;
        *(float *)matrix_access(input, 2) = c;
        
        *(float *)matrix_access(output, 0) = a+b;
        *(float *)matrix_access(output, 1) = b+c;
        
        backprop(net);
        
        if(runs_without_display > 100000)
        {
            runs_without_display = 0;
            float cost = calculate_cost(net);
            loginfo("neural_net", "Cost %f LR %f", cost, learning_rate);
        }
        runs_without_display += 1;

        accumulate_update(net, 1);
        apply_updates(net);
        
        // for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
        // {
        //     matrix weights = get_by_index(&net->weights, layer_index);
        //     matrix biases = get_by_index(&net->biases, layer_index);
        //     matrix del_weights = get_by_index(&net->del_weights, layer_index);
        //     matrix del_biases = get_by_index(&net->del_biases, layer_index);
        
        //     multiply(del_weights, -learning_rate);
        //     multiply(del_biases, -learning_rate);
        //     add(weights, del_weights);
        //     add(biases, del_biases);
        // }
    }
}

// NOTE(lubo): This is outdated usage, see neural_net_test() for API
internal void neural_net_test3()
{
    int layer_sizes[] = {3, 2};
    float learning_rate = 0.01f;
    Neural_Network _net = neural_net_parametrized(ArrayCount(layer_sizes), layer_sizes, learning_rate);
    Neural_Network *net = &_net;
    
    matrix input = get_by_index(&net->activations, 0);
    matrix output = net->expected_output;

    int total=0;
    int successful=0;
    int max_iters = 40000;
    float cost_target = 0.0000000001;
    for(;;)
    {
        reinitialize_weights_and_biases(net);
    
        int iter = 0;
    
        float cost = float_max;
        for(; iter<max_iters && cost>cost_target; ++iter)
        {
            float a = get_random_float(&net->random_series);
            float b = get_random_float(&net->random_series);
            float c = get_random_float(&net->random_series);
        
            *(float *)matrix_access(input, 0) = a;
            *(float *)matrix_access(input, 1) = b;
            *(float *)matrix_access(input, 2) = c;
        
            *(float *)matrix_access(output, 0) = a+b;
            *(float *)matrix_access(output, 1) = b+c;
        
            backprop(net);
            cost = calculate_cost(net);
        
        
            // loginfo("neural_net", "Iter %d Cost %f LR %f", iter, cost, learning_rate);

            for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
            {
                matrix weights = get_by_index(&net->weights, layer_index);
                matrix biases = get_by_index(&net->biases, layer_index);
                matrix del_weights = get_by_index(&net->del_weights, layer_index);
                matrix del_biases = get_by_index(&net->del_biases, layer_index);
        
                multiply(del_weights, -learning_rate);
                multiply(del_biases, -learning_rate);
                add(weights, del_weights);
                add(biases, del_biases);
            }
        }
        
        if(iter == max_iters)
        {
        }
        else
        {
            ++successful;
        }
        ++total;

        // NOTE(lubo): Empirically success rate was found to be around 0.75
        loginfo("neural_net", "Success rate %f", ratio((float)successful, (float)total));
    }
}

// NOTE(lubo): This is outdated usage, see neural_net_test() for API
internal void neural_net_test2()
{
    buffer contents = read_entire_file(String("data\\mnist\\train-images.idx3-ubyte"));
    LK_IDX_File idx = process_idx_file(contents);
    buffer labels = read_entire_file(String("data\\mnist\\train-labels.idx1-ubyte"));
    LK_IDX_File idx_labels = process_idx_file(labels);
    
    matrix expected_output = create_column_vector(10);
    
    int layer_sizes[] = {idx.element_size, 16, 10};
    float learning_rate = 1.0f;
    Neural_Network _net = neural_net_parametrized(ArrayCount(layer_sizes), layer_sizes, learning_rate);
    Neural_Network *net = &_net;

    matrix input = get_by_index(&net->activations, 0);
    
    int goes = 1;
    int minibatch_size = 32;
    int batch_count = idx.elements/minibatch_size;
    
    for(int go_index=0; go_index<goes; ++go_index)
    {
        for(int batch_index=0; batch_index<batch_count; ++batch_index)
        {
            int correct_classifications = 0;
        
            float avg_batch_cost = 0;
            for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
            {
                matrix avg_del_weights = get_by_index(&net->avg_del_weights, layer_index);
                matrix avg_del_biases = get_by_index(&net->avg_del_biases, layer_index);
        
                clear(avg_del_weights);
                clear(avg_del_biases);
            }

            for(int minibatch_index=0; minibatch_index<minibatch_size; ++minibatch_index)
            {
                int element_id = batch_index*minibatch_size + minibatch_index;
        
                // NOTE(lubo): Load class label
                void *label_data = idx_get_element(idx_labels, element_id);
                int class_label = *(u8 *)label_data;
                clear(expected_output);
                *(float *)matrix_access(expected_output, class_label) = 1;

                // NOTE(lubo): Load input
                void *element_data = idx_get_element(idx, element_id);
                u8 *read = (u8 *)element_data;
                float *write = (float *)input.allocation.memory;
                for(int value_index=0; value_index<idx.element_size; ++value_index)
                {
                    u32 value = *read;
                    *write = (float)value/(float)255;

                    read++;
                    write++;
                }

                backprop(net);
            }

            //learning_rate = Min(0.000001f, exp(-0.005f*(go_index*batch_count + batch_index)));
            
            loginfo("neural_net", "Go %d/%d Batch %d/%d Cost %f Correct %d/%d LR %f", go_index, goes, batch_index, batch_count, avg_batch_cost, correct_classifications, minibatch_size, learning_rate);

            for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
            {
                matrix weights = get_by_index(&net->avg_del_weights, layer_index);
                matrix biases = get_by_index(&net->avg_del_biases, layer_index);
                matrix avg_del_weights = get_by_index(&net->avg_del_weights, layer_index);
                matrix avg_del_biases = get_by_index(&net->avg_del_biases, layer_index);
        
                multiply(avg_del_weights, -learning_rate);
                multiply(avg_del_biases, -learning_rate);
                add(weights, avg_del_weights);
                add(biases, avg_del_biases);
            }
        }
    }
}

// TODO(lubo): Artificially expanding the training data regularization
internal void neural_net_test()
{
    buffer train_set_file = read_entire_file(String("data\\mnist\\train-images.idx3-ubyte"));
    buffer train_labels_file = read_entire_file(String("data\\mnist\\train-labels.idx1-ubyte"));
    buffer test_set_file = read_entire_file(String("data\\mnist\\t10k-images.idx3-ubyte"));
    buffer test_labels_file = read_entire_file(String("data\\mnist\\t10k-labels.idx1-ubyte"));
    
    LK_IDX_File train_set = process_idx_file(train_set_file);
    LK_IDX_File train_labels = process_idx_file(train_labels_file);
    LK_IDX_File test_set = process_idx_file(test_set_file);
    LK_IDX_File test_labels = process_idx_file(test_labels_file);

    // NOTE(lubo): Load one of the pictures into displayable texture
    {
        int display_element_id = 123;
        void *display_element_data = idx_get_element(train_set, display_element_id);
    
        global_debug_mnist_test_bitmap.width = train_set.dimension_sizes[1];
        global_debug_mnist_test_bitmap.height = train_set.dimension_sizes[2];
        global_debug_mnist_test_bitmap.channels = 4;
        global_debug_mnist_test_bitmap_buffer = general_allocate(global_debug_mnist_test_bitmap.width*global_debug_mnist_test_bitmap.height*global_debug_mnist_test_bitmap.channels);
        global_debug_mnist_test_bitmap.data = global_debug_mnist_test_bitmap_buffer.memory;
    
        u32 *write = (u32 *)global_debug_mnist_test_bitmap.data;
        for(int y=0; y<global_debug_mnist_test_bitmap.height; ++y)
        {
            u8 *read = (u8 *)display_element_data + (global_debug_mnist_test_bitmap.height-y-1)*train_set.type_size*global_debug_mnist_test_bitmap.width;
            for(int x=0; x<global_debug_mnist_test_bitmap.width; ++x)
            {
                u32 value = *read;
                #if 0
                *write = (value<<24 | value<<16 | value<<8 | value<<0);
                #else
                *write = (255<<24 | value<<16 | value<<8 | value<<0);
                #endif

                read++;
                write++;
            }
        }
    }

    // NOTE(lubo): These settings get about 94.5% on MNIST
    int layer_sizes[] = {train_set.element_size, 30, 10};
    int epochs = 30;
    int minibatch_size = 10;
    float learning_rate = 0.1f;    
    float weight_decay = 5.0f/train_set.elements;
    float dropout = 0;
    float momentum_coefficient = 0;

    // learning_rate = 0.01f;
    // epochs = 60;
    // momentum_coefficient = 0.05f;
    
    Neural_Network _net = neural_net_parametrized(ArrayCount(layer_sizes), layer_sizes, learning_rate, weight_decay, dropout, /*idx.elements, */momentum_coefficient);
    Neural_Network *net = &_net;

    //save(net);
    
    train_idx(net,
              train_set, train_labels,
              test_set, test_labels,
              epochs, minibatch_size);
}

internal void matrix_test()
{
    float _A[] =
    {
        1, 0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,-1,
        1, 0, 1, 0,-1, 0,
        0, 1, 0,-1, 0,-1,
        1, 0,-1, 0,-1, 0,
        0,-1, 0,-1, 0,-1,
        
        2, 0, 2, 0, 2, 0,
        0, 2, 0, 2, 0,-2,
        2, 0, 2, 0,-2, 0,
        0, 2, 0,-2, 0,-2,
        2, 0,-2, 0,-2, 0,
        0,-2, 0,-2, 0,-2,
        
        3, 0, 3, 0, 3, 0,
        0, 3, 0, 3, 0,-3,
        3, 0, 3, 0,-3, 0,
        0, 3, 0,-3, 0,-3,
        3, 0,-3, 0,-3, 0,
        0,-3, 0,-3, 0,-3,
    };
    matrix A = {6,6,3,1};
    A.allocation.memory = (u8 *)_A;

    float _B[] =
    {
        // 3x3x3 feature 1
        1, 0, 1,
        0, 1, 0,
        1, 0, 1,

        5, 0, 5,
        0, 5, 0,
        5, 0, 5,

        9, 0, 9,
        0, 9, 0,
        9, 0, 9,

        // 3x3x3 feature 2
        0, 1, 0,
        1, 0, 1,
        0, 1, 0,

        0, 5, 0,
        5, 0, 5,
        0, 5, 0,

        0, 9, 0,
        9, 0, 9,
        0, 9, 0,
    };
    matrix B = {3,3,3,2};
    B.allocation.memory = (u8 *)_B;

    matrix C = convolution(A, B);
    matrix D = pooling(C);

    //matrix E = D*D;
    int x = 1;
}
#endif

/*
internal void zkgra_codes()
{
    int dim = 5;
    matrix A = create_matrix(dim,dim);
    matrix R = create_matrix(dim,dim);
    float *A_values = (float *)A.allocation.memory;

    int matrix_elements = dim*dim;
    for(u32 bits=0; bits<(1U<<(matrix_elements)); ++bits)
    {
        for(int element=0; element<matrix_elements; ++element)
        {
            A_values[element] = (bits & (1 << element)) ? (float)1 : (float)0;
        }

        if(is_any_column_or_row_all_zeroes(A))
        {
            
        }
        else
        {
            multiply(R, A, A);
            scalar_op(R, R, MOP_MODULO, 2);

            if(is_zeroes(R))
            {
                int WOW = 3;
                int DUDDE = 1;
            }
        }
    }

    int SADDDD = 3;
    int VERY = 5;
}
*/

internal void project_to_plane_test()
{
    v3 a = {1,0,0};
    v3 p = {1,1,1};
    v3 o = {0,0,0};

    v3 d = distance_to_plane(o, p, a);
    
    v3 c = {(float)1/3, (float)1/3, (float)1/3};
    Assert(approximately(d, c));
}

internal void fuzzy_cluster_analysis()
{
    v2 data[] =
    {
        {9.5, 2.1},
        {16.0, 2.5},
        {20.0, 2.3},
        {24.5, 2.8},
        {26.0, 3.4},
        {27.0, 4.3},
        {32.0, 4.8},
        {38.0, 5.0},
        {41.0, 4.7},
        {47.0, 5.3},
    };

    int c = 3;

    
}

#include "geometric_algebra.h"

internal void unit_tests_run()
{
    //Assert(false, "Assertion fail detection test. Success! :D");
    
    string_ringbuffer_test();

    //gjk_test();
    small_gjk_test();

    barycentric_test();

    quaternion_test();

    project_to_plane_test();

    //simple_fuzzy_strategy_management();
    
    // matrix_test();

    //zkgra_codes();
    
    //neural_net_test5();
    //neural_net_test3();
    //neural_net_test2();
    //neural_net_test4();
    
    // neural_net_test();

    // print_geometric_product_for_dimension(4);
}
