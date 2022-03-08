/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

internal Neural_Network
create_feedforward_net(Neural_Network_Hyperparams params, int layer_count, int *layer_sizes)
{
    Neural_Network net = {};

    net.params = params;
    net.layer_count = layer_count;
    
    matrix input = create_column_vector(layer_sizes[0]);
    append_to_list(&net.activations, input);

    net.expected_output = create_column_vector(layer_sizes[layer_count-1]);
    
    for(int layer_index=0; layer_index<layer_count-1; ++layer_index)
    {
        matrix previous_layer = get_by_index(&net.activations, layer_index);
        matrix layer_a = create_column_vector(layer_sizes[layer_index+1]);
        matrix layer_z = create_matrix(layer_a);
        matrix weights = create_connection_matrix(previous_layer, layer_a);
        matrix biases = create_matrix(layer_a);
        randomize_values(weights);
        clear(biases);
        
        append_to_list(&net.activations, layer_a);
        append_to_list(&net.zs, layer_z);
        append_to_list(&net.weights, weights);
        append_to_list(&net.biases, biases);

        if(layer_index != layer_count-2)
        {
            matrix layer_dropout = create_matrix(layer_z);
            clear(layer_dropout);
            append_to_list(&net.dropouts, layer_dropout);
        }

        matrix del_weights = create_matrix(weights);
        matrix del_biases = create_matrix(biases);
        matrix avg_del_weights = create_matrix(weights);
        matrix avg_del_biases = create_matrix(biases);
        matrix vel_weights = create_matrix(weights);
        matrix vel_biases = create_matrix(biases);
        clear(avg_del_biases);
        clear(avg_del_weights);
        clear(vel_weights);
        clear(vel_biases);
        
        append_to_list(&net.del_weights, del_weights);
        append_to_list(&net.del_biases, del_biases);
        append_to_list(&net.avg_del_weights, avg_del_weights);
        append_to_list(&net.avg_del_biases, avg_del_biases);
        append_to_list(&net.vel_weights, vel_weights);
        append_to_list(&net.vel_biases, vel_biases);
    }

    net.input = get_by_index(&net.activations, 0);
    net.prediction = get_by_index(&net.activations, layer_count-1);
    
    return net;
}

internal void copy_neural_network(Neural_Network *dest, Neural_Network *source)
{
#define COPY_MATRIX_LIST(list)                              \
    Assert(dest->list.count == source->list.count)          \
    for(int i=0; i<dest->list.count; ++i)                   \
    {                                                       \
        matrix source_M = get_by_index(&source->list, i);   \
        matrix dest_M = get_by_index(&dest->list, i);       \
        copy_matrix(dest_M, source_M);                      \
    }

    COPY_MATRIX_LIST(activations);
    COPY_MATRIX_LIST(zs);
    COPY_MATRIX_LIST(dropouts);
    COPY_MATRIX_LIST(weights);
    COPY_MATRIX_LIST(biases);
    COPY_MATRIX_LIST(del_weights);
    COPY_MATRIX_LIST(del_biases);
    COPY_MATRIX_LIST(avg_del_weights);
    COPY_MATRIX_LIST(avg_del_biases);
    COPY_MATRIX_LIST(vel_weights);
    COPY_MATRIX_LIST(vel_biases);
}

internal void incinerate(Neural_Network *net)
{
    incinerate(net->expected_output);
    //incinerate(net->prediction); // IMPORTANT NOTE(lubo): These point to first and last activations, don't clear twice.
    //incinerate(net->input);
    
#define INCINERATE_MATRIX_LIST(list)            \
    foreach_by_value(matrix A, list)            \
    {                                           \
        incinerate(A);                          \
    }                                           \
    hard_clear_list(&list);

    INCINERATE_MATRIX_LIST(net->activations);
    INCINERATE_MATRIX_LIST(net->zs);
    INCINERATE_MATRIX_LIST(net->dropouts);
    INCINERATE_MATRIX_LIST(net->weights);
    INCINERATE_MATRIX_LIST(net->biases);
    INCINERATE_MATRIX_LIST(net->del_weights);
    INCINERATE_MATRIX_LIST(net->del_biases);
    INCINERATE_MATRIX_LIST(net->avg_del_weights);
    INCINERATE_MATRIX_LIST(net->avg_del_biases);
    INCINERATE_MATRIX_LIST(net->vel_weights);
    INCINERATE_MATRIX_LIST(net->vel_biases);
};

