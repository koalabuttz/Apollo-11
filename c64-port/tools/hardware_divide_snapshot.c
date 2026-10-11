/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "hardware_divide_cases.h"
int main(void){uint32_t n;for(n=0;n<HW_DIV_CASES;++n){agc_register_pair r;agc_register divisor;hw_div_input(n,&r,&divisor);r=agc_hardware_divide(r,divisor);printf("%u %u %u\n",(unsigned)n,r.a,r.l);}return 0;}
