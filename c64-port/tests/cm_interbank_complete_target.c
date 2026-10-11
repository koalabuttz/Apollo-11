/* SPDX-License-Identifier: MIT */
#include "cm_interbank_complete_cases.h"
#include "cm_interbank_complete_target.h"
#define CM_INTERBANK_COMPLETE_FROZEN_IMPLEMENTATION
#include "cm_interbank_complete_vectors.h"
static apollo_overlay_state state,expected;
int cm_interbank_complete_target_check(unsigned *completed){uint16_t row,i,index,value,nlo,nhi,kind,literal,word,status,super,irq,count,lo,hi;cm_native_io io,prior;frozen_words reader;
 *completed=0;
 value=0;if(!cm_interbank_complete_literal(04710u,&value)||value!=012000u||cm_interbank_complete_literal(04707u,&value)||value!=012000u||cm_interbank_complete_literal(04710u,0))return 9;
 for(row=0;row<CM_INTERBANK_COMPLETE_FROZEN_ROWS;++row){
  if(!cm_interbank_complete_reference_begin(&reader,row))return 1;
  if(!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&kind)||!frozen_words_next(&reader,&literal)||!frozen_words_next(&reader,&word)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&count))return 2;
  if(cm_interbank_complete_input(&state,&io,(uint32_t)nlo|((uint32_t)nhi<<16))!=literal)return 3;
  prior=io;
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&index)||index>=2048u||!frozen_words_next(&reader,&value))return 4;expected.words[index]=(agc_word)value;}
  for(i=count;i<CM_INTERBANK_COMPLETE_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo!=0||hi!=0)return 5;
  if(!frozen_words_finished(&reader))return 6;
  if((unsigned)!cm_interbank_complete(&state,&io,(enum cm_interbank_complete_entry)kind,(agc_word)literal,(agc_word)word)!=status||io.superbank!=super||io.interrupt_enabled!=irq)return 7;
  if(io.channel11!=prior.channel11||io.channel12!=prior.channel12||io.channel13!=prior.channel13||io.channel14!=prior.channel14||io.main_keys!=prior.main_keys||io.navigation_keys!=prior.navigation_keys||io.trap31a!=prior.trap31a||io.trap31b!=prior.trap31b||io.trap32!=prior.trap32||io.executive_alarm!=prior.executive_alarm||io.waitlist_alarm!=prior.waitlist_alarm)return 10;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 8;
  *completed=row+1u;
 }
 return 0;
}