internal void reinitialize_weights_and_biases(Neural_Network *net)
{
    foreach_by_value(matrix w, &net->weights)
    {
        randomize_values(w);
    }

    foreach_by_value(matrix b, &net->biases)
    {
        clear(b);
    }
}

internal void create_dropouts(Neural_Network *net)
{
    Assert(net->params.dropout_keep_p < 1);
    
    for(int layer_index=0; layer_index<net->layer_count-2; ++layer_index)
    {
        matrix dropout_layer = get_by_index(&net->dropouts, layer_index);

        if(dropout_layer.height > 10)
        {
            dropout_matrix(dropout_layer, net->params.dropout_keep_p);
        }
    }
}

internal void feedforward(Neural_Network *net)
{
    b32x use_dropout = net->params.dropout_keep_p < 1;

    if(use_dropout)
    {
        create_dropouts(net);
    }
    
    for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
    {
        matrix previous_layer = get_by_index(&net->activations, layer_index);
        matrix current_layer = get_by_index(&net->activations, layer_index+1);
        matrix z = get_by_index(&net->zs, layer_index);
        matrix weights = get_by_index(&net->weights, layer_index);
        matrix biases = get_by_index(&net->biases, layer_index);
        
        multiply(z, weights, previous_layer);
        reduce_precision(z, &net->params.forward_fpopts);
        add(z, biases);
        reduce_precision(z, &net->params.forward_fpopts);
        
        if(use_dropout && layer_index != net->layer_count - 2)
        {
            matrix dropout = get_by_index(&net->dropouts, layer_index);
            hadamard(z, dropout);
        }
        
        if(layer_index == net->layer_count-2)
        {
            if(net->last_layer_softmax)
            {
                // NOTE(lubo): This is not tested.
                InvalidCodePath;
                softmax(current_layer, z);
                reduce_precision(z, &net->params.forward_fpopts);
            }
            else
            {
                copy_matrix(current_layer, z);
            }
        }
        else
        {
            relu(current_layer, z);
            //sigmoid(current_layer, z);
        }
    }
}

internal double calculate_cost(Neural_Network *net)
{
    double base_cost = cost(net->prediction, net->expected_output);

    double weights_cost = 0;
    if(net->params.weight_decay > 0/* && net->size_of_training_set > 0*/)
    {
        for(int layer_index=net->layer_count-2; layer_index>=0; --layer_index)
        {
            matrix weights = get_by_index(&net->del_weights, layer_index);
            //weights_cost += net->weight_decay/(2*net->size_of_training_set)*matrix_l2(weights);
            weights_cost += net->params.weight_decay*matrix_l2(weights);
        }
    }

    double result = base_cost + weights_cost;
    return result;
}

// NOTE(lubo): Creates delta matrix, containing error between prediction and expected output.
internal matrix calculate_delta(Neural_Network *net)
{
    matrix output = get_by_index(&net->activations, net->layer_count-1);
    
    //matrix delta = create_matrix(output);
    matrix del_biases = get_by_index(&net->del_biases, net->layer_count-2);
    matrix delta = del_biases;
    dcost(delta, output, net->expected_output);
    reduce_precision(delta, &net->params.backprop_fpopts);

    return delta;
}

