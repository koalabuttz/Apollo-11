/* SPDX-License-Identifier: MIT */
#include "cm_priority_complete_cases.h"
#include "cm_context_change_cases.h"
void cm_priority_complete_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){unsigned c=n/14u,winner=(n/2u)%7u,i;agc_word priority=(agc_word)((c%31u+1u)*01000u);cm_executive e;
 cm_context_change_input(s,io,n%CM_CONTEXT_CHANGE_CASES);cm_native_gather_executive(s,io,&e);e.newjob=(agc_word)(c%7u*12u);e.core[0][CM_EXEC_PRIORITY]=010401;e.core[0][CM_EXEC_PUSH]=(agc_word)(0401u+c%16u);
 for(i=1;i<7u;++i)e.core[i][CM_EXEC_PRIORITY]=c%3u==0u?1:c%3u==1u?AGC_NEG_ZERO:agc_negate(010455);
 if(winner){e.core[winner][CM_EXEC_PRIORITY]=(agc_word)(priority+0455u);e.core[winner][CM_EXEC_PUSH]=c%4u==0u?0:c%4u==1u?AGC_NEG_ZERO:c%4u==2u?0455:agc_negate(0455);e.core[winner][CM_EXEC_LOC]=(n&1u)?agc_negate((agc_word)(03000u+c%0777u)):(agc_word)(02000u+c%01777u);e.core[winner][CM_EXEC_BANK]=(agc_word)((((c*13u)>>3)&31u)*02000u|(c&7u)|(((c>>3)&7u)*020u));}
 cm_native_commit_executive(s,io,&e);s->words[5]=05210;s->words[0]=priority;s->words[1]=(agc_register)(c*313u+040000u);s->words[2]=(agc_word)(0705u+c%0373u);
}
