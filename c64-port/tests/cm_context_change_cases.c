/* SPDX-License-Identifier: MIT */
#include "cm_context_change_cases.h"
#include "cm_admission_complete_cases.h"
void cm_context_change_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){cm_executive e;unsigned slot=(n/4u)%6u+1u,selected_interpreter=(n/2u)%2u,i,c=n/24u;
 (void)cm_admission_complete_input(s,io,n%CM_ADMISSION_COMPLETE_CASES);s->words[3]=(agc_word)((c&7u)*0400u);s->words[4]=(agc_word)(((c>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));io->superbank=(agc_word)(((c>>6)&7u)*020u);io->interrupt_enabled=(agc_word)(c&1u);
 cm_native_gather_executive(s,io,&e);e.alarm=0;e.newjob=(agc_word)(slot*12u);for(i=0;i<7u;++i)e.core[i][CM_EXEC_PRIORITY]=AGC_NEG_ZERO;
 e.core[0][CM_EXEC_PRIORITY]=010401;e.core[0][CM_EXEC_PUSH]=(agc_word)(0401u+(c%16u));e.core[0][CM_EXEC_LOC]=(agc_word)(03500u+c%0100u);e.core[0][CM_EXEC_BANK]=s->words[6];e.ovfind=c%3u==0u?0:c%3u==1u?1:AGC_NEG_ZERO;
 e.core[slot][CM_EXEC_PRIORITY]=020455;e.core[slot][CM_EXEC_PUSH]=c%4u==0u?0:c%4u==1u?AGC_NEG_ZERO:c%4u==2u?0455:agc_negate(0455);
 e.core[slot][CM_EXEC_LOC]=selected_interpreter?agc_negate((agc_word)(03000u+c%0777u)):(agc_word)(02000u+c%01777u);e.core[slot][CM_EXEC_BANK]=(agc_word)((((c*13u)>>3)&31u)*02000u|(c&7u)|(((c>>3)&7u)*020u));cm_native_commit_executive(s,io,&e);
 s->words[5]=(n&1u)?05170:05164;s->words[0]=(agc_register)(c*967u+0100000u);s->words[1]=(agc_register)(c*313u+040000u);s->words[2]=(n&1u)?(agc_register)(c*997u+0100000u):(agc_word)(03500u+c%0100u);s->words[023]=(agc_word)(c%0200u);s->words[0115]=(agc_word)(c&1u?040000u:0u);
}