// NOTE(lubo): Function deallocates/consumes 'delta'
// NOTE(lubo): Propagates error given in 'delta' backwards through the network
internal void backprop(Neural_Network *net, matrix delta)
{
    b32x use_dropout = net->params.dropout_keep_p < 1;
    
    for(int layer_index=net->layer_count-2; layer_index>=0; --layer_index)
    {
        matrix activation = get_by_index(&net->activations, layer_index);
        matrix z = get_by_index(&net->zs, layer_index);
        matrix del_weights = get_by_index(&net->del_weights, layer_index);
        matrix del_biases = get_by_index(&net->del_biases, layer_index);
        
        if(layer_index != net->layer_count-2)
        {
            matrix weights = get_by_index(&net->weights, layer_index+1);
            
            // NOTE(lubo): Transpose magic transform so we don't need to transpose non-vector matrices
            // multiply(delta, transpose(weights), delta);
            //matrix new_delta = transpose_vector(transpose_vector(delta) * weights);
            matrix new_delta = transpose_vector(del_biases);
            multiply(new_delta, transpose_vector(delta), weights);
            new_delta = transpose_vector(new_delta);
            reduce_precision(new_delta, &net->params.backprop_fpopts);
            //incinerate(delta);
            delta = new_delta;
        }

        if(layer_index != net->layer_count-2)
        {
            // TODO(lubo): MOP_DRELU so that we dont have to destroy the 'zs'?
            // NOTE(lubo): We're destroying zs here
            matrix d_activation = z;
            drelu(d_activation);
            //dsigmoid(d_activation);
            
            hadamard(delta, d_activation);
        }

        if(use_dropout && layer_index != net->layer_count - 2)
        {
            // TODO(lubo): Couldn't we just multiply by inverted dropout here?
            matrix dropout = get_by_index(&net->dropouts, layer_index);
            hadamard(delta, dropout);
        }
        
        //copy_matrix(del_biases, delta);
        multiply(del_weights, delta, transpose_vector(activation));
        reduce_precision(del_weights, &net->params.backprop_fpopts);
    }
    
    //incinerate(delta);
}

// internal void backprop(Neural_Network *net, b32x use_dropout)
// {
//     matrix delta = calculate_delta(net);
//     backprop(net, delta, use_dropout);
// }

// // NOTE(lubo): Calculates gradient, does not update weights
// internal void forward_and_backprop(Neural_Network *net)
// {
//     feedforward(net);

//     // matrix delta = calculate_delta(net);
//     // backprop(net, delta);

//     backprop(net);
// }

// internal void apply_immediate_update(Neural_Network *net)
// {
//     for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
//     {
//         matrix weights = get_by_index(&net->weights, layer_index);
//         matrix biases = get_by_index(&net->biases, layer_index);
//         matrix del_weights = get_by_index(&net->del_weights, layer_index);
//         matrix del_biases = get_by_index(&net->del_biases, layer_index);
        
//         multiply(del_weights, -net->learning_rate);
//         multiply(del_biases, -net->learning_rate);
//         add(weights, del_weights);
//         add(biases, del_biases);
//     }
// }

// NOTE(lubo): Accumulate changes in minibatch to avg_del_weights
internal void accumulate_update(Neural_Network *net, int minibatch_size)
{
    for(int layer_index=net->layer_count-2; layer_index>=0; --layer_index)
    {
        matrix del_biases = get_by_index(&net->del_biases, layer_index);
        matrix del_weights = get_by_index(&net->del_weights, layer_index);
        matrix avg_del_biases = get_by_index(&net->avg_del_biases, layer_index);
        matrix avg_del_weights = get_by_index(&net->avg_del_weights, layer_index);
                    
        multiply(del_biases, 1/(double)minibatch_size);
        reduce_precision(del_biases, &net->params.update_fpopts);
        multiply(del_weights, 1/(double)minibatch_size);
        reduce_precision(del_weights, &net->params.update_fpopts);
        add(avg_del_biases, del_biases);
        reduce_precision(avg_del_biases, &net->params.update_fpopts);
        add(avg_del_weights, del_weights);
        reduce_precision(avg_del_weights, &net->params.update_fpopts);
    }
}

