/*
 *      segs.h
 *      Name: Flora Liu (fliu01), Bill Yung (byung01)
 *      Date: November 15, 2015
 *
 *      Purpose: Interface for segmented memory
 */

#ifndef SEGS_H_INCLUDED
#define SEGS_H_INCLUDED
#include <stdint.h>

#define T Segs_T
typedef struct T *T;

extern T         Segs_new   ();
extern void      Segs_free  (T memory);
extern uint32_t  Segs_map   (T memory, int size);
extern void      Segs_unmap (T memory, unsigned segid);
extern void      Segs_store (T memory, uint32_t value, unsigned segid,
                             unsigned offset);
extern uint32_t  Segs_load  (T memory, unsigned segid, unsigned offset);
extern int       Segs_length(T memory, unsigned segid);
extern void      Segs_load_program(T memory, unsigned segid);

#undef T
#endif
