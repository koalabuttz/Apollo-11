/* SPDX-License-Identifier: MIT */
#include "standard_lead_target.h"
#define STANDARD_LEAD_FROZEN_IMPLEMENTATION
#if STANDARD_LEAD_TARGET_CM
#include "standard_lead_vectors_cm.h"
#define MISSION STANDARD_LEAD_CM
#else
#include "standard_lead_vectors_lm.h"
#define MISSION STANDARD_LEAD_LM
#endif
static apollo_overlay_state state,expected;
int main(void){volatile unsigned char *box=(volatile unsigned char *)0xc000;uint16_t row,i,kind,n,phase,status,want_status,super,irq,count,index,value;frozen_words reader;p66_bias_io io;
 __asm__("sei");*(volatile unsigned char *)1u&=0xfeu;box[0]=0xff;
 for(row=0;row<STANDARD_LEAD_FROZEN_ROWS;++row){if(!standard_lead_reference_begin(&reader,row))goto failed;
  if(!frozen_words_next(&reader,&kind)||!frozen_words_next(&reader,&n)||!frozen_words_next(&reader,&phase)||!frozen_words_next(&reader,&want_status)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&count))goto failed;
  standard_lead_target_input(&state,&io,MISSION,kind,n);for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&index)||index>=2048u||!frozen_words_next(&reader,&value))goto failed;expected.words[index]=value;}
  /* Padding carries no expected words; the uniform lossless row is validated. */
  for(i=count;i<STANDARD_LEAD_TARGET_MAX_CHANGES;++i){if(!frozen_words_next(&reader,&index)||!frozen_words_next(&reader,&value)||index!=0||value!=0)goto failed;}
  if(!frozen_words_finished(&reader)||!standard_lead_target_execute(&state,&io,MISSION,kind,n,phase,&status)||status!=want_status||io.superbank!=super||io.interrupt_enabled!=irq)goto failed;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])goto failed;
 }
 box[1]=(unsigned char)row;box[2]=(unsigned char)(row>>8);box[0]=0xa5;for(;;){}
 failed:box[1]=(unsigned char)row;box[2]=(unsigned char)(row>>8);box[0]=0xee;for(;;){}
}
