/*
 *      ummain.c
 *      By Flora Liu and Bill Yung, 9/20/15
 *      Comp 40 Homework 2: Interfaces, Implementations, and Images
 *
 *      Purpose: Main function to link the interfaces
 */

#include <assert.h>
#include "umload.h"
#include "umexec.h"

int main(int argc, char *argv[])
{
        if (argc < 2) {
                fprintf(stderr, "Too few arguments.\n");
                exit(EXIT_FAILURE);
        }

        if (argc > 2) {
                fprintf(stderr, "Too many arguments.\n");
                exit(EXIT_FAILURE);
        }

        Segs_T program;
        uint32_t registers[8] = { 0 };
        
        FILE *fp = fopen(argv[1], "rb");
        if (fp == NULL) {
                fprintf(stderr, "%s %s %s\n","Could not open file ",
                                argv[1], "for reading.");
                exit(1);
        }

        program = um_load_prog(fp);
        assert(program);
        fclose(fp);
        
        um_exec_prog(program, registers);

        Segs_free(program);
        
        return 0;
}
