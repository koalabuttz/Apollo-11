/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_delay_image_cases.h"
#include "lm_waitlist_image_cases.h"
static int reject(unsigned n){apollo_overlay_state s,before;p66_bias_io io,oldio;agc_word delay;uint16_t *w=s.words;enum lm_delay_entry entry=LM_FIXDELAY_ENTRY;uint8_t internal=(uint8_t)(n>=16u);
 delay=lm_delay_image_input(&s,&io,internal?2u:0u);
 switch(n){case 0:w[5]++;break;case 1:w[7]=1;break;case 2:w[3]=04000;break;
 case 3:w[3]|=1;break;case 4:w[4]|=1;break;case 5:w[6]^=1;break;
 case 6:io.superbank=1;break;case 7:io.interrupt_enabled=1;break;
 case 8:w[2]=077;break;case 9:w[2]=07776;break;case 10:w[026]=040000;break;
 case 11:entry=(enum lm_delay_entry)2;break;case 12:w[0705]=16251;break;
 case 13:w[2]=03200;delay=0100000;break;
 case 14:delay=lm_delay_image_input(&s,&io,1);entry=LM_VARDELAY_ENTRY;w[0]=040000;break;
 case 15:delay=lm_delay_image_input(&s,&io,1);entry=LM_VARDELAY_ENTRY;w[0]=16251;break;
 case 16:w[5]++;break;case 17:w[7]=1;break;case 18:w[3]=01000;break;
 case 19:w[4]=04000;break;case 20:w[6]^=1;break;case 21:io.superbank=1;break;
 case 22:io.interrupt_enabled=1;break;case 23:w[0]=040000;break;
 case 24:w[2]=040000;break;case 25:w[063]=077;break;case 26:w[063]=010000;break;
 case 27:w[1]|=01000;break;case 28:w[0]|=01000;break;case 29:w[026]=040000;break;
 case 30:w[2]=16251;break;default:return 0;}
 before=s;oldio=io;return (internal?lm_waitlist_internal(&s,&io):lm_delay_image(&s,&io,entry,delay))==WL_OUT_OF_SCOPE&&!memcmp(&s,&before,sizeof s)&&!memcmp(&io,&oldio,sizeof io);
}
int main(void){unsigned n;apollo_overlay_state s,before;p66_bias_io io,oldio;
 for(n=0;n<31u;++n)if(!reject(n)){fprintf(stderr,"guard%u failed\n",n);return 1;}
 if(lm_delay_image(0,&io,LM_FIXDELAY_ENTRY,1)!=WL_OUT_OF_SCOPE||lm_delay_image(&s,0,LM_FIXDELAY_ENTRY,1)!=WL_OUT_OF_SCOPE||lm_waitlist_internal(0,&io)!=WL_OUT_OF_SCOPE||lm_waitlist_internal(&s,0)!=WL_OUT_OF_SCOPE)return 2;
 for(n=0;n<LM_WAITLIST_IMAGE_CASES;++n){exec_address task=lm_waitlist_image_input(&s,&io,(uint16_t)n);enum wl_status a,b;agc_word caller=s.words[2],bank=s.words[6],delay=agc_correct(s.words[0]);before=s;oldio=io;
  a=lm_waitlist_image(&s,&io,(enum lm_waitlist_entry)(n&1u),task);
  if(n&1u){caller=(agc_word)(caller-1u);task.bank=(agc_word)(bank|oldio.superbank);}
  before.words[061]=caller;before.words[063]=task.location;before.words[0]=agc_extend(bank);
  before.words[1]=agc_extend(task.bank);before.words[2]=agc_extend(delay);
  before.words[3]=01400;before.words[4]=02000;before.words[6]=02003;before.words[5]=03231;oldio.interrupt_enabled=0;
  b=lm_waitlist_internal(&before,&oldio);
  if(a!=b||memcmp(&s,&before,sizeof s)||memcmp(&io,&oldio,sizeof io)){fprintf(stderr,"compatibility%u failed\n",n);return 3;}
 }
 puts("PASS:16384 frozen WAITLIST/TWIDDLE body equivalences;31 all-object delay/WAIT2 mutation guards and4 null interfaces");return 0;
}