// NOTE(lubo): Apply changes from minibatch
internal void apply_updates(Neural_Network *net)
{
    for(int layer_index=net->layer_count-2; layer_index>=0; --layer_index)
    {
        matrix biases = get_by_index(&net->biases, layer_index);
        matrix weights = get_by_index(&net->weights, layer_index);
        matrix vel_biases = get_by_index(&net->vel_biases, layer_index);
        matrix vel_weights = get_by_index(&net->vel_weights, layer_index);
        matrix avg_del_biases = get_by_index(&net->avg_del_biases, layer_index);
        matrix avg_del_weights = get_by_index(&net->avg_del_weights, layer_index);

        if(net->params.weight_decay > 0/* && net->size_of_training_set > 0*/)
        {
            //double decay_step = net->learning_rate*(net->weight_decay/net->size_of_training_set);
            double decay_step = net->params.learning_rate*net->params.weight_decay;
            decay_l2(weights, decay_step);
            reduce_precision(weights, &net->params.update_fpopts);
            //decay_l1(weights, decay_step);
        }
                
        multiply(avg_del_biases, -net->params.learning_rate);
        reduce_precision(avg_del_biases, &net->params.update_fpopts);
        multiply(avg_del_weights, -net->params.learning_rate);
        reduce_precision(avg_del_weights, &net->params.update_fpopts);

        if(net->params.momentum_coefficient)
        {
            multiply(vel_biases, net->params.momentum_coefficient);
            reduce_precision(vel_biases, &net->params.update_fpopts);
            multiply(vel_weights, net->params.momentum_coefficient);
            reduce_precision(vel_weights, &net->params.update_fpopts);
            add(vel_biases, avg_del_biases);
            reduce_precision(vel_biases, &net->params.update_fpopts);
            add(vel_weights, avg_del_weights);
            reduce_precision(vel_weights, &net->params.update_fpopts);
            add(biases, vel_biases);
            reduce_precision(biases, &net->params.update_fpopts);
            add(weights, vel_weights);
            reduce_precision(weights, &net->params.update_fpopts);
        }
        else
        {
            add(biases, avg_del_biases);
            reduce_precision(biases, &net->params.update_fpopts);
            add(weights, avg_del_weights);
            reduce_precision(weights, &net->params.update_fpopts);
        }
        
        clear(avg_del_biases);
        clear(avg_del_weights);
    }
}

