/* SPDX-License-Identifier: MIT */
#include "cm_basic_wake.h"
enum exec_status cm_basic_wake(apollo_overlay_state *s,cm_native_io *io){
 uint16_t *w=s->words,caller=w[2],oldl=w[1],oldfb=w[4],wake=agc_correct(w[0]),savedbank=0;
 uint8_t i,slot=7;cm_executive e;enum exec_status status;
 if(w[5]!=05201u||w[0]!=agc_extend(wake)||io->executive_alarm!=0||caller<0100u||caller>07777u||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||w[0167]==0||w[0167]>=040000u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||(w[067]!=AGC_NEG_ZERO&&(w[067]>72u||w[067]%12u)))return EXEC_OUT_OF_SCOPE;
 for(i=1;i<7u;++i){uint16_t p=w[0167u+12u*i],loc=w[0164u+12u*i];if(p==0)return EXEC_OUT_OF_SCOPE;if(slot==7u&&p>=040000u&&p<AGC_NEG_ZERO&&(loc==wake||((loc==0||loc==AGC_NEG_ZERO)&&(wake==0||wake==AGC_NEG_ZERO)))){slot=i;savedbank=w[0165u+12u*i];}}
 cm_native_gather_executive(s,io,&e);status=cm_exec_wake_basic(&e,wake);if((uint16_t)status!=(uint16_t)EXEC_OK&&(uint16_t)status!=(uint16_t)EXEC_NOT_FOUND)return status;
 if((uint16_t)status==(uint16_t)EXEC_OK)e.newloc[1]=agc_correct(oldl);
 cm_native_commit_executive(s,io,&e);w[061]=oldfb;w[062]=(uint16_t)(slot<7u?6u-slot:0u);
 w[0]=agc_extend(oldfb);w[1]=slot<7u?agc_extend(savedbank):agc_extend(agc_correct(oldl));w[2]=06712;w[5]=caller;io->interrupt_enabled=0;
 return status;
}
