/* SPDX-License-Identifier: MIT; original Comanche idle Executive. */
#include "cm_idle_dispatch.h"
enum exec_status cm_idle_dispatch(apollo_overlay_state *s,cm_native_io *io,exec_transfer *next){agc_word flag,oldbb,debris;uint8_t slot,interp;cm_executive e;exec_address selected;enum exec_status status;
 if(s==0||io==0||next==0||s->words[5]!=03231u||s->words[4]!=02000u||s->words[3]>03400u||(s->words[3]&0377u)||s->words[6]!=(agc_word)(02000u|(s->words[3]>>8))||s->words[7]!=0||(io->superbank&~0160u)||io->interrupt_enabled>1u||io->channel11>AGC_MASK)return EXEC_OUT_OF_SCOPE;
 flag=s->words[0067];oldbb=s->words[6];debris=agc_correct(s->words[1]);
 if(flag==AGC_NEG_ZERO){if(s->words[01361]==0||s->words[01361]>07777u)return EXEC_OUT_OF_SCOPE;s->words[0]=agc_extend(066102);s->words[1]=agc_extend(s->words[01361]);s->words[2]=2;s->words[3]=01000;s->words[4]=066000;s->words[6]=066002;s->words[5]=s->words[01361];io->superbank=0100;next->address=(exec_address){s->words[5],066102};next->interpretive=0;return EXEC_IDLE;}
 if(flag>72u||flag%12u)return EXEC_OUT_OF_SCOPE;
 slot=(uint8_t)(flag/12u);interp=(uint8_t)((s->words[0164u+flag]&AGC_SIGN)!=0);if(!slot&&interp)return EXEC_OUT_OF_SCOPE;
 cm_native_gather_executive(s,io,&e);if(interp){agc_word loc=e.core[slot][CM_EXEC_LOC];if(agc_negate(loc)>=07777u)return EXEC_OUT_OF_SCOPE;e.core[slot][CM_EXEC_LOC]=agc_add_word(agc_negate(loc),1);}
 status=cm_exec_start_basic(&e,&selected);if(status!=EXEC_OK||(selected.bank&(agc_word)~076167u))return EXEC_OUT_OF_SCOPE;
 io->channel11=(agc_word)(io->channel11|2u);if(slot){e.core[0][CM_EXEC_BANK]=debris;if(interp){e.core[0][CM_EXEC_LOC]=selected.location;e.core[0][CM_EXEC_BANK]=(agc_word)(selected.bank&076007u);}}
 else{e.core[0][CM_EXEC_LOC]=io->channel11;e.core[0][CM_EXEC_BANK]=debris;}
 cm_native_commit_executive(s,io,&e);s->words[6]=(agc_word)(selected.bank&076007u);s->words[3]=(agc_word)((s->words[6]&7u)*0400u);s->words[4]=(agc_word)(s->words[6]&076000u);io->superbank=(agc_word)(selected.bank&0160u);
 if(!slot){s->words[0]=agc_extend(selected.bank);s->words[1]=agc_extend(selected.location);s->words[2]=2;s->words[5]=selected.location;}
 else{io->interrupt_enabled=1;s->words[0]=03013;s->words[1]=oldbb;s->words[5]=selected.location;if(interp){s->words[0115]=(agc_word)(s->words[6]&040000u);s->words[023]=0;s->words[0]=agc_extend(s->words[0115]);s->words[5]=06037;}}
 next->address=selected;next->interpretive=interp;return EXEC_OK;}
