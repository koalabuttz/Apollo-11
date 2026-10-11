/* SPDX-License-Identifier: MIT */
#include "lm_interbank_cases.h"
#include "lm_interbank_target.h"
#define LM_INTERBANK_FROZEN_IMPLEMENTATION
#include "lm_interbank_vectors.h"
static apollo_overlay_state state,expected;
int lm_interbank_target_check(unsigned *completed){uint16_t row,i,index,value,nlo,nhi,kind,literal,word,status,super,irq,count,lo,hi;p66_bias_io io;frozen_words reader;
 *completed=0;
 value=0;if(!lm_interbank_literal(04644u,&value)||value!=012000u||lm_interbank_literal(04643u,&value)||value!=012000u||lm_interbank_literal(04644u,0))return 9;
 for(row=0;row<LM_INTERBANK_FROZEN_ROWS;++row){
  if(!lm_interbank_reference_begin(&reader,row))return 1;
  if(!frozen_words_next(&reader,&nlo)||!frozen_words_next(&reader,&nhi)||!frozen_words_next(&reader,&kind)||!frozen_words_next(&reader,&literal)||!frozen_words_next(&reader,&word)||!frozen_words_next(&reader,&status)||!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&irq)||!frozen_words_next(&reader,&count))return 2;
  if(lm_interbank_input(&state,&io,(uint32_t)nlo|((uint32_t)nhi<<16))!=literal)return 3;
  for(i=0;i<2048u;++i)expected.words[i]=state.words[i];
  for(i=0;i<count;++i){if(!frozen_words_next(&reader,&index)||index>=2048u||!frozen_words_next(&reader,&value))return 4;expected.words[index]=(agc_word)value;}
  for(i=count;i<LM_INTERBANK_MAX_CHANGES;++i)if(!frozen_words_next(&reader,&lo)||!frozen_words_next(&reader,&hi)||lo!=0||hi!=0)return 5;
  if(!frozen_words_finished(&reader))return 6;
  if((unsigned)!lm_interbank(&state,&io,(enum lm_interbank_entry)kind,(agc_word)literal,(agc_word)word)!=status||io.superbank!=super||io.interrupt_enabled!=irq)return 7;
  for(i=0;i<2048u;++i)if(state.words[i]!=expected.words[i])return 8;
  *completed=row+1u;
 }
 return 0;
}
