/*
 *      umlab.c
 *      Name: Flora Liu (fliu01), Bill Yung (byung01)
 *      Date: November 15, 2015
 *
 *      Purpose: Implementation for unit tests
 */

#include <stdint.h>
#include "bitpack.h"
#include <seq.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define COND_MOVE 0
#define SEG_LOAD 1
#define SEG_STORE 2
#define ADDITION 3
#define MULTIPLY 4
#define DIVISION 5
#define BIT_NAND 6
#define HALT 7
#define MAP_SEG 8
#define UNMAP_SEG 9
#define OUTPUT 10
#define INPUT 11
#define LOAD_PROG 12
#define LOAD_VAL 13

typedef uint32_t Um_instruction;
typedef unsigned Um_opcode;

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc);
Um_instruction loadval(unsigned ra, unsigned val);

static inline Um_instruction cond_move(int A, int B, int C)
{ 
        return three_register(COND_MOVE, A, B, C);
}

static inline Um_instruction seg_load(int A, int B, int C)
{
        return three_register(SEG_LOAD, A, B, C);
}

static inline Um_instruction seg_store(int A, int B, int C)
{
        return three_register(SEG_STORE, A, B, C);
}

static inline Um_instruction addition(int A, int B, int C)
{
        return three_register(ADDITION, A, B, C);
}

static inline Um_instruction multiply(int A, int B, int C)
{
        return three_register(MULTIPLY, A, B, C);
}

static inline Um_instruction division(int A, int B, int C)
{
        return three_register(DIVISION, A, B, C);
}

static inline Um_instruction bit_nand(int A, int B, int C)
{
        return three_register(BIT_NAND, A, B, C);
}

static inline Um_instruction halt(void)
{
        return three_register(HALT, 0, 0, 0);
}

static inline Um_instruction map_seg(int B, int C)
{
        return three_register(MAP_SEG, 0, B, C);
}

static inline Um_instruction unmap_seg(int C)
{
        return three_register(UNMAP_SEG, 0, 0, C);
}

static inline Um_instruction output(int C)
{
        return three_register(OUTPUT, 0, 0, C);
}

static inline Um_instruction input(int C)
{
        return three_register(INPUT, 0, 0, C);
}

static inline Um_instruction loadprogram(int B, int C)
{
        return three_register(LOAD_PROG, 0, B, C);
}

static inline Um_instruction load_val(unsigned A, unsigned value)
{
        return loadval(A, value);
}

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc)
{
        uint32_t instr = 0;
        instr = Bitpack_newu(instr, 4, 28, op); /* Inserting Opcode */
        instr = Bitpack_newu(instr, 3, 6, ra); /* Inserting Register number */
        instr = Bitpack_newu(instr, 3, 3, rb); /* Inserting Register number */
        instr = Bitpack_newu(instr, 3, 0, rc); /* Inserting Register number */
        return instr;
}

Um_instruction loadval(unsigned ra, unsigned val)
{
        uint32_t instr = 0;
        instr = Bitpack_newu(instr, 4, 28, LOAD_VAL);
        instr = Bitpack_newu(instr, 3, 25, ra);
        instr = Bitpack_newu(instr, 25, 0, val);
        return instr;
}

enum regs { r0 = 0, r1, r2, r3, r4, r5, r6, r7 };

static inline void emit(Seq_T stream, Um_instruction inst)
{
	assert(sizeof(inst) <= sizeof(uintptr_t));
	Seq_addhi(stream, (void *)(uintptr_t) inst);
}

static inline Um_instruction get_inst(Seq_T stream, int i)
{
	assert(sizeof(Um_instruction) <= sizeof(uintptr_t));
	return (Um_instruction) (uintptr_t) (Seq_get(stream, i));
}

static inline void put_inst(Seq_T stream, int i, Um_instruction inst)
{
	assert(sizeof(inst) <= sizeof(uintptr_t));
	Seq_put(stream, i, (void *)(uintptr_t) inst);
}

void emit_halt_test(Seq_T stream)
{
	emit(stream, halt());
	emit(stream, loadval(r1, 'B'));
	emit(stream, output(r1));
	emit(stream, loadval(r1, 'a'));
	emit(stream, output(r1));
	emit(stream, loadval(r1, 'd'));
	emit(stream, output(r1));
	emit(stream, loadval(r1, '!'));
	emit(stream, output(r1));
	emit(stream, loadval(r1, '\n'));
	emit(stream, output(r1));
}

static void add_label(Seq_T stream, int location_to_patch, int label_value)
{
	Um_instruction inst = get_inst(stream, location_to_patch);
	unsigned k = Bitpack_getu(inst, 25, 0);
	inst = Bitpack_newu(inst, 25, 0, label_value + k);
	put_inst(stream, location_to_patch, inst);
}

static void emit_out_string(Seq_T stream, const char *s, int aux_reg)
{
        int string_length = strlen(s);
        for (int i = 0; i < string_length; i++) {
                char c = s[i];
                emit(stream, load_val(aux_reg, c));
                emit(stream, output(aux_reg));
        }
}