// NOTE(lubo): Mnist 1 hidden 30 neuron vanilla net 96.39% correctly classified.
internal int train_idx(Neural_Network *net,
                       LK_IDX_File train_set, LK_IDX_File train_labels,
                       LK_IDX_File test_set, LK_IDX_File test_labels,
                       int epochs=30, int minibatch_size=8)
{
    int batch_count = train_set.elements/minibatch_size;

    int last_correct_classifications = 0;
    int best_epoch_duration = 0;
    int best_correct_classifications = 0;
    
    for(int epoch=0; epoch<epochs; ++epoch)
    {
        for(int batch_index=0; batch_index<batch_count; ++batch_index)
        {
            double avg_batch_cost = 0;
            
            // for(int layer_index=net->layer_count-2; layer_index>=0; --layer_index)
            // {
            //     matrix avg_del_biases = get_by_index(&net->avg_del_biases, layer_index);
            //     matrix avg_del_weights = get_by_index(&net->avg_del_weights, layer_index);
                
            //     clear(avg_del_biases);
            //     clear(avg_del_weights);
            // }

            b32x print_this_batch = (batch_index % 1024) == 0;
            
            for(int minibatch_index=0; minibatch_index<minibatch_size; ++minibatch_index)
            {
                int element_id = batch_index*minibatch_size + minibatch_index;

                // NOTE(lubo): Evaluate input
                u8 *byte_data = (u8 *)idx_get_element(train_set, element_id);
                net->evaluate_byte_array(byte_data);

                // NOTE(lubo): Learn correct output
                int correct_class = *(u8 *)idx_get_element(train_labels, element_id);
                net->learn_onehot(correct_class, minibatch_size);
                
                // NOTE(lubo): Print
                if(print_this_batch)
                {
                    double current_cost = calculate_cost(net);
                    double cost_influence = 1/(double)(minibatch_index+1);
                    avg_batch_cost = (1-cost_influence)*avg_batch_cost + cost_influence*current_cost;
                }
            }

            if(print_this_batch)
            {
                #ifdef PY_SSIZE_T_CLEAN
                PySys_WriteStdout("Epoch %d/%d Batch %d/%d Cost %f LR %f\n", epoch, epochs, batch_index, batch_count, avg_batch_cost, net->params.learning_rate);
                #endif
                loginfo("neural_net", "Epoch %d/%d Batch %d/%d Cost %f LR %f", epoch, epochs, batch_index, batch_count, avg_batch_cost, net->params.learning_rate);
            }
        }

        int correct_classifications = 0;
        for(int test_index=0; test_index<test_set.elements; ++test_index)
        {
            int element_id = test_index;

            // NOTE(lubo): Evaluate testing input
            u8 *byte_data = (u8 *)idx_get_element(test_set, element_id);
            matrix prediction = net->evaluate_byte_array(byte_data);
            
            // NOTE(lubo): Check if we predicted the correct label
            int correct_label = *(u8 *)idx_get_element(test_labels, element_id);
            if(argmax(prediction) == correct_label)
            {
                correct_classifications += 1;
            }
        }

        if(correct_classifications > best_correct_classifications)
        {
            best_correct_classifications = correct_classifications;
            best_epoch_duration = 0;
        }
        else
        {
            best_epoch_duration += 1;
        }

        last_correct_classifications = correct_classifications;

        #ifdef PY_SSIZE_T_CLEAN
        {
            char buffer[1024];
            stbsp_snprintf(
                buffer, ArrayCount(buffer),
                "Epoch %d/%d Correct %d/%d Best %d(%d)", epoch, epochs, correct_classifications, test_set.elements, best_correct_classifications, best_epoch_duration);
            PySys_WriteStdout(buffer);
        }
        #endif
        
        loginfo("neural_net", "Epoch %d/%d Correct %d/%d Best %d(%d)", epoch, epochs, correct_classifications, test_set.elements, best_correct_classifications, best_epoch_duration);

        if(last_correct_classifications < 2000)
        {
            loginfo("neural_net", "Early stopping due to poor performance");
            #ifdef PY_SSIZE_T_CLEAN
            PySys_WriteStdout("Early stopping due to poor performance\n");
            #endif
            return last_correct_classifications;
        }
    }

    return last_correct_classifications;
}

internal void write_matrix_to_file(FILE *file, matrix A)
{
    Assert(A.allocation.size == sizeof(double)*A.height*A.width*A.layers*A.features);
    
    fwrite(&A.height, 1, sizeof(A.height), file);
    fwrite(&A.width, 1, sizeof(A.width), file);
    fwrite(&A.layers, 1, sizeof(A.layers), file);
    fwrite(&A.features, 1, sizeof(A.features), file);
    
    fwrite(A.allocation.memory, 1, A.allocation.size, file);
}

internal matrix read_matrix_from_file(FILE *file)
{
    matrix A = {};
    fread(&A.height, 1, sizeof(A.height), file);
    fread(&A.width, 1, sizeof(A.width), file);
    fread(&A.layers, 1, sizeof(A.layers), file);
    fread(&A.features, 1, sizeof(A.features), file);

    A = create_matrix(A);

    fread(A.allocation.memory, 1, A.allocation.size, file);
    
    return A;
}

