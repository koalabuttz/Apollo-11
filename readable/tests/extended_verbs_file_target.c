/* SPDX-License-Identifier: MIT */
/* The program the port's frozen cm-extended-verbs-file cases run
 * (c64-port/tests/test_cm_extended_verbs_file.c, the original program's
 * results in Virtual AGC): the readable file whole. */
#include "extended_verbs_file_program.h"

const agc_native_program *irc_program(uint8_t cm);

const agc_native_program *irc_program(uint8_t cm)
{
    static agc_native_program program;

    (void)cm;
    readable_cm_file_program(&program, 0);
    return &program;
}
