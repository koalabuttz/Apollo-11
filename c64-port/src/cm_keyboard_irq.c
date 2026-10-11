/* SPDX-License-Identifier: MIT */
#include "cm_keyboard_irq.h"
static uint8_t available(const apollo_overlay_state *s,const cm_native_io *io){cm_executive e;
 cm_native_gather_executive(s,io,&e);return (uint8_t)(cm_exec_admit(&e,030000,02000,060101,0)==EXEC_OK);
}
static uint8_t common(const apollo_overlay_state *s,const cm_native_io *io){const uint16_t *w=s->words;
 return (uint8_t)(w[3]<=03400 && (w[3]&0377u)==0 && w[6]==(w[4]|(w[3]>>8)) && io->superbank<=0160 && (io->superbank&017u)==0 && io->interrupt_enabled<=1 && available(s,io));
}
static enum exec_status keycom(apollo_overlay_state *s,cm_native_io *io){uint16_t *w=s->words,key=w[0],old_q=w[2];enum exec_status status;exec_address job={02000,060101};
 w[0]=030000;w[2]=03710;status=cm_admission_image(s,io,job,0);
 if(status!=EXEC_OK){w[0]=key;w[2]=old_q;return EXEC_OUT_OF_SCOPE;}
 /* KEYTEMP/FLAGWRD5 are disjoint from the admission view. Committing them
  * after the supported source call avoids partial unsupported mutation. */
 w[073]=key;w[0101]=(uint16_t)(w[0101]|040000u);w[0154+w[064]]=key;w[0]=key;w[2]=03716;w[5]=05333;w[7]=0;return EXEC_OK;
}
enum exec_status cm_keyboard_keycom(apollo_overlay_state *s,cm_native_io *io){
 if(s->words[5]!=03702 || s->words[0]>037 || s->words[4]!=016000 || !common(s,io))return EXEC_OUT_OF_SCOPE;
 s->words[1]=agc_extend(agc_correct(s->words[1]));return keycom(s,io);
}
static enum exec_status keyrupt(apollo_overlay_state *s,cm_native_io *io,uint16_t channel15){uint16_t *w=s->words,old_sample=w[014];
 w[016]=agc_correct(w[0]);w[012]=agc_correct(w[2]);
 if(agc_overflow(w[0])!=0)w[012]=agc_overflow(w[0])>0 ? 1u : 077776u;
 if(agc_overflow(w[0])!=0 || agc_overflow(w[2])==0){w[013]=w[024];w[014]=w[025];w[1]=agc_extend(old_sample);}
 w[0]=(uint16_t)(channel15&037u);return keycom(s,io);
}
enum exec_status cm_keyboard_keyrupt(apollo_overlay_state *s,cm_native_io *io,uint16_t channel15){
 if(s->words[5]!=03673 || channel15>077777 || s->words[4]!=016000 || !common(s,io))return EXEC_OUT_OF_SCOPE;
 s->words[1]=agc_extend(agc_correct(s->words[1]));return keyrupt(s,io,channel15);
}
enum exec_status cm_keyboard_interrupt(apollo_overlay_state *s,cm_native_io *io,uint16_t *in_interrupt,uint16_t channel15){uint16_t *w=s->words,old_bank=w[6],old_lrupt=w[011];agc_word old_qrupt,a,l;enum exec_status status;
 if(in_interrupt==0 || *in_interrupt!=1 || channel15>077777 || w[5]!=04024 || w[4]>076000 || (w[4]&01777u)!=0 || w[015]==0 || w[015]>010000 || !common(s,io))return EXEC_OUT_OF_SCOPE;
 w[010]=agc_correct(w[0]);w[011]=agc_correct(w[1]);w[1]=agc_extend(old_lrupt);w[0]=agc_extend(old_bank);w[3]=0;w[4]=016000;w[6]=016000;w[5]=03673;
 status=keyrupt(s,io,channel15);if(status!=EXEC_OK)return status;
 old_qrupt=w[012];w[012]=agc_correct(w[2]);w[2]=agc_extend(old_qrupt);
 a=w[010];l=w[011];w[010]=016000;w[011]=agc_correct(w[1]);w[0]=agc_extend(a);w[1]=agc_extend(l);
 w[6]=(uint16_t)(w[016]&076007u);w[3]=(uint16_t)((w[6]&7u)<<8);w[4]=(uint16_t)(w[6]&076000u);w[5]=(uint16_t)(w[015]-1u);w[7]=0;io->interrupt_enabled=1;*in_interrupt=0;return status;
}
