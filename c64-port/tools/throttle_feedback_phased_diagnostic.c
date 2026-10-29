/* SPDX-License-Identifier: MIT */
#include <stdint.h>
unsigned throttle_feedback_phased_check(void);
#ifdef FEEDBACK_PHASE_DEBUG
extern uint16_t feedback_mismatches[24],feedback_trace[8];
#endif
int main(void){volatile uint8_t *mail=(volatile uint8_t *)0xc080;unsigned f;mail[0]=0;f=throttle_feedback_phased_check();mail[1]=(uint8_t)f;mail[2]=(uint8_t)(f>>8);mail[3]=32;mail[4]=FEEDBACK_PHASE_SELECTED;
#ifdef FEEDBACK_PHASE_DEBUG
{unsigned i;volatile uint16_t *dump=(volatile uint16_t *)0xc090;for(i=0;i<24u;++i)dump[i]=feedback_mismatches[i];for(i=0;i<8u;++i)dump[24u+i]=feedback_trace[i];}
#endif
mail[0]=0xa5;return f?1:0;}
