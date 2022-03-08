/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Lubomir Kurcak $
   $Notice: (C) Copyright 2020 All Rights Reserved. $
   ======================================================================== */

/*

  - Documentation -


  First include this file in your code like this:

      #include "dynamic_list_template.h"

  Then, define list types for any amount of types like this:

      DefineDynamicListType(int);
      DefineDynamicListType(float);
      DefineDynamicListType(vector2);
      DefineDynamicListType(player);
                     ...


  Later create and use lists like this:

      list(int) numbers;
      append_to_list(&numbers, 1);
      append_to_list(&numbers, 2);
      append_to_list(&numbers, 3);
      int first = get_by_index(&numbers, 0);
      int *ptr_to_first = get_pointer_by_index(&numbers, 0);
      if(is_in_list(&numbers, 2))
      {
          clear_list(&numbers);
      }
      foreach_by_value(int i, &numbers)
      {
          print(i);
      }

  - Extra Functions -

   #define DL_EQUALITY

      - Used when searching by value

   #define DL_COMPARISON

      - Used for ordered lists, etc
      - Implies equality is defined


   - Todo -

   Enable sort/aggregate functions, but that wasn't needed yet.
                     
 */

#if defined(DL_COMPARISON) && !defined(DL_EQUALITY)
#define DL_EQUALITY
#endif

#ifndef foreach
#define foreach(i, A) for(int ___KEEP=1, item_index=0, ___SIZE=__internal_get_list_length(A); \
                                 ___KEEP && item_index != ___SIZE;      \
                                 ___KEEP = !___KEEP, ++item_index)      \
        for(i = __internal_get_values(A)+item_index; ___KEEP; ___KEEP = !___KEEP)

#define foreach_by_value(i, A) for(int ___KEEP=1, item_index=0, ___SIZE=__internal_get_list_length(A); \
                                 ___KEEP && item_index != ___SIZE;      \
                                 ___KEEP = !___KEEP, ++item_index)      \
        for(i = __internal_get_values(A)[item_index]; ___KEEP; ___KEEP = !___KEEP)

#define foreach_starting_index(i, A, S) for(int ___KEEP=1, item_index=(S), ___SIZE=__internal_get_list_length(A); \
                                 ___KEEP && item_index != ___SIZE;      \
                                 ___KEEP = !___KEEP, ++item_index)      \
        for(i = __internal_get_values(A)+item_index; ___KEEP; ___KEEP = !___KEEP)

#define foreach_by_value_starting_index(i, A, S) for(int ___KEEP=1, item_index=(S), ___SIZE=__internal_get_list_length(A); \
                                 ___KEEP && item_index != ___SIZE;      \
                                 ___KEEP = !___KEEP, ++item_index)      \
        for(i = __internal_get_values(A)[item_index]; ___KEEP; ___KEEP = !___KEEP)
#endif

#ifndef list
#define list(Type, ...) dynamic_list_##Type
#endif

#ifndef DL_TYPE
#error Please define 'DL_TYPE' to a type before including "dynamic_list_template.h"
#endif

#define _DL_NAME(Type) list(Type)
#define DL_NAME _DL_NAME(DL_TYPE)

#pragma message("Creating '" STRINGIZE(DL_NAME) "'")

typedef struct
{
    int max_count;
    int count;
    DL_TYPE *values; // If you get an error here, your type is not defined
    buffer allocation;
} DL_NAME;

internal int __internal_get_list_length(DL_NAME L){return L.count;}
internal int __internal_get_list_length(DL_NAME *L){return L->count;}
internal DL_TYPE *__internal_get_values(DL_NAME L){return L.values;}
internal DL_TYPE *__internal_get_values(DL_NAME *L){return L->values;}

#if 0
internal void
free_list(DL_NAME *L)
{
    free(L->values);
}
#endif

internal void
clear_list(DL_NAME *L)
{
    L->count = 0;
}

internal void
hard_clear_list(DL_NAME *L)
{
    if(L->allocation.memory)
    {
        general_deallocate(L->allocation);
    }
    *L = {};
}

inline DL_TYPE *
get_pointer_by_index(DL_NAME *L, int Index)
{
    Assert(Index >= 0 && Index < L->count, STRINGIZE(DL_NAME));
    DL_TYPE *Result = L->values + Index;
    return Result;
}

inline DL_TYPE *
get_last_pointer(DL_NAME *L)
{
    DL_TYPE *Result = get_pointer_by_index(L, L->count-1);
    return Result;
}

inline DL_TYPE
get_by_index(DL_NAME *L, int Index)
{
    DL_TYPE Result = *get_pointer_by_index(L, Index);
    return Result;
}

