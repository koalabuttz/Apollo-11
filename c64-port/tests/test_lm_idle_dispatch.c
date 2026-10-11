/* SPDX-License-Identifier: MIT */
#include "lm_idle_dispatch_cases.h"
#include "lm_idle_start.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void){uint16_t n;unsigned test;
 for(n=0;n<2048u;++n){apollo_overlay_state a,b;p66_bias_io x,y;agc_word cx,cy;
  exec_address expected;exec_transfer actual;lm_idle_dispatch_input(&a,&x,&cx,n);
  if(x.interrupt_enabled!=1)continue;
  b=a;y=x;cy=cx;
  assert(lm_idle_start(&a,&x,&cx,&expected)==EXEC_OK);
  assert(lm_idle_dispatch(&b,&y,&cy,LM_IDLE_ADVAN,&actual)==EXEC_OK);
  assert(!memcmp(&a,&b,sizeof a)&&!memcmp(&x,&y,sizeof x)&&cx==cy);
  assert(expected.location==actual.address.location&&expected.bank==actual.address.bank&&actual.interpretive==0);
 }
 for(test=0;test<14u;++test){apollo_overlay_state image,before;p66_bias_io io,saved;
  agc_word channel,old_channel;exec_transfer next={{0123,0456},1},old_next;
  enum lm_idle_dispatch_entry entry=LM_IDLE_ADVAN;
  lm_idle_dispatch_input(&image,&io,&channel,3000);
  if(test==0)image.words[5]=0;
  if(test==1)image.words[7]=1;
  if(test==2)image.words[3]=1;
  if(test==3)image.words[4]=0;
  if(test==4)image.words[6]^=1;
  if(test==5)io.superbank=1;
  if(test==6)io.interrupt_enabled=2;
  if(test==7)image.words[067]=13;
  if(test==8)image.words[0164+image.words[067]]=0;
  if(test==9)image.words[0165+image.words[067]]=0100000;
  if(test==10)image.words[0167+image.words[067]]=0;
  if(test==11)entry=(enum lm_idle_dispatch_entry)3;
  if(test==12){image.words[067]=AGC_NEG_ZERO;image.words[01361]=0;}
  if(test==13){image.words[5]=03225;image.words[0]=0;entry=LM_IDLE_NUDIRECT;}
  before=image;saved=io;old_channel=channel;old_next=next;
  assert(lm_idle_dispatch(&image,&io,&channel,entry,&next)==EXEC_OUT_OF_SCOPE);
  assert(!memcmp(&image,&before,sizeof image)&&!memcmp(&io,&saved,sizeof io));
  assert(channel==old_channel&&!memcmp(&next,&old_next,sizeof next));
 }
 assert(lm_idle_dispatch(0,0,0,LM_IDLE_ADVAN,0)==EXEC_OUT_OF_SCOPE);
 puts("PASS:1024 old ready-basic idle states unchanged;14 mutation guards and null guard");
 return 0;}
