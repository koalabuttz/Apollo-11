/* SPDX-License-Identifier: MIT */
/* The program the cm-interpreter cases run: Comanche055
 * INTERPRETER' routines alone, basic ones at Z and
 * interpretive ones at NEWOPS. */
#include "agc_transfer.h"
#include "cm_interpreter.h"
#include "rope_image.h"
/* DMP, POLY and the others read their callers' words from the rope image
 * (tools/basic_compile.py 'fixed_rope'): the host loads the yaYUL binary
 * once; the C64 reads the REU's (build/self-check.reu). */
#ifndef __mos__
#ifndef IRC_ROPE
#define IRC_ROPE "build/fixed-data/Comanche055/MAIN.agc.bin"
#endif
static int rope_loaded;
#endif
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program cm={&cm_interpreter_basic,1,&cm_interpreter_interpretive,1,0,0,
 (uint16_t)(2048u+06037u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;
#ifndef __mos__
 if(!rope_loaded)rope_loaded=apollo_rope_load(1u,IRC_ROPE)?1:-1;
#endif
 return &cm;}
