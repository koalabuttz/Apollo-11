/* SPDX-License-Identifier: MIT; original Comanche055 EJSCAN groups. */
#include "cm_scan_zero.h"
enum exec_status cm_scan_zero(apollo_overlay_state *s,cm_native_io *io,exec_transfer *next){
 agc_word *w,pc,base,highest,buf0,buf1,buf2,q,oldbb;
 uint8_t i,hole=0,slot3zero;
 enum exec_status status;
 static const agc_word calls[6]={03143,03147,03153,03157,03163,03167};
 if(!s||!io||!next)return EXEC_OUT_OF_SCOPE;
 w=s->words;pc=w[5];base=(agc_word)(w[0167]&0777u);oldbb=w[6];
 if((pc!=05217u&&pc!=05175u&&pc!=05210u)||w[3]>03400u||(w[3]&0377u)||w[4]>076000u||(w[4]&01777u)||oldbb!=(agc_word)(w[4]|(w[3]>>8))||w[7]||!w[0167]||w[0167]>=040000u||(io->superbank&~0160u)||io->interrupt_enabled>1u||io->channel11>AGC_MASK||io->executive_alarm||w[2]<0100u||w[2]>07777u)return EXEC_OUT_OF_SCOPE;
 if(pc==05210u&&(!w[0]||(w[0]&0140777u)))return EXEC_OUT_OF_SCOPE;
 if(pc==05175u&&w[0]!=agc_extend(agc_correct(w[0])))return EXEC_OUT_OF_SCOPE;
 if(pc==05217u&&base>CM_EXEC_FAKEPRET&&(base<0401u||base>0661u||(base-0401u)%054u))return EXEC_OUT_OF_SCOPE;
 for(i=1;i<7u;++i)if(i!=3u&&!w[0167u+12u*i]){hole=i;break;}
 slot3zero=(uint8_t)(w[0233]==0);
 if(!hole){
  /* The source's slot-three +0 successor is the -CCSPR word, followed by
   * TCF+1. Its AD result is overwritten by the following CCS and has no
   * retained effect at the completed source boundary. A transient -0 lets
   * the proven ordinary exchange run, then restore the actual +0 slot. */
  if(slot3zero)w[0233]=AGC_NEG_ZERO;
  if(pc==05217u)status=cm_basic_finish(s,io,next);
  else if(pc==05175u)status=cm_basic_sleep(s,io,next);
  else status=cm_priority_complete(s,io,next);
  if(slot3zero)w[0233]=0;
  return status;
 }
 buf0=w[0130];buf1=AGC_NEG_ZERO;buf2=w[0132];q=w[2];highest=0;
 w[1]=agc_extend(agc_correct(w[1]));
 io->interrupt_enabled=0;
 if(pc==05210u){
  w[0063]=w[0];w[6]=02006;w[3]=03000;w[4]=02000;
  w[0165]=oldbb;w[0164]=agc_correct(q);buf0=0;
  highest=(agc_word)(w[0063]+base);w[0167]=highest;buf1=agc_negate(highest);
 }else{
  w[4]=02000;w[6]=(agc_word)(02000u|(w[3]>>8));
  if(pc==05175u){w[0164]=agc_correct(w[0]);w[0167]=agc_negate(w[0167]);w[0165]=(agc_word)((w[6]&0177u)|io->superbank);}
  else{w[0167]=AGC_NEG_ZERO;w[1]=base;if(base>CM_EXEC_FAKEPRET)w[base-1u]=(agc_word)(base-1u);}
 }
 for(i=1;i<hole;++i){agc_word priority=w[0167u+i*12u];if(priority&&priority<040000u){buf2=(agc_word)(priority-1u);if(buf2>highest){highest=buf2;buf1=agc_negate(buf2);buf0=calls[i-1u];q=03223;}else q=03216;}}
 w[0130]=buf0;w[0131]=buf1;w[0132]=buf2;w[0]=0;w[2]=(agc_word)(calls[hole-1u]+1u);w[5]=05747;
 next->interpretive=0;next->address.location=05747;next->address.bank=(agc_word)(w[6]|io->superbank);
 return EXEC_OK;
}
