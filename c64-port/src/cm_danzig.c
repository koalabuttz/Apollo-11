/* SPDX-License-Identifier: MIT; original Comanche interpreter/Executive. */
#include "cm_danzig.h"
enum exec_status cm_danzig(apollo_overlay_state *s,cm_native_io *io,exec_transfer *next){cm_executive e;exec_interpreter control;enum exec_status status;agc_word bank,flag;
 if(s==0||io==0||next==0)return EXEC_OUT_OF_SCOPE;
 bank=s->words[0165];flag=s->words[0067];
 if(s->words[5]!=06030u||flag==AGC_NEG_ZERO||s->words[023]!=0||s->words[7]!=0||(bank&(agc_word)~076007u)||s->words[0164]==0||s->words[0164]>=07777u||(io->superbank&(agc_word)~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 if(flag==0){s->words[6]=bank;s->words[3]=(agc_word)((bank&7u)*0400u);s->words[4]=(agc_word)(bank&076000u);s->words[0]=0;if(flag==0)s->words[0164]=agc_add_word(s->words[0164],1);s->words[5]=06037;next->interpretive=1;next->address.location=s->words[0164];next->address.bank=(agc_word)(bank|io->superbank);return EXEC_OK;}
 cm_native_gather_executive(s,io,&e);control.edop=0;control.intbit15=s->words[0115];
 status=cm_exec_change_interpretive(&e,io->superbank,&control,next);if(status!=EXEC_OK)return status;
 cm_native_commit_executive(s,io,&e);s->words[6]=(agc_word)(next->address.bank&076007u);s->words[3]=(agc_word)((s->words[6]&7u)*0400u);s->words[4]=(agc_word)(s->words[6]&076000u);io->superbank=(agc_word)(next->address.bank&0160u);io->interrupt_enabled=1;
 s->words[1]=02006;s->words[0]=03013;s->words[5]=next->address.location;
 if(next->interpretive){s->words[0115]=control.intbit15;s->words[023]=control.edop;s->words[0]=agc_extend(control.intbit15);s->words[5]=06037;}
 return EXEC_OK;}
