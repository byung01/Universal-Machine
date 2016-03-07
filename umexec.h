/*
 *      umexec.h
 *      Name: Flora Liu (fliu01), Bill Yung (byung01)
 *      Date: November 20, 2015
 *
 *      Purpose: Interface for executing the program
 */

#include <stdio.h>
#include <stdlib.h>
#include "segs.h"

extern void um_exec_prog(Segs_T program, unsigned *regs);
