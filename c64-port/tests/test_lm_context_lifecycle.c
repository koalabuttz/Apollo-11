/* SPDX-License-Identifier: MIT */
#include "lm_context_lifecycle_cases.h"
#include "lm_priority_cases.h"
#include "lm_sleep_cases.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){uint16_t n;unsigned entry;
 for(entry=0;entry<2u;++entry)for(n=0;n<8192u;++n){apollo_overlay_state a,b;
  p66_bias_io x,y;agc_word cx,cy;exec_address expected;exec_transfer actual;
  enum exec_status old_status,new_status;
  if(entry)lm_sleep_input(&a,&x,&cx,n);
  else{lm_priority_input(&a,&x,n);cx=(agc_word)((n*17u)&AGC_MASK);}
  b=a;y=x;cy=cx;
  old_status=entry?lm_basic_sleep(&a,&x,&cx,&expected):lm_basic_priority_change(&a,&x,&expected);
  new_status=lm_context_lifecycle(&b,&y,&cy,(enum lm_context_lifecycle_entry)entry,&actual);
  assert(old_status==new_status&&actual.interpretive==0);
  assert(expected.location==actual.address.location&&expected.bank==actual.address.bank);
  assert(!memcmp(&a,&b,sizeof a)&&!memcmp(&x,&y,sizeof x)&&cx==cy);
 }
 for(entry=0;entry<12u;++entry){apollo_overlay_state image,before;p66_bias_io io,saved;
  agc_word channel,old_channel;exec_transfer next={{0123,0456},1},old_next;
  enum lm_context_lifecycle_entry which=LM_CONTEXT_PRIORITY;
  lm_context_lifecycle_input(&image,&io,&channel,4096);
  /* Force a known winning slot before varying the selected-pair guards. */
  image.words[0]=01000;image.words[0167]=010401;image.words[0203]=037777;
  if(entry==0)image.words[5]=0;
  if(entry==1)image.words[7]=1;
  if(entry==2)image.words[3]=1;
  if(entry==3)image.words[4]=02001;
  if(entry==4)image.words[6]^=1;
  if(entry==5)io.superbank=1;
  if(entry==6)io.interrupt_enabled=2;
  if(entry==7)image.words[0167]=0;
  if(entry==8)image.words[0217]=0;
  if(entry==9)image.words[0200]=0;
  if(entry==10)image.words[0201]=1u<<15;
  if(entry==11)which=(enum lm_context_lifecycle_entry)2;
  before=image;saved=io;old_channel=channel;old_next=next;
  assert(lm_context_lifecycle(&image,&io,&channel,which,&next)==EXEC_OUT_OF_SCOPE);
  assert(!memcmp(&image,&before,sizeof image)&&!memcmp(&io,&saved,sizeof io));
  assert(channel==old_channel&&!memcmp(&next,&old_next,sizeof next));
 }
 assert(lm_context_lifecycle(0,0,0,LM_CONTEXT_PRIORITY,0)==EXEC_OUT_OF_SCOPE);
 puts("PASS:16384 frozen basic lifecycle states unchanged;12 all-object mutation guards and null guard");
 return 0;}
