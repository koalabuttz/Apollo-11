/* SPDX-License-Identifier: MIT
 * Direct Comanche055 EXECUTIVE source composition; no opcode decoder. */
#include "cm_basic_priority.h"
enum exec_status cm_basic_priority_change(apollo_overlay_state *s,cm_native_io *io,exec_address *next){
 uint16_t *w=s->words,bank=w[6],caller=w[2],oldl=w[1],highest,buf0=0,buf1,buf2=w[0132],q=caller;
 uint8_t i,winner=0;cm_executive e;exec_address resume,result;enum exec_status status;
 static const uint16_t calls[6]={03143,03147,03153,03157,03163,03167};
 if(next==0||w[5]!=05210u||w[3]>03400u||(w[3]&0377u)!=0||(w[4]&01777u)!=0||w[4]>076000u||bank!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)!=0||io->interrupt_enabled>1u||caller<0100u||caller>07777u)return EXEC_OUT_OF_SCOPE;
 for(i=1;i<7u;++i)if(w[0167u+i*12u]==0)return EXEC_OUT_OF_SCOPE;
 cm_native_gather_executive(s,io,&e);resume.location=caller;resume.bank=(agc_word)(bank|io->superbank);
 highest=(uint16_t)(w[0]+(w[0167]&0777u));buf1=agc_negate(highest);
 for(i=1;i<7u;++i){uint16_t priority=w[0167u+i*12u];if(priority<040000u){buf2=(uint16_t)(priority-1u);if(buf2>highest){highest=buf2;buf1=agc_negate(buf2);buf0=calls[i-1u];winner=i;q=03223;}else q=03216;}}
 if(winner!=0u&&(w[0165u+winner*12u]&(uint16_t)~076167u)!=0)return EXEC_OUT_OF_SCOPE;
 status=cm_exec_priority_basic(&e,w[0],resume,&result);if(status!=EXEC_OK)return status;
 cm_native_commit_executive(s,io,&e);if(winner==0u)w[0165]=agc_correct(oldl);
 w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;
 w[0]=03013;w[1]=02006;w[2]=q;w[5]=result.location;w[6]=(uint16_t)(result.bank&076007u);
 w[3]=(uint16_t)((w[6]&7u)*0400u);w[4]=(uint16_t)(w[6]&076000u);
 if(winner!=0u)io->superbank=(uint16_t)(result.bank&0160u);
 io->interrupt_enabled=1;*next=result;return EXEC_OK;
}
