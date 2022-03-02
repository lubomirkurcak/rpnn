/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

internal Neural_Network
create_feedforward_net(Neural_Network_Hyperparams *params, int layer_count, int *layer_sizes)
{
    Neural_Network net = {};

    net.params = params;
    net.layer_count = layer_count;
    //net.layer_sizes = layer_sizes;
    
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
        randomize_values(&net.params->random_series, weights);
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
    auto series = &net->params->random_series;
    
    foreach_by_value(matrix w, &net->weights)
    {
        randomize_values(series, w);
    }

    foreach_by_value(matrix b, &net->biases)
    {
        clear(b);
    }
}

// IMPORTANT NOTE/TODO(lubo): Last layer is linear by default
internal void feedforward(Neural_Network *net, b32x use_dropout=false)
{
    for(int layer_index=0; layer_index<net->layer_count-1; ++layer_index)
    {
        matrix previous_layer = get_by_index(&net->activations, layer_index);
        matrix current_layer = get_by_index(&net->activations, layer_index+1);
        matrix z = get_by_index(&net->zs, layer_index);
        matrix weights = get_by_index(&net->weights, layer_index);
        matrix biases = get_by_index(&net->biases, layer_index);
        
        multiply(z, weights, previous_layer);
        reduce_precision(z, &net->params->forward_fpopts);
        add(z, biases);
        reduce_precision(z, &net->params->forward_fpopts);
        
        if(layer_index == net->layer_count-2)
        {
            if(net->last_layer_softmax)
            {
                softmax(current_layer, z);
                reduce_precision(z, &net->params->forward_fpopts);
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
        
        if(use_dropout && layer_index != net->layer_count - 2)
        {
            matrix dropout = get_by_index(&net->dropouts, layer_index);
            hadamard(current_layer, dropout);
        }
    }
}

internal double calculate_cost(Neural_Network *net)
{
    double base_cost = cost(net->prediction, net->expected_output);

    double weights_cost = 0;
    if(net->params->weight_decay > 0/* && net->size_of_training_set > 0*/)
    {
        for(int layer_index=net->layer_count-2; layer_index>=0; --layer_index)
        {
            matrix weights = get_by_index(&net->del_weights, layer_index);
            //weights_cost += net->weight_decay/(2*net->size_of_training_set)*matrix_l2(weights);
            weights_cost += net->params->weight_decay*matrix_l2(weights);
        }
    }

    double result = base_cost + weights_cost;
    return result;
}

internal void create_dropouts(Neural_Network *net)
{
    auto series = &net->params->random_series;
    double inv_p = 1/(1-net->params->dropout);
    
    double dropout_chance = net->params->dropout;
    for(int layer_index=0; layer_index<net->layer_count-2; ++layer_index)
    {
        matrix dropout = get_by_index(&net->dropouts, layer_index);
        double *values = (double *)dropout.allocation.memory;

        for(int g=0; g<dropout.features; ++g)
        {
            for(int h=0; h<dropout.layers; ++h)
            {
                for(int i=0; i<dropout.height; ++i)
                {
                    for(int j=0; j<dropout.width; ++j)
                    {
                        *values = probability((float)dropout_chance, series) ? inv_p : 0.0;
            
                        ++values;
                    }
                }
            }
        }
    }
}

// NOTE(lubo): Calculates gradient, does not update weights
internal void backprop(Neural_Network *net)
{
    b32x use_dropout = net->params->dropout > 0;
    if(use_dropout)
    {
        create_dropouts(net);
    }

    feedforward(net, use_dropout);
    
    matrix output = get_by_index(&net->activations, net->layer_count-1);
    
    matrix delta = create_matrix(output);
    dcost(delta, output, net->expected_output);
    reduce_precision(delta, &net->params->backprop_fpopts);

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
            // multiply(delta, transpose(weights3), delta);
            matrix new_delta = transpose_vector(transpose_vector(delta) * weights);
            reduce_precision(new_delta, &net->params->backprop_fpopts);
            incinerate(delta);
            delta = new_delta;
        }

        if(layer_index != net->layer_count-2)
        {
            // NOTE(lubo): We're destroying zs here
            matrix d_activation = z;
            drelu(d_activation);
            //dsigmoid(d_activation);
            
            hadamard(delta, d_activation);
        }

        if(use_dropout && layer_index != net->layer_count-2)
        {
            matrix dropout = get_by_index(&net->dropouts, layer_index);
            hadamard(delta, dropout);

            // double multiplier = 1/(1-net->dropout);
            // multiply(delta, multiplier);
        }
        
        copy_matrix(del_biases, delta);
        multiply(del_weights, delta, transpose_vector(activation));
        reduce_precision(del_weights, &net->params->backprop_fpopts);
    }
    
    incinerate(delta);
}

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
        reduce_precision(del_biases, &net->params->update_fpopts);
        multiply(del_weights, 1/(double)minibatch_size);
        reduce_precision(del_weights, &net->params->update_fpopts);
        add(avg_del_biases, del_biases);
        reduce_precision(avg_del_biases, &net->params->update_fpopts);
        add(avg_del_weights, del_weights);
        reduce_precision(avg_del_weights, &net->params->update_fpopts);
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

        if(net->params->weight_decay > 0/* && net->size_of_training_set > 0*/)
        {
            //double decay_step = net->learning_rate*(net->weight_decay/net->size_of_training_set);
            double decay_step = net->params->learning_rate*net->params->weight_decay;
            decay_l2(weights, decay_step);
            reduce_precision(weights, &net->params->update_fpopts);
            //decay_l1(weights, decay_step);
        }
                
        multiply(avg_del_biases, -net->params->learning_rate);
        reduce_precision(avg_del_biases, &net->params->update_fpopts);
        multiply(avg_del_weights, -net->params->learning_rate);
        reduce_precision(avg_del_weights, &net->params->update_fpopts);

        if(net->params->momentum_coefficient)
        {
            multiply(vel_biases, net->params->momentum_coefficient);
            reduce_precision(vel_biases, &net->params->update_fpopts);
            multiply(vel_weights, net->params->momentum_coefficient);
            reduce_precision(vel_weights, &net->params->update_fpopts);
            add(vel_biases, avg_del_biases);
            reduce_precision(vel_biases, &net->params->update_fpopts);
            add(vel_weights, avg_del_weights);
            reduce_precision(vel_weights, &net->params->update_fpopts);
            add(biases, vel_biases);
            reduce_precision(biases, &net->params->update_fpopts);
            add(weights, vel_weights);
            reduce_precision(weights, &net->params->update_fpopts);
        }
        else
        {
            add(biases, avg_del_biases);
            reduce_precision(biases, &net->params->update_fpopts);
            add(weights, avg_del_weights);
            reduce_precision(weights, &net->params->update_fpopts);
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
        
                // NOTE(lubo): Load training class labels
                void *label_data = idx_get_element(train_labels, element_id);
                int class_label = *(u8 *)label_data;
                clear(net->expected_output);
                *(double *)matrix_access(net->expected_output, class_label) = 1;

                // NOTE(lubo): Load training input
                matrix input = net->input;
                void *element_data = idx_get_element(train_set, element_id);
                u8 *read = (u8 *)element_data;
                double *write = (double *)input.allocation.memory;
                for(int value_index=0; value_index<train_set.element_size; ++value_index)
                {
                    u32 value = *read;
                    *write = (double)value/(double)255;

                    read++;
                    write++;
                }
                reduce_precision(input, &net->params->forward_fpopts);

                // NOTE(lubo): Backprop
                backprop(net);
                accumulate_update(net, minibatch_size);
                
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
                PySys_WriteStdout("Epoch %d/%d Batch %d/%d Cost %f LR %f\n", epoch, epochs, batch_index, batch_count, avg_batch_cost, net->params->learning_rate);
                #endif
                loginfo("neural_net", "Epoch %d/%d Batch %d/%d Cost %f LR %f", epoch, epochs, batch_index, batch_count, avg_batch_cost, net->params->learning_rate);
            }

            apply_updates(net);
        }

        int correct_classifications = 0;
        for(int test_index=0; test_index<test_set.elements; ++test_index)
        {
            int element_id = test_index;
        
            // NOTE(lubo): Load testing class labels
            void *label_data = idx_get_element(test_labels, element_id);
            int class_label = *(u8 *)label_data;
            clear(net->expected_output);
            *(double *)matrix_access(net->expected_output, class_label) = 1;

            // NOTE(lubo): Load testing input
            matrix input = net->input;
            void *element_data = idx_get_element(test_set, element_id);
            u8 *read = (u8 *)element_data;
            double *write = (double *)input.allocation.memory;
            for(int value_index=0; value_index<test_set.element_size; ++value_index)
            {
                u32 value = *read;
                *write = (double)value/(double)255;

                read++;
                write++;
            }
            reduce_precision(input, &net->params->forward_fpopts);

            feedforward(net);
            
            correct_classifications += class_label == argmax(net->prediction);
        }

        if(correct_classifications > best_correct_classifications)
        {
            best_correct_classifications = correct_classifications;
            best_epoch_duration = 0;
        }
        else
        {
            best_epoch_duration += 1;

            // if(best_epoch_duration >= 10)
            // {
            //     net->learning_rate *= 0.7f;
            //     if(net->learning_rate <= 0.00001f)
            //     {
            //         loginfo("neural_net", "DONE Epoch %d/%d Correct %d/%d", epoch, epochs, correct_classifications, net->test_set.elements);
            //         break;
            //     }
            // }
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

internal void save(Neural_Network *net)
{
    NotImplemented;
}

internal void load(Neural_Network *net)
{
    NotImplemented;
}

// NOTE(lubo): Given the matrix expected_R, calculate A and B such that A*B ~= expected_R
float64 matrix_factorization(Neural_Network_Hyperparams *params, matrix expected_R, int features, int epochs, b32x non_negative)
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
    
    randomize_values(&params->random_series, A);
    randomize_values(&params->random_series, B);

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

    float64 final_cost = cost(R, expected_R);
    printf("Final cost: %lf\n", final_cost);
    return final_cost;
}
