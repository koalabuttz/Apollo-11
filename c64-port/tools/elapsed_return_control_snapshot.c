/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "elapsed_return_control_cases.h"
int main(void){uint16_t n;for(n=0;n<EQ_CASES;++n){apollo_overlay_state image;elapsed_case c;unsigned i;
 agc_dp before,piptime;agc_word q;eq_prepare(&image,n);memset(&c,0,sizeof c);
 before.high=image.words[0154];before.low=image.words[0155];piptime.high=image.words[01234];piptime.low=image.words[01235];q=image.words[2];
 eq_gather(&c,&image);(void)p66_elapsed_time(&c.previous.previous.preparation,&c.previous.previous.scratch,&c.elapsed);eq_commit(&image,&c);image.words[5]=06061;
 if(p66_elapsed_return_control(&image,q,before,piptime))return 2;
 printf("%u",n);for(i=0;i<2048u;++i)printf(" %u",image.words[i]);printf(" %u %u\n",(n&2u)?060u:0u,n&1u);
 }return 0;}
