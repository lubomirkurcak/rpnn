/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

class Function_Approximator
{
public:
    matrix input;
    matrix prediction;
    matrix expected_output;
    int minibatch_count;
                     
    virtual matrix evaluate(matrix input) final
    {
        copy_matrix(this->input, input);
        matrix result = evaluate_input();
        copy_matrix(this->expected_output, result);
        return result;
    }
    virtual matrix evaluate(double value)
    {
        Assert(this->input.allocation.size == sizeof(value));
        memcpy(this->input.allocation.memory, &value, this->input.allocation.size);
        matrix result = evaluate_input();
        copy_matrix(this->expected_output, result);
        return result;
    }
    virtual matrix evaluate(double *input) final
    {
        memcpy(this->input.allocation.memory, input, this->input.allocation.size);
        matrix result = evaluate_input();
        copy_matrix(this->expected_output, result);
        return result;
    }
    // NOTE(lubo): maps [0, 255] -> [0.0, 1.0]
    virtual matrix evaluate_byte_array(u8 *byte_data)
    {
        u8 *read = byte_data;
        double *write = (double *)this->input.allocation.memory;
        double *write_stop = (double *)((u8 *)this->input.allocation.memory + this->input.allocation.size);
        while(write < write_stop)
        {
            u32 value = *read;
            *write = (double)value/(double)255;

            read++;
            write++;
        }
        matrix result = evaluate_input();
        copy_matrix(this->expected_output, result);
        return result;
    }

    virtual void learn(matrix expected, int minibatch_size) final
    {
        copy_matrix(this->expected_output, expected);
        learn_from_expected_output(minibatch_size);
    }
    virtual void learn(double expected, int minibatch_size) final
    {
        Assert(this->expected_output.allocation.size == sizeof(expected));
        memcpy(this->expected_output.allocation.memory, &expected, this->expected_output.allocation.size);
        learn_from_expected_output(minibatch_size);
    }
    virtual void learn(double *expected, int minibatch_size) final
    {
        memcpy(this->expected_output.allocation.memory, expected, this->expected_output.allocation.size);
        learn_from_expected_output(minibatch_size);
    }
    virtual void learn_onehot(int class_index, int minibatch_size) final
    {
        clear(this->expected_output);
        matrix_set(this->expected_output, class_index, 1);
        learn_from_expected_output(minibatch_size);
    }

private:    
    // NOTE(lubo): 'evaluate_input' must fill 'this->prediction', based on 'this->input'
    virtual matrix evaluate_input() = 0;

    // NOTE(lubo): learn based on the difference between 'this->prediction' calculated by 'evaluate'
    // and the 'this->expected_output', set by the user.
    virtual void learn_from_expected_output(int minibatch_size) = 0;
    
};
