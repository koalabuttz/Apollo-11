/* SPDX-License-Identifier: MIT; original CM JOBWAKE/WAKETEST/SPECTEST. */
#include "cm_wake_complete.h"
enum exec_status cm_wake_complete(apollo_overlay_state *s,cm_native_io *io,exec_transfer *next){agc_word *w,wake,caller,oldfb,oldl,oldbb,flag,loc,bank,oldloc,oldbank,priority;uint8_t i,slot=7,hole=7;agc_register difference;enum exec_status status;
 if(!s||!io||!next)return EXEC_OUT_OF_SCOPE;
 w=s->words;wake=agc_correct(w[0]);caller=w[2];oldfb=w[4];oldbb=w[6];oldl=agc_correct(w[1]);flag=w[0067];
 if(w[5]!=05201u||w[0]!=agc_extend(wake)||caller<0100u||caller>07777u||w[3]>03400u||(w[3]&0377u)||(oldfb&~076000u)||oldbb!=(agc_word)(oldfb|(w[3]>>8))||w[7]||(io->superbank&~0160u)||io->interrupt_enabled>1u||io->executive_alarm||(flag!=AGC_NEG_ZERO&&(flag>72u||flag%12u)))return EXEC_OUT_OF_SCOPE;
 for(i=0;i<7u;++i){agc_word p=w[0167u+12u*i],cadr=w[0164u+12u*i];if(!p){hole=i;break;}if(p>=040000u&&p<AGC_NEG_ZERO&&(cadr==wake||((cadr==0||cadr==AGC_NEG_ZERO)&&(wake==0||wake==AGC_NEG_ZERO)))){slot=i;break;}}
 if(slot<7u&&(w[0165u+12u*slot]&~0167u))return EXEC_OUT_OF_SCOPE;
 io->interrupt_enabled=0;w[0065]=wake;w[061]=oldfb;w[4]=02000;w[6]=(agc_word)(02000u|(w[3]>>8));w[1]=agc_extend(oldl);
 if(hole<7u){w[0064]=(agc_word)(hole*12u);w[062]=(agc_word)(6u-hole);w[0]=0;w[2]=03055;w[5]=014;status=EXEC_OK;}
 else if(slot==7u){w[0064]=077776;w[062]=0;w[0]=agc_extend(oldfb);w[2]=06712;w[5]=caller;w[4]=oldfb;w[6]=oldbb;status=EXEC_NOT_FOUND;}
 else{
  agc_word displacement=(agc_word)(slot*12u),index=flag==AGC_NEG_ZERO?0:flag;
  w[0064]=displacement;w[062]=(agc_word)(6u-slot);oldloc=w[0164u+displacement];oldbank=w[0165u+displacement];priority=agc_negate(w[0167u+displacement]);w[0063]=priority;w[0167u+displacement]=priority;
  loc=(agc_word)((wake&01777u)+02000u);bank=(agc_word)((wake&076000u)+oldbank);w[0065]=loc;w[0066]=bank;
  if(slot==0u&&flag==0){w[0]=0;w[2]=02677;w[5]=05747;status=EXEC_OK;}
  else{
   w[0065]=(agc_word)(slot?displacement-1u:flag==AGC_NEG_ZERO?0:flag-1u);w[0066]=oldl;w[0164u+displacement]=loc;w[0165u+displacement]=bank;w[1]=agc_extend(oldbank);
   if(slot==0u&&flag==AGC_NEG_ZERO)w[0067]=0;
   else{difference=agc_add_register(agc_extend(agc_negate(w[0167u+index])),agc_extend(priority));if(difference!=0&&difference!=0177777u&&!(difference&0100000u))w[0067]=displacement;}
   (void)oldloc;w[0]=agc_extend(oldfb);w[2]=06712;w[5]=caller;w[4]=oldfb;w[6]=oldbb;status=EXEC_OK;
  }
 }
 next->interpretive=0;next->address=(exec_address){w[5],(agc_word)(w[6]|io->superbank)};return status;
}
