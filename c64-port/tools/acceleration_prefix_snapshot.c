/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "radial_cases.h"
#include "guidance_acceleration.h"
#include "acceleration_prefix.h"
static void emit(radial_case *c,agc_dp tau,uint16_t n,uint8_t step,uint8_t stage,unsigned status){uint16_t w[RA_WORDS];unsigned i;ra_words(c,w);printf("%u %u %u %u",n,step,stage,status);for(i=0;i<RA_WORDS;++i)printf(" %u",w[i]);printf(" %u %u\n",tau.high,tau.low);}
int main(void){uint16_t n;for(n=0;n<RA_CASES;++n){radial_case c;agc_dp tau=acc_prefix_time(n);uint8_t step;ra_prepare(&c,n);for(step=0;step<ra_steps(n);++step){unsigned status;ra_next(&c,n,step);if(ra_integrated(n))ra_prefix(&c,n);emit(&c,tau,n,step,0,0);status=(unsigned)ra_apply(&c);emit(&c,tau,n,step,1,status);if(status!=P66_RADIAL_ALARM_1302)status=(unsigned)p66_acceleration(&c.previous.previous.previous.previous.previous.previous.preparation,&c.previous.previous.previous.previous.previous.previous.scratch,&c.previous.previous.previous.previous.elapsed,&c.previous.previous.previous.scratch,&c.norm,&c.previous.previous.previous.previous.previous.push,&tau,c.previous.previous.gravity);emit(&c,tau,n,step,2,status);}}return 0;}
