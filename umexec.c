/*
 *      umexec.c
 *      Name: Flora Liu (fliu01), Bill Yung (byung01)
 *      Date: November 20, 2015
 *
 *      Purpose: Implementation for executing the program
 */

#include <assert.h>
#include <math.h>
#include "umexec.h"
#include "bitpack.h"

/******************************************************************************/
/*                                 Instructions                               */
/******************************************************************************/
static void conditional_move(unsigned *regs, unsigned A, unsigned B,
                             unsigned C);
static void segmented_load(unsigned *regs, unsigned A, unsigned B, unsigned C,
                           Segs_T program);
static void segmented_store(unsigned *regs, unsigned A, unsigned B, unsigned C,
                            Segs_T program);
static void addition(unsigned *regs, unsigned A, unsigned B, unsigned C);
static void multiplication(unsigned *regs, unsigned A, unsigned B, unsigned C);
static void division(unsigned *regs, unsigned A, unsigned B, unsigned C);
static void bitwise_nand(unsigned *regs, unsigned A, unsigned B, unsigned C);
static void halt(Segs_T program);
static void map_segment(unsigned *regs, unsigned B, unsigned C, Segs_T program);
static void unmap_segment(unsigned *regs, unsigned C, Segs_T program);
static void output(unsigned *regs, unsigned C);
static void input(unsigned *regs, unsigned C);
static void load_program(unsigned *regs, unsigned B, unsigned C,
                         Segs_T program, unsigned *prog_ctr);
static void load_value(unsigned *regs, unsigned A, uint32_t value);

/******************************************************************************/
/*                               Helper Functions                             */
/******************************************************************************/
typedef struct Instruction {
        unsigned opcode;
        unsigned regA, regB, regC;
} Instruction;

static Instruction extract_instruction(uint32_t word, Instruction instr);
static void run_instruction(Instruction instr, unsigned *regs, Segs_T program,
                            unsigned *prog_ctr);

/******************************************************************************/
/*                           Function Definitions                             */
/******************************************************************************/
                            
extern void um_exec_prog(Segs_T program, unsigned *regs)
{
        uint32_t word;
        Instruction instruction;
        unsigned prog_ctr = 0;

        while (true) {
                word = Segs_load(program, 0, prog_ctr);
                instruction = extract_instruction(word, instruction);
                run_instruction(instruction, regs, program, &prog_ctr);
                prog_ctr++;
        }
}

static Instruction extract_instruction(uint32_t word, Instruction instr)
{
        instr.opcode = Bitpack_getu(word, 4, 28);
        
        if (instr.opcode == 13) {
                instr.regA = Bitpack_getu(word, 3, 25);
                instr.regB = Bitpack_getu(word, 25, 0);
        } else {
                instr.regA = Bitpack_getu(word, 3, 6);
                instr.regB = Bitpack_getu(word, 3, 3);
                instr.regC = Bitpack_getu(word, 3, 0);
        }

        return instr;
}

static void run_instruction(Instruction instr, unsigned *regs, Segs_T program,
                            unsigned *prog_ctr)
{
        assert(instr.opcode <= 13);

        switch(instr.opcode) {
                case 0:
                        conditional_move(regs, instr.regA, instr.regB,
                                         instr.regC);
                        break;
                case 1:
                        segmented_load(regs, instr.regA, instr.regB,
                                       instr.regC, program);
                        break;
                case 2:
                        segmented_store(regs, instr.regA, instr.regB,
                                        instr.regC, program);
                        break;
                case 3:
                        addition(regs, instr.regA, instr.regB, instr.regC);
                        break;
                case 4:
                        multiplication(regs, instr.regA, instr.regB,
                                       instr.regC);
                        break;
                case 5:
                        division(regs, instr.regA, instr.regB, instr.regC);
                        break; 
                case 6:
                        bitwise_nand(regs, instr.regA, instr.regB, instr.regC);
                        break;
                case 7:
                        halt(program);
                        break;
                case 8:
                        map_segment(regs, instr.regB, instr.regC, program);
                        break;
                case 9:
                        unmap_segment(regs, instr.regC, program);
                        break;
                case 10:
                        output(regs, instr.regC);
                        break;
                case 11:
                        input(regs, instr.regC);
                        break;
                case 12:
                        load_program(regs, instr.regB, instr.regC, program,
                                     prog_ctr);
                        break;          
                case 13:
                        load_value(regs, instr.regA, (uint32_t)instr.regB);
                        break;
        }
}

static void conditional_move(uint32_t *regs, unsigned A, unsigned B, unsigned C)
{
        if (regs[C] != 0) {
                regs[A] = regs[B];
        }
}

static void segmented_load(uint32_t *regs, unsigned A, unsigned B, unsigned C,
                           Segs_T program)
{
        regs[A] = Segs_load(program, regs[B], regs[C]);
}

static void segmented_store(uint32_t *regs, unsigned A, unsigned B, unsigned C,
                            Segs_T program)
{
        Segs_store(program, regs[C], regs[A], regs[B]);
}

static void addition(uint32_t *regs, unsigned A, unsigned B, unsigned C)
{
        regs[A] = regs[B] + regs[C];
}

static void multiplication(uint32_t *regs, unsigned A, unsigned B, unsigned C)
{
        regs[A] = regs[B] * regs[C];
}

static void division(uint32_t *regs, unsigned A, unsigned B, unsigned C)
{
        regs[A] = regs[B] / regs[C];
}

static void bitwise_nand(uint32_t *regs, unsigned A, unsigned B, unsigned C)
{
        regs[A] = ~(regs[B] & regs[C]);
}

static void halt(Segs_T program)
{
        Segs_free(program);
        exit(0);
}

static void map_segment(uint32_t *regs, unsigned B, unsigned C, Segs_T program)
{
        unsigned address = Segs_map(program, regs[C]);
        regs[B] = address;
}

static void unmap_segment(uint32_t *regs, unsigned C, Segs_T program)
{
        Segs_unmap(program, regs[C]);
}

static void output(uint32_t *regs, unsigned C)
{
        printf("%c", regs[C]);
}


static void input(uint32_t *regs, unsigned C)
{
        regs[C] = fgetc(stdin);
}

static void load_program(uint32_t *regs, unsigned B, unsigned C,
                         Segs_T program, unsigned *prog_ctr)
{
        if (regs[B] != 0) {
                Segs_load_program(program, regs[B]);
        }

        *prog_ctr = regs[C] - 1;
}

static void load_value(uint32_t *regs, unsigned A, uint32_t value)
{
        regs[A] = value;
}
