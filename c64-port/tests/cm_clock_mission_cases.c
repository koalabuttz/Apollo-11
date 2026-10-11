/* SPDX-License-Identifier: MIT */
#include "cm_clock_mission_cases.h"
#include "cm_mission_loop_cases.h"
void cm_clock_mission_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){
 cm_executive e;uint16_t local=(uint16_t)(n-384u);
 if(n<384u){cm_mission_loop_input(s,io,n);if(n/64u==2u)s->words[01145]=01450;return;}
 cm_mission_loop_input(s,io,local);cm_exec_init(&e);
 e.core[0][CM_EXEC_PRIORITY]=020110;e.core[0][CM_EXEC_LOC]=03332;
 e.core[0][CM_EXEC_BANK]=050066;e.core[0][CM_EXEC_PUSH]=0110;
 e.core[1][CM_EXEC_PRIORITY]=010110;
 e.core[1][CM_EXEC_LOC]=agc_negate((agc_word)(03500u+local));
 e.core[1][CM_EXEC_BANK]=(agc_word)(02000u*(local%32u)|(local%8u));
 e.core[1][CM_EXEC_PUSH]=0110;cm_native_commit_executive(s,io,&e);
 s->words[3]=03000;s->words[4]=050000;s->words[6]=050006;s->words[5]=03332;
 s->words[01145]=0;io->superbank=060;io->interrupt_enabled=(agc_word)(local&1u);
}
