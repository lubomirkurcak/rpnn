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
internal void matrix_test()
{
    double _A[] =
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

    double _B[] =
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

internal void transpose_test()
{
    double _A[] =
    {
        1, 2, 3,
        4, 5, 6,
    };
    matrix A = {2,3,1,1};
    A.allocation.memory = (u8 *)_A;
    A.allocation.size = sizeof(_A);

    double _B[] =
    {
        1, 4,
        2, 5,
        3, 6,
    };
    matrix B = {3,2,1,1};
    B.allocation.memory = (u8 *)_B;
    B.allocation.size = sizeof(_B);

    matrix C = transpose(A);
    
    Assert(matrices_are_equal(B, C));

    incinerate(C);
}

internal void matrix_factorization_test()
{
    double _C[] =
    {
        3, 1, 1, 3, 1,
        1, 2, 4, 1, 3,
        3, 1, 1, 3, 1,
        4, 3, 5, 4, 4,
    };
    matrix C = {4,5,1,1};
    C.allocation.memory = (u8 *)_C;
    C.allocation.size = sizeof(_C);

    Neural_Network_Hyperparams params = default_hyperparams();
    params.learning_rate = 0.001f;
    
    int features = 2;
    int epochs = 500;
    b32x non_negative = true;
    matrix_factorization(&params, C, features, epochs, non_negative);
}

internal void matrix_factorization_test2()
{
    // NOTE(lubo): Uniformly random integers from [1, 5]
    double _C[] =
    {
        5, 2, 5, 4, 1, 5, 5, 3, 1, 3, 2, 4, 5, 2, 3, 1, 4, 3, 5, 3, 1, 2, 5, 4, 3, 5, 5, 3, 3, 1, 2, 1, 4, 5, 5, 1, 2, 3, 1, 5, 1, 3, 2, 1, 2, 3, 3, 2, 2, 1,
        4, 5, 4, 1, 4, 1, 3, 4, 3, 4, 2, 2, 4, 2, 1, 3, 2, 3, 1, 2, 2, 1, 3, 5, 3, 2, 2, 5, 2, 1, 5, 5, 2, 3, 1, 3, 1, 2, 5, 1, 1, 1, 4, 1, 2, 3, 5, 3, 5, 2,
        1, 3, 5, 3, 4, 3, 5, 5, 1, 5, 5, 5, 5, 1, 5, 3, 2, 3, 1, 5, 4, 4, 3, 3, 1, 4, 1, 2, 4, 5, 1, 3, 3, 2, 3, 5, 5, 4, 4, 1, 5, 1, 4, 5, 2, 4, 5, 4, 5, 4,
        5, 2, 2, 3, 3, 5, 2, 3, 4, 5, 1, 4, 3, 2, 2, 4, 4, 3, 4, 3, 2, 1, 2, 5, 4, 1, 4, 5, 5, 1, 2, 4, 2, 5, 3, 1, 3, 4, 3, 4, 2, 5, 2, 4, 3, 3, 4, 5, 1, 1,
        2, 5, 1, 1, 1, 2, 2, 2, 1, 2, 1, 4, 5, 5, 4, 2, 4, 1, 5, 3, 3, 4, 1, 4, 1, 5, 4, 3, 5, 1, 3, 4, 2, 1, 5, 1, 1, 2, 1, 5, 4, 4, 4, 2, 1, 2, 1, 2, 3, 5,
        2, 5, 5, 5, 5, 5, 2, 2, 3, 5, 3, 2, 3, 5, 5, 3, 3, 3, 2, 4, 3, 2, 4, 5, 1, 4, 2, 1, 5, 2, 4, 3, 3, 4, 5, 2, 5, 3, 3, 3, 3, 2, 3, 2, 4, 3, 3, 3, 4, 2,
        5, 3, 1, 3, 2, 1, 4, 5, 4, 5, 4, 2, 4, 3, 2, 1, 4, 5, 4, 3, 1, 5, 1, 2, 4, 2, 1, 3, 2, 5, 4, 5, 3, 5, 1, 2, 3, 1, 4, 5, 5, 1, 1, 3, 2, 5, 2, 1, 1, 1,
        2, 4, 4, 1, 5, 1, 4, 4, 1, 5, 3, 3, 4, 3, 1, 3, 4, 1, 2, 5, 3, 4, 5, 5, 4, 3, 3, 5, 1, 5, 4, 4, 3, 4, 5, 5, 4, 1, 4, 4, 3, 2, 3, 5, 5, 5, 3, 3, 1, 4,
        3, 1, 2, 5, 2, 3, 1, 2, 1, 3, 5, 2, 2, 1, 5, 2, 1, 3, 2, 1, 4, 3, 2, 5, 3, 3, 5, 2, 3, 5, 4, 3, 3, 5, 5, 5, 5, 4, 1, 3, 2, 4, 1, 1, 4, 3, 1, 5, 1, 2,
        5, 5, 3, 5, 4, 3, 1, 1, 2, 2, 3, 2, 4, 4, 4, 5, 3, 1, 3, 4, 3, 4, 4, 5, 1, 5, 5, 1, 2, 3, 5, 5, 3, 5, 4, 5, 3, 3, 1, 1, 3, 2, 4, 3, 1, 3, 1, 5, 5, 2,
        5, 3, 3, 4, 3, 2, 2, 5, 4, 5, 4, 4, 3, 1, 2, 3, 5, 5, 3, 5, 1, 1, 4, 1, 1, 2, 3, 3, 5, 3, 5, 5, 1, 5, 2, 3, 1, 5, 1, 5, 5, 5, 1, 4, 5, 5, 2, 5, 1, 4,
        4, 5, 2, 2, 3, 1, 1, 4, 1, 2, 5, 3, 3, 2, 4, 2, 3, 4, 5, 2, 5, 3, 4, 5, 3, 5, 5, 1, 4, 4, 1, 2, 5, 3, 1, 3, 2, 2, 5, 3, 1, 5, 3, 1, 5, 4, 1, 2, 3, 4,
        1, 5, 2, 5, 5, 4, 4, 4, 3, 2, 2, 4, 1, 1, 1, 5, 4, 2, 2, 5, 5, 4, 1, 5, 1, 5, 1, 4, 4, 1, 4, 2, 4, 3, 5, 5, 2, 4, 3, 2, 5, 1, 1, 4, 1, 3, 5, 5, 3, 4,
        5, 3, 4, 5, 4, 4, 3, 1, 5, 5, 4, 4, 3, 4, 5, 2, 4, 5, 5, 2, 2, 3, 1, 3, 1, 3, 1, 5, 3, 1, 1, 4, 5, 5, 4, 4, 3, 5, 1, 2, 1, 1, 4, 2, 3, 2, 1, 1, 3, 3,
        5, 5, 1, 5, 3, 1, 3, 1, 2, 3, 5, 5, 2, 5, 5, 3, 4, 2, 5, 5, 1, 5, 3, 1, 5, 2, 4, 5, 2, 3, 1, 2, 1, 3, 4, 2, 2, 3, 3, 3, 4, 5, 4, 2, 4, 4, 3, 2, 4, 3,
        4, 3, 1, 3, 3, 5, 5, 5, 2, 3, 3, 5, 2, 2, 2, 3, 4, 2, 4, 2, 4, 1, 2, 4, 5, 5, 1, 2, 5, 2, 5, 3, 2, 5, 4, 1, 5, 1, 2, 1, 1, 1, 5, 3, 5, 1, 3, 2, 2, 2,
        2, 1, 5, 2, 5, 5, 3, 1, 2, 5, 3, 3, 5, 2, 4, 1, 2, 3, 3, 3, 2, 5, 5, 5, 2, 1, 4, 3, 4, 1, 4, 3, 4, 5, 5, 4, 3, 2, 5, 5, 2, 5, 3, 5, 3, 4, 3, 3, 1, 2,
        1, 1, 5, 5, 5, 3, 2, 2, 3, 5, 5, 4, 5, 4, 5, 4, 2, 3, 2, 2, 5, 2, 1, 4, 4, 1, 3, 3, 4, 3, 5, 4, 1, 5, 4, 3, 4, 3, 1, 2, 1, 4, 2, 2, 1, 4, 1, 4, 4, 2,
        3, 1, 1, 4, 1, 1, 1, 4, 1, 4, 4, 1, 4, 5, 4, 5, 4, 5, 1, 3, 1, 5, 4, 1, 5, 1, 4, 3, 3, 4, 4, 1, 4, 3, 5, 2, 4, 3, 4, 4, 5, 4, 1, 1, 2, 2, 2, 5, 5, 1,
        2, 1, 1, 5, 2, 1, 1, 4, 1, 2, 1, 4, 2, 5, 4, 4, 1, 2, 5, 4, 3, 1, 1, 3, 2, 1, 3, 2, 5, 4, 2, 2, 5, 1, 2, 2, 3, 2, 2, 3, 3, 5, 1, 3, 4, 5, 2, 3, 1, 1,
    };
    matrix C = {20,50,1,1};
    C.allocation.memory = (u8 *)_C;
    C.allocation.size = sizeof(_C);

    Neural_Network_Hyperparams params = default_hyperparams();
    params.learning_rate = 0.001f;

    // NOTE(lubo): Experimental results (50000 epochs, 0.001 learning_rate, squared error cost):
    // 10 features - Final cost: 521.018701
    // 15 features - Final cost: 179.855787
    // 20 features - Final cost: 19.138323
    // 25 features - Final cost: 0.031741
    int features = 10;
    int epochs = 50000;
    b32x non_negative = true;
    matrix_factorization(&params, C, features, epochs, non_negative);
}

internal void matrix_factorization_test3()
{
    double _A[] =
    {
       1, 0,
       0, 1,
       1, 0,
       1, 1,
    };
    matrix A = {4,2,1,1};
    A.allocation.memory = (u8 *)_A;
    A.allocation.size = sizeof(_A);

    double _B[] =
    {
        3, 1, 1, 3, 1,
        1, 2, 4, 1, 3,
    };
    matrix B = {2,5,1,1};
    B.allocation.memory = (u8 *)_B;
    B.allocation.size = sizeof(_B);

    matrix C = A*B;

    Neural_Network_Hyperparams params = default_hyperparams();
    params.learning_rate = 0.0001f;
    
    int features = 2;
    int epochs = 50000;
    b32x non_negative = true;
    matrix_factorization(&params, C, features, epochs, non_negative);
}

//#include "geometric_algebra.h"

internal void unit_tests_run()
{
    //Assert(false, "Assertion fail detection test. Success! :D");
    
    string_ringbuffer_test();

    //gjk_test();
    //small_gjk_test();

    barycentric_test();

    quaternion_test();

    project_to_plane_test();

    //simple_fuzzy_strategy_management();
    
    //matrix_test();

    //zkgra_codes();
    
    //neural_net_test5();
    //neural_net_test3();
    //neural_net_test2();
    //neural_net_test4();
    
    // neural_net_test();

    // print_geometric_product_for_dimension(4);

    transpose_test();

    //matrix_factorization_test();
    //matrix_factorization_test2();
    //matrix_factorization_test3();
}