inline DL_TYPE
get_last(DL_NAME *L)
{
    DL_TYPE Result = *get_last_pointer(L);
    return Result;
}

internal int
alloc_on_list(DL_NAME *L, int alloc_count)
{
    int count = L->count;
    int max_count = L->max_count;
    int free_slots = max_count - count;

    Assert(free_slots >= 0);

    if(free_slots < alloc_count)
    {
        while(free_slots < alloc_count)
        {        
            if(max_count) max_count *= 2;
            else max_count = 16;
            free_slots = max_count - count;
        }

//        L->values = ReallocArray(L->values, DL_TYPE, max_count);
        L->allocation = general_reallocate_array(L->allocation, DL_TYPE, max_count);
        L->values = (DL_TYPE *)L->allocation.memory;
        
        L->max_count = max_count;
    }

    void *debug_first_modified_byte = (u8 *)L->values + sizeof(DL_TYPE)*count;
    void *debug_last_modified_byte = (u8 *)L->values + sizeof(DL_TYPE)*(count+1) - 1;
    void *debug_first_array_byte = (u8 *)L->values;
    void *debug_last_array_byte = (u8 *)L->values + sizeof(DL_TYPE)*L->max_count - 1;
    Assert(debug_first_modified_byte >= debug_first_array_byte);
    Assert(debug_first_modified_byte <= debug_last_array_byte);
    Assert(debug_last_modified_byte >= debug_first_array_byte);
    Assert(debug_last_modified_byte <= debug_last_array_byte);
    
    int inserted_item_index = count;
    L->count += alloc_count;

    return inserted_item_index;
}

internal int
append_to_list(DL_NAME *L, DL_TYPE Value)
{
    int index = alloc_on_list(L, 1);
    DL_TYPE *inserted_item = get_pointer_by_index(L, index);
    *inserted_item = Value;
    return index;
}

internal DL_TYPE *
append_to_list(DL_NAME *L, DL_NAME *N)
{
    DL_TYPE *inserted_item = 0;

    if(N->count)
    {
        int index = alloc_on_list(L, N->count);
        inserted_item = get_pointer_by_index(L, index);

        block_copy(inserted_item, N->values, sizeof(DL_TYPE)*N->count);
    }
    
    return inserted_item;
}

internal DL_TYPE *
alloc_at_index(DL_NAME *L, int index)
{
    int last_index = alloc_on_list(L, 1);
    DL_TYPE *last = get_pointer_by_index(L, last_index);
    DL_TYPE *to_insert = get_pointer_by_index(L, index);

    *last = *to_insert;
    
    return to_insert;
}

internal void
insert_at_index(DL_NAME *L, int index, DL_TYPE Value)
{
    DL_TYPE *to_insert = alloc_at_index(L, index);
    *to_insert = Value;
}

internal void
set_at_index(DL_NAME *L, int index, DL_TYPE Value)
{
    *get_pointer_by_index(L, index) = Value;
}

internal void
remove_by_index_preserve_order(DL_NAME *L, int Index)
{
    Assert(Index >= 0 && Index < L->count);
    
    int count = L->count;
    int new_count = count - 1;
    int copy_count = count - Index - 1;
    DL_TYPE *Write = L->values + Index;
    DL_TYPE *Read = L->values + Index + 1;

    L->count = new_count;
    
    for(int copy_index=0; copy_index<copy_count; ++copy_index)
    {
        *Write++ = *Read++;
    }
}

internal void
remove_by_index(DL_NAME *L, int Index)
{
    DL_TYPE *to_delete = get_pointer_by_index(L, Index);
    DL_TYPE *last = get_last_pointer(L);

    *to_delete = *last;
    
    L->count -= 1;
}

inline DL_TYPE
pop_at_index(DL_NAME *L, int Index)
{
    DL_TYPE Result = get_by_index(L, Index);
    remove_by_index(L, Index);

    return Result;
}

internal void
reverse(DL_NAME *L)
{
    int halfway = L->count / 2;
    for(int i=0; i<halfway; ++i)
    {
        Swap(L->values[i], L->values[L->count-1 - i]);
    }
}

internal void
shuffle(DL_NAME *L, Random_Series *series=&global_default_random_series)
{
    int n = L->count;
    for(int i=0; i<n-1; ++i)
    {
        int j = get_random_int_between(i, n, series);
        Swap(L->values[i], L->values[j]);
    }
}

internal DL_TYPE *get_pointer_to_random(DL_NAME *L, Random_Series *series=&global_default_random_series)
{
    DL_TYPE *result = get_pointer_by_index(L, get_random_int_up_to(L->count, series));
    return result;
}

