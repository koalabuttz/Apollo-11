/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "acceleration_phased_cases.h"
#include "guidance_control_observer.h"
int main(void){uint16_t number;for(number=0;number<ACC_CASES;++number){acceleration_case c;uint8_t phase;agc_word q=01030;acc_input(&c,number);for(phase=0;phase<4u;++phase){p66_preparation *s=&c.previous.base.previous.previous.previous.preparation;agc_dp before={s->mpac[0],s->mpac[1]};unsigned result=acc_phase(&c,phase);if(result)return 2;q=p66_acceleration_return_q(phase,q,before,s->rate.desired,c.taurod,&c.previous.norm,c.previous.base.previous.previous.previous.scratch.mptemp);printf("%u %u %u\n",number,phase,q);}}return 0;}
