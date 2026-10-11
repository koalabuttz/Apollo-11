/* SPDX-License-Identifier: MIT */
#include "cm_priority_full_cases.h"
#include "cm_context_change_cases.h"
void cm_priority_full_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){
 static const agc_word bases[6]={0110,0401,0455,0531,0605,0661};static const agc_register raw[8]={0,0177777,1,0177776,040000,0137777,077777,0100000};
 uint32_t local=n%10752u;unsigned zero=n/10752u,level=local%32u,resource=(local/32u)%6u,winner=(local/192u)%7u,interp=(local/1344u)%2u,relation=(local/2688u)%4u,i;agc_word base=bases[resource],incoming=bases[(resource+1u)%6u],p;cm_executive e;
 cm_context_change_input(s,io,(n/32768u)&1u);s->words[3]=(agc_word)((n&7u)*0400u);s->words[4]=(agc_word)(((n>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(s->words[3]>>8));io->superbank=(agc_word)(((n>>8)&7u)*020u);io->interrupt_enabled=(agc_word)(n&1u);
 cm_native_gather_executive(s,io,&e);e.newjob=0;e.core[0][CM_EXEC_PRIORITY]=(agc_word)(023000u+base);e.core[0][CM_EXEC_PUSH]=base;e.fixloc=base;e.ovfind=(agc_word)(relation==0u?0u:relation==1u?1u:relation==2u?AGC_NEG_ZERO:2u);
 for(i=1;i<7u;++i)e.core[i][CM_EXEC_PRIORITY]=(i+n)%3u?AGC_NEG_ZERO:agc_negate(010110);
 if(winner){unsigned candidate_level=relation==0u?(level?level-1u:0u):relation==2u?(level<31u?level+1u:31u):level;p=(agc_word)(candidate_level*01000u+incoming);if(relation==3u&&base==0110u){incoming=0110;p=(agc_word)(level*01000u+incoming);}e.core[winner][CM_EXEC_PRIORITY]=p;e.core[winner][CM_EXEC_PUSH]=relation==0u?0:relation==1u?AGC_NEG_ZERO:relation==2u?incoming:agc_negate(incoming);e.core[winner][CM_EXEC_LOC]=interp?agc_negate(03420):(agc_word)03332;e.core[winner][CM_EXEC_BANK]=(agc_word)(interp?(022000u|(n&7u)|((n>>8)&7u)*020u):050066u);}
 if(zero<6u)e.core[zero+1u][CM_EXEC_PRIORITY]=0;
 cm_native_commit_executive(s,io,&e);s->words[5]=05210;s->words[0]=(agc_word)(level*01000u);s->words[1]=raw[(n/32u)%8u];s->words[2]=(agc_word)(0100u+n%07700u);s->words[023]=(agc_word)(n%0200u);s->words[0115]=(agc_word)(n&1u?040000u:0u);
}
