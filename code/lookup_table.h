/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2021 All Rights Reserved. $
   ======================================================================== */

class Lookup_Table : public Function_Approximator
{
public:
    int *buckets;
    matrix ranges;
    
    buffer allocation;

    double lerp_value;

    matrix evaluate_input();
    void learn_from_expected_output(int minibatch_size);
};

internal Lookup_Table create_lookup_table(int *buckets, matrix ranges, int output_count)
{
    Lookup_Table table = {};

    int inputs_count = ranges.height;
    
    int buckets_array_size = sizeof(int)*inputs_count;
    table.buckets = (int *)malloc(buckets_array_size);
    memcpy(table.buckets, buckets, buckets_array_size);
    
    table.input = create_matrix(inputs_count, 1);
    table.prediction = create_matrix(output_count, 1);
    table.expected_output = create_matrix(output_count, 1);
    table.ranges = copy_matrix(ranges);

    s64 total_buckets = 1;
    for(int i=0; i<inputs_count; ++i)
    {
        total_buckets *= table.buckets[i];
    }
    
    table.allocation = general_allocate_array(double, total_buckets*output_count);
    memset(table.allocation.memory, 0, table.allocation.size);

    return table;
}

internal void incinerate(Lookup_Table *table)
{
    incinerate(table->input);
    incinerate(table->prediction);
    incinerate(table->expected_output);
    incinerate(table->ranges);
    
    free(table->buckets);

    general_deallocate(table->allocation);
}

inline int lookup_table_select_bucket(int buckets, double min, double max, double value)
{
    double bucket_cont = buckets*inv_lerp(min, max, value);
    int bucket_index = (int)(bucket_cont);
    bucket_index = Clamp(bucket_index, 0, buckets - 1);
    return bucket_index;
}

internal double *lookup_table_access(const Lookup_Table *table, const double *input)
{
    int input_count = table->ranges.height;
    int output_count = table->prediction.height;
    
    double *base = (double *)table->allocation.memory;

    smm multiplier = 1;
    smm index = 0;
    for(int i=0; i<table->ranges.height; ++i)
    {
        int bucket = lookup_table_select_bucket(
            table->buckets[i],
            matrix_get(table->ranges, i, 0),
            matrix_get(table->ranges, i, 1),
            input[i]);

        index += bucket*multiplier;
        multiplier *= table->buckets[i];
    }

    double *result = &base[output_count*index];
    return result;
}

// internal matrix evaluate(const Lookup_Table *table, const double *input)
// {
//     int output_count = table->prediction.height;
//     double *source = lookup_table_access(table, input);
//     memcpy(table->prediction.allocation.memory, source, sizeof(double)*output_count);

//     return table->prediction;
// }

internal void set_table_value(const Lookup_Table *table, const double *input, matrix output)
{
    int output_count = table->prediction.height;
    
    double *dest = lookup_table_access(table, input);
    memcpy(dest, output.allocation.memory, output.allocation.size);
}

internal void set_table_value(const Lookup_Table *table, const double *input, const double *output)
{
    int output_count = table->prediction.height;
    
    double *dest = lookup_table_access(table, input);
    memcpy(dest, output, sizeof(double)*output_count);
}

internal void save_lookup_table(const Lookup_Table *table, char *filename)
{
    FILE *file = fopen(filename, "wb");

    int input_count = table->ranges.height;
    int output_count = table->prediction.height;
    
    write_matrix_to_file(file, table->ranges);

    fwrite(&output_count, 1, sizeof(output_count), file);
    
    fwrite(table->buckets, 1, input_count*sizeof(table->buckets[0]), file);

    fwrite(table->allocation.memory, 1, table->allocation.size, file);
    
    fclose(file);
}

internal Lookup_Table load_lookup_table(char *filename)
{
    Lookup_Table table = {};
    
    FILE *file = fopen(filename, "rb");
    if(file)
    {
        matrix ranges = read_matrix_from_file(file);
        int input_count = ranges.height;
        int output_count;
        fread(&output_count, 1, sizeof(output_count), file);

        int buckets_array_size = sizeof(int)*input_count;
        int *buckets = (int *)malloc(buckets_array_size);
        fread(buckets, 1, buckets_array_size, file);
    
        table = create_lookup_table(buckets, ranges, output_count);

        fread(table.allocation.memory, 1, table.allocation.size, file);
    
        fclose(file);

        incinerate(ranges);
        free(buckets);
    }

    return table;
}

matrix Lookup_Table::evaluate_input()
{
    double *source = lookup_table_access(this, (double *)this->input.allocation.memory);
    memcpy(this->prediction.allocation.memory, source, this->prediction.allocation.size);

    return this->prediction;
}

void Lookup_Table::learn_from_expected_output(int minibatch_size)
{
    int output_count = this->prediction.height;
    double *dest = lookup_table_access(this, (double *)this->input.allocation.memory);
    double *source = (double *)this->expected_output.allocation.memory;
    
    if(this->lerp_value == 0)
    {
        memcpy(dest, source, this->expected_output.allocation.size);
    }
    else
    {
        for(int i=0; i<output_count; ++i)
        {
            dest[i] = lerp(source[i], dest[i], lerp_value);
        }
    }
}
