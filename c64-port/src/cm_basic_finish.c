/* SPDX-License-Identifier: MIT; derived from original Comanche Executive. */
#include "cm_basic_finish.h"
enum exec_status cm_basic_finish(apollo_overlay_state *s,cm_native_io *io,exec_transfer *next){uint16_t *w=s->words,base=(uint16_t)(w[0167]&0777u),oldEB=w[3],highest=0,buf0=w[0130],buf1=AGC_NEG_ZERO,buf2=w[0132],q=w[2];uint8_t i,winner=0,interpretive=0;cm_executive e;exec_address result;enum exec_status status;static const agc_word calls[6]={03143,03147,03153,03157,03163,03167};
 if(next==0||w[5]!=05217u||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]!=0||w[0167]==0||w[0167]>=040000u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||io->channel11>AGC_MASK||(base>CM_EXEC_FAKEPRET&&(base<0401u||base>0661u||(base-0401u)%054u)))return EXEC_OUT_OF_SCOPE;
 for(i=1;i<7u;++i){uint16_t priority=w[0167u+12u*i];if(priority==0)return EXEC_OUT_OF_SCOPE;if(priority<040000u){buf2=(uint16_t)(priority-1u);if(buf2>highest){highest=buf2;buf1=agc_negate(buf2);buf0=calls[i-1u];winner=i;q=03223;}else q=03216;}}
 if(winner){agc_word loc=w[0164u+12u*winner],bank=w[0165u+12u*winner];interpretive=(uint8_t)((loc&AGC_SIGN)!=0);if((bank&(agc_word)~076167u)||(!interpretive&&(loc==0||loc>07777u))||(interpretive&&agc_negate(loc)>=07777u))return EXEC_OUT_OF_SCOPE;}
 cm_native_gather_executive(s,io,&e);for(i=1;i<7u;++i)if(w[0167u+12u*i]==1u)e.core[i][CM_EXEC_PRIORITY]=AGC_NEG_ZERO;
 /* Reuse native source context exchange. The canonical negative LOC is
  * transformed only in this transient typed view to its positive INTRSM next
  * address; the selected slot receives the same original outgoing LOC. */
 if(interpretive)e.core[winner][CM_EXEC_LOC]=agc_add_word(agc_negate(e.core[winner][CM_EXEC_LOC]),1);
 status=cm_exec_complete_basic(&e,io->superbank,&result);if(status!=EXEC_OK&&status!=EXEC_IDLE)return status;
 for(i=1;i<7u;++i)if(w[0167u+12u*i]==1u)e.core[i][CM_EXEC_PRIORITY]=1;
 cm_native_commit_executive(s,io,&e);w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;w[2]=q;io->interrupt_enabled=1;
 if(status==EXEC_IDLE){io->channel11=(agc_word)(io->channel11&077775u);w[0]=agc_extend(io->channel11);w[1]=base;w[4]=02000;w[6]=(agc_word)(02000u|(oldEB>>8));w[5]=03231;}
 else{w[1]=(agc_word)(02000u|(oldEB>>8));w[6]=(agc_word)(result.bank&076007u);w[3]=(agc_word)((w[6]&7u)*0400u);w[4]=(agc_word)(w[6]&076000u);io->superbank=(agc_word)(result.bank&0160u);if(interpretive){w[0164]=result.location;w[0165]=w[6];w[0115]=(agc_word)(w[6]&040000u);w[023]=0;w[0]=agc_extend(w[0115]);w[5]=06037;}else{w[0]=03013;w[5]=result.location;}}
 next->address=result;next->interpretive=interpretive;return status;}
