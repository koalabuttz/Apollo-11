/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "hardware_divide_cases.h"
#include "hardware_divide_vectors.h"
unsigned hardware_divide_check(void){unsigned i,failures=0;for(i=0;i<HARDWARE_DIVIDE_ROWS;++i){agc_register_pair p;agc_register d;hw_div_input(hardware_divide_indices[i],&p,&d);p=agc_hardware_divide(p,d);if(p.a!=hardware_divide_expected[i][0]||p.l!=hardware_divide_expected[i][1])++failures;}return failures;}
#ifndef HARDWARE_DIVIDE_CHECK_ONLY
int main(void){unsigned failures=hardware_divide_check();printf("%u complete hardware-DV register states: %s\n",HARDWARE_DIVIDE_ROWS,failures?"FAIL":"PASS");return failures?1:0;}
#endif
