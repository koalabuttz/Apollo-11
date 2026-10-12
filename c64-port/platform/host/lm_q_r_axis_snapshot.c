/* SPDX-License-Identifier: MIT */
/* Native lm-q-r-axis cases in the oracle's format: case, outcome kind,
 * channel 13, the interrupt-enable and interrupt-in-progress bits and all
 * 2048 erasable words at the stop. A refused case prints kind
 * LM_QR_REFUSED and the unchanged input image;
 * tools/lm_q_r_axis_freeze.py leaves those rows out of the oracle
 * comparison. */
#include <stdio.h>
#include "lm_q_r_axis_cases.h"
int main(void){unsigned n,i,kind;
 for(n=0;n<LM_QR_CASES;++n){apollo_overlay_state s;lm_flight_io io;uint16_t steps;
  kind=lm_qr_run(&s,&io,(uint16_t)n,&steps);
  if(kind>=LM_QR_NONE){fprintf(stderr,"lm-q-r-axis case %u stopped at %05o\n",n,(unsigned)s.words[5]);return 1;}
  printf("%u %u %u %u %u",n,kind,(unsigned)io.chan13,(unsigned)io.bias.interrupt_enabled,(unsigned)io.in_isr);
  for(i=0;i<2048u;++i)printf(" %u",(unsigned)s.words[i]&(i<2u?0177777u:077777u));
  putchar('\n');
 }return 0;}