internal void remove_random(DL_NAME *L, Random_Series *series=&global_default_random_series)
{
    remove_by_index(L, get_random_int_up_to(L->count, series));
}

#ifdef DL_EQUALITY
internal int
get_index_of(DL_NAME *L, DL_TYPE Value)
{
    int Result = -1;
    for(int Index=0; Index<L->count; ++Index)
    {
        if(get_by_index(L, Index) == Value)
        {
            Result = Index;
            break;
        }
    }

    return Result;
}

internal b32x
is_in_list(DL_NAME *L, DL_TYPE Value)
{
    b32x Result = get_index_of(L, Value) != -1;
    return Result;
}

internal int
append_if_unique(DL_NAME *L, DL_TYPE Value)
{
    int index = get_index_of(L, Value);
    if(index == -1)
    {
        index = append_to_list(L, Value);
    }
    return index;
}

internal void
append_if_unique(DL_NAME *L, DL_NAME *N)
{
    foreach_by_value(DL_TYPE Value, N)
    {
        if(!is_in_list(L, Value))
        {
            append_to_list(L, Value);
        }
    }
}

internal b32x
remove_from_list(DL_NAME *L, DL_TYPE Value)
{
    b32x Success = false;
    int Index = get_index_of(L, Value);
    if(Index != -1)
    {
        Success = true;
        remove_by_index(L, Index);
    }
    return Success;
}

#endif

// TODO(lubomir): Make this optional
// TODO(lubomir): Maybe let users choose flags:
//   - Comparable - include comparison based sorts
//   - countable  - include counting sort
#ifdef DL_COMPARISON
internal DL_TYPE
find_maximum(DL_NAME *L)
{
    Assert(L->count > 0);
    
    DL_TYPE result = L->values[0];
    foreach_by_value_starting_index(DL_TYPE i, L, 1)
    {
        result = Max(result, i);
    }

    return result;
}

// NOTE(lubo): Quicksort algorithm by Sir Charles Antony Richard Hoare (pronounced whore?)
internal int
quicksort_partition(DL_NAME *A, int lo, int hi)
{
    DL_TYPE pivot = A->values[(hi + lo) / 2];
    int i = lo;
    int j = hi;
    while(1)
    {
        while(A->values[i] < pivot) i += 1;
        while(A->values[j] > pivot) j -= 1;
        if(i >= j) return j;
        Swap(A->values[i], A->values[j]);
    }
}

// TODO(lubo): Use insert sort for small arrays
internal void
quicksort(DL_NAME *A, int lo, int hi)
{
    if(lo < hi)
    {
        int pivot_index = quicksort_partition(A, lo, hi);
        quicksort(A, lo, pivot_index);
        quicksort(A, pivot_index + 1, hi);
    }
}

internal void
sort(DL_NAME *A)
{
    quicksort(A, 0, A->count-1);
}

#if 0
internal void
counting_sort(DL_NAME *L, DL_TYPE OnePastMax)
{
    u32 count = L->count;
    DL_TYPE *Array = L->values;
    
    DL_TYPE *Occurencies = AllocArray(DL_TYPE, OnePastMax);
    DL_TYPE *Sorted = AllocArray(DL_TYPE, count);

    ForI(Index, 0, OnePastMax)
    {
        Occurencies[Index] = 0;
    }
    
    ForI(Index, 0, count)
    {
        ++Occurencies[Array[Index]];
    }

    ForI(Index, 1, OnePastMax)
    {
        Occurencies[Index] += Occurencies[Index - 1];
    }
    
    ForIBackwards(Index, 0, count)
    {
        Sorted[--Occurencies[Array[Index]]] = Array[Index];
    }

    ForI(Index, 0, count)
    {
        Array[Index] = Sorted[Index];
    }

    FreeMemory(Sorted);
    FreeMemory(Occurencies);
}

enum dynamic_list_sort_type
{
    DynamicListSortType_Default,

    DynamicListSortType_CountingSort,
    DynamicListSortType_QuickSort,
};
internal void
sort_list(DL_NAME *L, dynamic_list_sort_type SortType=DynamicListSortType_Default)
{
    switch(SortType)
    {
        default:
        case ListSortType_CountingSort:
        {
            DL_TYPE Maximum = FindMaximum(L);
            CountingSort(L, Maximum + 1);
        } break;
    }
}
#endif

#endif

// Cleanup
#undef DL_NAME
#undef DL_TYPE
#ifdef DL_EQUALITY
#undef DL_EQUALITY
#endif
#ifdef DL_COMPARISON
#undef DL_COMPARISON
#endif
