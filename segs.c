/*
 *      segs.c
 *      Name: Flora Liu (fliu01), Bill Yung (byung01)
 *      Date: November 15, 2015
 *
 *      Purpose: Implementation for segmented memory
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <seq.h>
#include <uarray.h>
#include <stack.h>
#include "segs.h"

#define T Segs_T

struct T {
        Seq_T segments;  /* Represents our segmented memory */
        Stack_T tracker; /* Keeps track of which segid's are unmapped */
};

extern T Segs_new()
{
        T memory = malloc(sizeof(*memory));
        assert(memory);
        
        /* Creating a sequence to represent our main memory */
        memory->segments = Seq_new(5);
        assert(memory->segments);

        /* Creating a stack to keep track of unmapped segments */
        memory->tracker = Stack_new();

        return memory;
}

extern void Segs_free(T memory)
{       
        UArray_T segment;
        int length;
        
        /* Freeing segments in memory */
        length = Seq_length(memory->segments);
        for (int i = 0; i < length; i++) {
                segment = Seq_remlo(memory->segments);
                if (segment != NULL)
                        UArray_free(&segment);
        }

        /* Freeing the struct and the elements within it */
        Seq_free(&(memory->segments));
        Stack_free(&(memory->tracker));
        free(memory);
}

extern uint32_t Segs_map(T memory, int size)
{
        unsigned index;
        UArray_T new_segment = UArray_new(size, sizeof(uint32_t));
        assert(new_segment);

        /* Checks to see if there are any existing unmapped segments */
        if (Stack_empty(memory->tracker)) {
                Seq_addhi(memory->segments, (void *)new_segment);
                index = Seq_length(memory->segments) - 1;
        } else {
                index = (unsigned)(uintptr_t)Stack_pop(memory->tracker);
                Seq_put(memory->segments, index, (void *)new_segment);
        }
        
        return (uint32_t)index;
}

extern void Segs_unmap(T memory, unsigned segid)
{
        UArray_T segment = Seq_put(memory->segments, segid, NULL);
        UArray_free(&segment);
        Stack_push(memory->tracker, (void *)(uintptr_t)segid);
}

extern void Segs_store(T memory, uint32_t value, unsigned segid,
                       unsigned offset)
{
        assert(sizeof(value) <= sizeof(uintptr_t));
        UArray_T segment = Seq_get(memory->segments, segid);
        *(uint32_t *)UArray_at(segment, offset) = value;
}

extern uint32_t Segs_load(T memory, unsigned segid, unsigned offset)
{
        UArray_T segment = Seq_get(memory->segments, segid);
        return *(uint32_t *)UArray_at(segment, offset);
}

extern int Segs_length(T memory, unsigned segid)
{
        UArray_T segment = Seq_get(memory->segments, segid);
        return UArray_length(segment);
}

extern void Segs_load_program(T memory, unsigned segid)
{
        UArray_T segment = Seq_get(memory->segments, segid);
        int length = UArray_length(segment);
        UArray_T new_prog = UArray_copy(segment, length);
        UArray_T segment0 = Seq_put(memory->segments, 0, new_prog);
        UArray_free(&segment0);
}