internal void write_matrix_list_to_file(FILE *file, list(matrix) *L)
{
    fwrite(&L->count, 1, sizeof(L->count), file);
    foreach_by_value(matrix A, L)
    {
        write_matrix_to_file(file, A);
    }
}

internal void read_matrix_list_from_file(FILE *file, list(matrix) *L)
{
    int count;
    fread(&count, 1, sizeof(count), file);

    foreach_by_value(matrix A, L)
    {
        incinerate(A);
    }
    
    clear_list(L);
    int first_index = alloc_on_list(L, count);

    for(int index=0; index<count; ++index)
    {
        matrix *write_to = get_pointer_by_index(L, first_index + index);
        *write_to = read_matrix_from_file(file);
    }
}

#define VERSION_NAME "RPNNv0.0"
internal void save_network(Neural_Network *net, char *filename)
{
    FILE *file = fopen(filename, "wb");

    fwrite(VERSION_NAME, 1, sizeof(VERSION_NAME)-1, file);

    fwrite(&net->params, 1, sizeof(net->params), file);

    fwrite(&net->last_layer_softmax, 1, sizeof(net->last_layer_softmax), file);

    write_matrix_to_file(file, net->expected_output);

    write_matrix_list_to_file(file, &net->activations);
    write_matrix_list_to_file(file, &net->zs);
    write_matrix_list_to_file(file, &net->dropouts);
    write_matrix_list_to_file(file, &net->weights);
    write_matrix_list_to_file(file, &net->biases);
    write_matrix_list_to_file(file, &net->del_weights);
    write_matrix_list_to_file(file, &net->del_biases);
    write_matrix_list_to_file(file, &net->avg_del_weights);
    write_matrix_list_to_file(file, &net->avg_del_biases);
    write_matrix_list_to_file(file, &net->vel_weights);
    write_matrix_list_to_file(file, &net->vel_biases);
    
    fclose(file);
}

internal b32x load_network(Neural_Network *net, char *filename)
{
    FILE *file = fopen(filename, "rb");

    b32x success = false;
    if(file)
    {
        success = true;
        char save_version[sizeof(VERSION_NAME)-1];
        fread(save_version, 1, sizeof(save_version), file);
        Assert(memcmp(save_version, VERSION_NAME, sizeof(save_version)) == 0);

        fread(&net->params, 1, sizeof(net->params), file);

        fread(&net->last_layer_softmax, 1, sizeof(net->last_layer_softmax), file);

        net->expected_output = read_matrix_from_file(file);

        read_matrix_list_from_file(file, &net->activations);
        read_matrix_list_from_file(file, &net->zs);
        read_matrix_list_from_file(file, &net->dropouts);
        read_matrix_list_from_file(file, &net->weights);
        read_matrix_list_from_file(file, &net->biases);
        read_matrix_list_from_file(file, &net->del_weights);
        read_matrix_list_from_file(file, &net->del_biases);
        read_matrix_list_from_file(file, &net->avg_del_weights);
        read_matrix_list_from_file(file, &net->avg_del_biases);
        read_matrix_list_from_file(file, &net->vel_weights);
        read_matrix_list_from_file(file, &net->vel_biases);
        
        fclose(file);
    }

    return success;
}

internal int save_matrix_as_image_to_file(matrix A, char const *filename)
{
    Assert(A.layers == 1);
    Assert(A.features == 1);
    
    double *A_values = (double *)A.allocation.memory;
    
    u8 *data = (u8 *)malloc(A.height * A.width * sizeof(u8));
    u8 *at = data;

    for(int i=0; i<A.height; ++i)
    {
        for(int j=0; j<A.width; ++j)
        {
            *at = clamp_to_u8(255*inv_lerp(-1, 1, *A_values));
            
            ++A_values;
            ++at;
        }
    }

    int success = stbi_write_png(filename, A.width, A.height, 1, data, 0);

    free(data);
    
    return success;
}

