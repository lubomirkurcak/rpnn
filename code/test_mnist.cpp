/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

//global buffer global_debug_mnist_test_bitmap_buffer;
//global Loaded_Bitmap global_debug_mnist_test_bitmap;

// TODO(lubo): Artificially expanding the training data regularization
internal int train_mnist(
    Neural_Network_Hyperparams params,
    int layer_count, int *layer_sizes,
    int epochs = 30,
    int minibatch_size = 10)
{
    buffer train_set_file = read_entire_file(String("..\\data\\mnist\\train-images.idx3-ubyte"));
    buffer train_labels_file = read_entire_file(String("..\\data\\mnist\\train-labels.idx1-ubyte"));
    buffer test_set_file = read_entire_file(String("..\\data\\mnist\\t10k-images.idx3-ubyte"));
    buffer test_labels_file = read_entire_file(String("..\\data\\mnist\\t10k-labels.idx1-ubyte"));
    
    LK_IDX_File train_set = process_idx_file(train_set_file);
    LK_IDX_File train_labels = process_idx_file(train_labels_file);
    LK_IDX_File test_set = process_idx_file(test_set_file);
    LK_IDX_File test_labels = process_idx_file(test_labels_file);

    #if 0
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
    #endif
    
    Neural_Network _net = create_feedforward_net(params, layer_count, layer_sizes);
    Neural_Network *net = &_net;

    //save(net);
    
    int correctly_classified_after_training = train_idx(net,
              train_set, train_labels,
              test_set, test_labels,
              epochs, minibatch_size);

    incinerate(net);
              
    return correctly_classified_after_training;
}
