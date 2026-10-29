/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "dsky_octal_verb_check.h"
static apollo_overlay_state s,before;
static enum p66_bias_status apply(p66_bias_io *io,enum dsky_nv_mission mission,uint8_t entry,agc_word value,enum dsky_octal_verb_route *route){return entry==2u?dsky_octal_verb_channel(&s,io,mission,value,route):entry?dsky_octal_verb_return(&s,io,mission,route):dsky_octal_verb_begin(&s,io,mission,route);}
int main(void){p66_bias_io io,old;enum dsky_nv_mission mission;enum dsky_octal_verb_route route,prior;uint8_t entry,n,g,e;uint16_t failures=0,guards=0;agc_word value;
 for(n=0;n<128u;++n)failures=(uint16_t)(failures+dsky_octal_verb_checkpoint(n,&s,&before));
 for(n=0;n<2u;++n)for(e=0;e<3u;++e)for(g=0;g<12u;++g){dsky_octal_verb_input(&s,&io,(uint16_t)(n*8192u+e*256u+1u),&mission,&entry);value=s.words[01020];route=prior=DSKY_VERB_CHANNEL;
 switch(g){case 0:s.words[5]=0;break;case 1:s.words[4]=0;break;case 2:s.words[3]=1;break;case 3:s.words[6]^=1;break;case 4:s.words[7]=1;break;case 5:io.superbank=0;break;case 6:io.superbank=0101;break;case 7:io.superbank=0200;break;case 8:io.interrupt_enabled=2;break;case 9:mission=(enum dsky_nv_mission)2;break;case 10:if(e==1u)s.words[0122]=3;else s.words[01001]=0;break;default:if(e==0u)s.words[0140]=0;else if(e==1u)s.words[0122]=077777;else value=0100000;break;}
 before=s;old=io;++guards;if(apply(&io,mission,entry,value,&route)!=P66_BIAS_OUT_OF_SCOPE||memcmp(&s,&before,sizeof s)||memcmp(&io,&old,sizeof io)||route!=prior)++failures;
 }
 printf("128 complete original octal verb states + %u unchanged invalid guards: %s\n",guards,failures?"FAIL":"PASS");return failures?1:0;
}
