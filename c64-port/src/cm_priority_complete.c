/* SPDX-License-Identifier: MIT; source-derived CM typed priority bridge. */
#include "cm_priority_complete.h"
enum exec_status cm_priority_complete(apollo_overlay_state *s,cm_native_io *io,exec_transfer *next){uint16_t *w;agc_word bank,caller,oldl,highest,buf0=0,buf1,buf2,q;uint8_t i,winner=0,interpretive=0;cm_executive e;exec_address resume,result;enum exec_status status;static const agc_word calls[6]={03143,03147,03153,03157,03163,03167};
 if(!s||!io||!next)return EXEC_OUT_OF_SCOPE;
 w=s->words;bank=w[6];caller=w[2];oldl=w[1];buf2=w[0132];q=caller;
 if(w[5]!=05210u||w[3]>03400u||(w[3]&0377u)||(w[4]&01777u)||w[4]>076000u||bank!=(agc_word)(w[4]|(w[3]>>8))||w[7]||(io->superbank&~0160u)||io->interrupt_enabled>1u||!caller||caller>07777u||io->executive_alarm||!w[0]||(w[0]&0140777u)||!w[0167]||w[0167]>=040000u)return EXEC_OUT_OF_SCOPE;
 for(i=1;i<7u;++i)if(w[0167u+i*12u]==0)return EXEC_OUT_OF_SCOPE;
 cm_native_gather_executive(s,io,&e);resume.location=caller;resume.bank=(agc_word)(bank|io->superbank);highest=(agc_word)(w[0]+(w[0167]&0777u));buf1=agc_negate(highest);
 for(i=1;i<7u;++i){agc_word priority=w[0167u+i*12u];if(priority<040000u){buf2=(agc_word)(priority-1u);if(buf2>highest){highest=buf2;buf1=agc_negate(buf2);buf0=calls[i-1u];winner=i;q=03223;}else q=03216;}}
 if(winner){agc_word loc=e.core[winner][CM_EXEC_LOC],saved=e.core[winner][CM_EXEC_BANK];interpretive=(uint8_t)((loc&AGC_SIGN)!=0);if((saved&(agc_word)~076167u)||(!interpretive&&(!loc||loc>07777u))||(interpretive&&agc_negate(loc)>=07777u))return EXEC_OUT_OF_SCOPE;if(interpretive)e.core[winner][CM_EXEC_LOC]=agc_add_word(agc_negate(loc),1);}
 status=cm_exec_priority_basic(&e,w[0],resume,&result);if(status!=EXEC_OK)return status;
 if(interpretive){e.core[0][CM_EXEC_LOC]=result.location;e.core[0][CM_EXEC_BANK]=(agc_word)(result.bank&076007u);}
 cm_native_commit_executive(s,io,&e);if(!winner)w[0165]=agc_correct(oldl);w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;w[0]=03013;w[1]=02006;w[2]=q;w[5]=result.location;w[6]=(agc_word)(result.bank&076007u);w[3]=(agc_word)((w[6]&7u)*0400u);w[4]=(agc_word)(w[6]&076000u);if(winner)io->superbank=(agc_word)(result.bank&0160u);io->interrupt_enabled=1;
 if(interpretive){w[0115]=(agc_word)(w[6]&040000u);w[023]=0;w[0]=agc_extend(w[0115]);w[5]=06037;}
 next->interpretive=interpretive;next->address=result;return status;
}