void emit_goto_test(Seq_T stream)
{
	int patch_L = Seq_length(stream);
	emit(stream, loadval(r7, 0));	     /* will be patched to 'r7 := L' */
	emit(stream, loadprogram(r0, r7));   /* should goto label L          */
	emit_out_string(stream, "GOTO failed.\n", r1);
	emit(stream, halt());
	/* define 'L' to be here */
	add_label(stream, patch_L, Seq_length(stream));	
	emit_out_string(stream, "GOTO passed.\n", r1);
}

void emit_cond_move_test(Seq_T stream)
{
        int patch_L1 = Seq_length(stream);
        emit(stream, loadval(r7, 0));
        int patch_L2 = Seq_length(stream);
        emit(stream, loadval(r6, 0));
        emit(stream, cond_move(r7, r6, r0));
        emit(stream, loadprogram(r0, r7));
        emit_out_string(stream, "Halted.\n", r5);
        emit(stream, halt());
        add_label(stream, patch_L2, Seq_length(stream));
        emit_out_string(stream, "Conditional Move on zero register failed.\n",
                        r5);
        add_label(stream, patch_L1, Seq_length(stream));
        emit_out_string(stream, "Conditional Move on zero register passed.\n",
                        r5);
        emit(stream, halt());
}

void emit_mapping_test(Seq_T stream)
{
        emit(stream, loadval(r7, '\n'));
        emit(stream, loadval(r1, 10));
        emit(stream, map_seg(r2, r1)); /* r2 holds the address of mapped seg */
        emit_out_string(stream, "Initial address: ", r6);
        emit(stream, loadval(r4, 48)); 
        emit(stream, addition(r5, r2, r4)); /* Adding to print num in acsii */
        emit(stream, output(r5)); /* Printing the address of mapped seg */
        emit(stream, output(r7));
        emit(stream, loadval(r3, 7)); /* Setting offset in the segment */
        emit(stream, loadval(r4, 2)); /* Storing 2 in m[r2][r3] */
        emit(stream, seg_store(r2, r3, r4));
        emit(stream, seg_load(r5, r2, r3));
        emit_out_string(stream, "Value of what we stored: ", r6);
        emit(stream, loadval(r4, 48));
        emit(stream, addition(r5, r5, r4)); /* Adding to print num in acsii */
        emit(stream, output(r5)); /* Printing what we stored */
        emit(stream, output(r7));
        emit(stream, unmap_seg(r2));
        emit(stream, map_seg(r2, r1)); /* Mapping again */
        emit_out_string(stream, "Second address: ", r6);
        emit(stream, loadval(r4, 48));
        emit(stream, addition(r2, r2, r4)); /* Adding to print num in acsii */
        emit(stream, output(r2));
        emit(stream, output(r7));
        emit(stream, halt());
}

void emit_io_test(Seq_T stream)
{
        /* Takes in input from stdin and outputs it */
        /* #HelloWorld */
        emit(stream, loadval(r7, '\n'));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, input(r1));
        emit(stream, output(r1));
        emit(stream, output(r7));
        emit(stream, halt());
}

void emit_arith_test(Seq_T stream)
{
        emit(stream, loadval(r0, 3));
        emit(stream, loadval(r1, 48));
        emit(stream, loadval(r2, 4));
        emit(stream, loadval(r3, 16));
        emit(stream, loadval(r4, 197));
        emit(stream, loadval(r5, 3));
        emit(stream, loadval(r7, '\n'));

        /* Addition */
        emit(stream, addition(r6, r0, r1)); /* r6 should hold 51 */
        emit(stream, output(r6)); /* Should print 3 */
        emit(stream, output(r7));

        /* Multiplication */
        emit(stream, multiply(r6, r2, r3)); /* r6 should hold 64 */
        emit(stream, output(r6)); /* Should print @ */
        emit(stream, output(r7));

        /* Division */
        emit(stream, division(r6, r4, r5)); /* r6 should hold 65 */
        emit(stream, output(r6)); /* Should print A */
        emit(stream, output(r7));
        emit(stream, halt());
}

void emit_nand_test(Seq_T stream)
{
        emit(stream, loadval(r0, 65537));
        emit(stream, loadval(r1, 65535));
        emit(stream, loadval(r2, 65528));
        emit(stream, loadval(r3, 65544));
        emit(stream, loadval(r7, '\n'));

        emit(stream, multiply(r4, r0, r1)); /* (2^32) - 1 */
        emit(stream, multiply(r5, r2, r3)); /* (2^32) - 64 */

        emit(stream, bit_nand(r6, r4, r5)); /* r6 Should be 63 */
        emit(stream, output(r6)); /* Should print out ? */
        emit(stream, output(r7));
        emit(stream, halt());
}

extern void Um_write_sequence(FILE *output, Seq_T stream)
{
        int length = Seq_length(stream);
        int i;
        Um_instruction instr;

        for (i = 0; i < length; i++) {
                instr = get_inst(stream, i);
                for (int j = 3; j >= 0; j--) {
                char byte = (char)Bitpack_getu(instr, 8, (j * 8));
                putc(byte, output);
                }
        }
}