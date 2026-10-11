/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "guidance_control_observer.h"
#include "control_observer_vectors.h"
unsigned control_observer_check(void){unsigned row,failures=0;for(row=0;row<CONTROL_OBSERVER_ROWS;++row){const uint16_t *w=control_observer_vectors[row];p66_norm_scratch n;agc_dp a={w[2],w[3]},b={w[4],w[5]},c={w[6],w[7]};n.addrwd=w[8];if(p66_acceleration_return_q((uint8_t)w[0],w[1],a,b,c,&n,w[9])!=w[10])++failures;}return failures;}
#ifndef CONTROL_OBSERVER_CHECK_ONLY
int main(void){unsigned f=control_observer_check();printf("%u complete original arithmetic Q states: %s\n",CONTROL_OBSERVER_ROWS,f?"FAIL":"PASS");return f?1:0;}
#endif
