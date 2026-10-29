/* SPDX-License-Identifier: MIT */
/* The program the lm-interpreter cases run: Luminary099
 * INTERPRETER' routines alone, basic ones at Z and
 * interpretive ones at NEWOPS. */
#include "agc_transfer.h"
#include "lm_interpreter.h"
#include "rope_image.h"
/* DMP, POLY and the others read their callers' words from the rope image
 * (tools/basic_compile.py 'fixed_rope'): the host loads the yaYUL binary
 * once; the C64 reads the REU's (build/self-check.reu). */
#ifndef __mos__
/* Next to this file's directory, so a runner built and run from another
 * directory (the cert self-check's scratch builds) still finds the rope. */
#include <stdio.h>
#include <string.h>
#ifndef IRC_ROPE
static const char *irc_rope_path(void){static char b[1024];const char *e=strrchr(__FILE__,'/');
 snprintf(b,sizeof b,"%.*s../build/fixed-data/Luminary099/MAIN.agc.bin",e?(int)(e-__FILE__+1):0,__FILE__);return b;}
#define IRC_ROPE irc_rope_path()
#endif
static int rope_loaded;
#endif
const agc_native_program *irc_program(uint8_t cm);
static const agc_native_program lm={&lm_interpreter_basic,1,&lm_interpreter_interpretive,1,0,0,
 (uint16_t)(2048u+06070u-04000u),0164,0165,0,0,0};
const agc_native_program *irc_program(uint8_t m){(void)m;
#ifndef __mos__
 if(!rope_loaded)rope_loaded=apollo_rope_load(0u,IRC_ROPE)?1:-1;
#endif
 return &lm;}
