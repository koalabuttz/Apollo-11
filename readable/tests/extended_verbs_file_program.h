/* SPDX-License-Identifier: MIT */
/* The CM's readable EXTENDED_VERBS as one program (tests/
 * extended_verbs_file_program.c). */
#ifndef READABLE_EXTENDED_VERBS_FILE_PROGRAM_H
#define READABLE_EXTENDED_VERBS_FILE_PROGRAM_H
#include "agc_transfer.h"

/* Fill *p: the readable file's tables and the parts', with hardware for
 * the program's agc_hardware. */
void readable_cm_file_program(agc_native_program *p, agc_hardware *hardware);
#endif
