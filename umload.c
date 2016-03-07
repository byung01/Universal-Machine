/*
 *      umload.c
 *      Name: Flora Liu (fliu01), Bill Yung (byung01)
 *      Date: November 20, 2015
 *
 *      Purpose: Implementation for loading the program
 */

#include "umload.h"
#include "bitpack.h"

extern Segs_T um_load_prog(FILE *fp)
{
        /* Getting the file_size */
        int prev = ftell(fp);
        fseek(fp, 0L, SEEK_END);
        int file_size = ftell(fp);
        fseek(fp, prev, SEEK_SET);
        
        int number_instructions = file_size / 4;
        Segs_T memory = Segs_new();
        uint32_t address = Segs_map(memory, number_instructions);
        
        /* Loop to read in the 32-bit instructions */
        for (int i = 0; i < number_instructions; i++) {
                uint64_t instr = 0;
                for (int j = 3; j >= 0; j--) {
                        uint64_t byte = getc(fp);
                        instr = Bitpack_newu(instr, 8, (j * 8), byte);
                }
                Segs_store(memory, (uint32_t)instr, address, i);
        }
        
        return memory;
}