internal void save_network_weights_images(Neural_Network *net, int directory_number)
{
    char filename[256];
    sprintf(filename, "mkdir network_images\\%d", directory_number);

    system(filename);
    
    for(int i=0; i<net->weights.count; ++i)
    {
        sprintf(filename, "network_images\\%d\\weights_%d.png", directory_number, i);
        matrix A = get_by_index(&net->weights, i);
        save_matrix_as_image_to_file(A, filename);
    }

    for(int i=0; i<net->biases.count; ++i)
    {
        sprintf(filename, "network_images\\%d\\biases_%d.png", directory_number, i);
        matrix A = get_by_index(&net->biases, i);
        save_matrix_as_image_to_file(A, filename);
    }
}


// NOTE(lubo): Given the matrix expected_R, calculate A and B such that A*B ~= expected_R
internal float64 matrix_factorization(Neural_Network_Hyperparams *params, matrix expected_R, int features, int epochs, b32x non_negative)
{
    matrix A = create_matrix(expected_R.height, features);
    matrix B = create_matrix(features, expected_R.width);

    matrix A_grad = create_matrix(A);
    matrix B_grad = create_matrix(B);

    matrix AT = create_matrix(A.width, A.height);
    matrix BT = create_matrix(B.width, B.height);
    
    matrix R = create_matrix_with_dimension_of_product(A, B);
    Assert(R.width == expected_R.width);
    Assert(R.height == expected_R.height);
    
    randomize_values(A);
    randomize_values(B);

    if(non_negative)
    {
        absolute_value(A);
        absolute_value(B);
    }
    
    for(int epoch=0; epoch<epochs; ++epoch)
    {
        multiply(R, A, B);
        reduce_precision(R, &params->forward_fpopts);

        if((epoch % 1024) == 0)
        {
            printf("Cost: %lf\n", cost(R, expected_R));
        }
        
        dcost(R, R, expected_R);
        reduce_precision(R, &params->backprop_fpopts);
        
        transpose(AT, A);
        transpose(BT, B);

        multiply(A_grad, R, BT);
        multiply(B_grad, AT, R);
        reduce_precision(A_grad, &params->backprop_fpopts);
        reduce_precision(B_grad, &params->backprop_fpopts);

        multiply(A_grad, -params->learning_rate);
        multiply(B_grad, -params->learning_rate);
        reduce_precision(A_grad, &params->update_fpopts);
        reduce_precision(B_grad, &params->update_fpopts);

        add(A, A_grad);
        add(B, B_grad);
        reduce_precision(A, &params->update_fpopts);
        reduce_precision(B, &params->update_fpopts);

        // NOTE(lubo): We could just call relu, but wouldn't that be confusing?
        if(non_negative)
        {
            scalar_op(A, A, MOP_MAX, 0);
            scalar_op(B, B, MOP_MAX, 0);
        }
    }

    multiply(R, A, B);
    reduce_precision(R, &params->forward_fpopts);

    #if false
    print_matrix(A);
    printf("\n");
    print_matrix(B);
    printf("\n");
    print_matrix(R);
    printf("\n");
    print_matrix(expected_R);
    printf("\n");
    #endif

    incinerate(A);
    incinerate(B);
    incinerate(A_grad);
    incinerate(B_grad);
    incinerate(AT);
    incinerate(BT);
    incinerate(R);
    
    float64 final_cost = cost(R, expected_R);
    printf("Final cost: %lf\n", final_cost);
    return final_cost;
}

void Neural_Network::evaluate_input()
{
    feedforward(this);
    //matrix output = get_by_index(&this->activations, this->layer_count-1);
    //return output;
}

void Neural_Network::learn_from_expected_output(int minibatch_size)
{
    matrix delta = calculate_delta(this);
    backprop(this, delta);
    
    accumulate_update(this, minibatch_size);
    this->minibatch_count += 1;

    if(this->minibatch_count >= minibatch_size)
    {
        apply_updates(this);
        this->minibatch_count = 0;
    }
}
