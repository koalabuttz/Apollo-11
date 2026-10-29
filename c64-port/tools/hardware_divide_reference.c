/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only adapter; original assembly DV Q operand with pinned hardware DV. */
#define main admission_reference_main
#include "reference_runner.c"
#undef main
#include "hardware_divide_cases.h"
int main(void){uint32_t n;InhibitAlarms=1;for(n=0;n<HW_DIV_CASES;++n){agc_register_pair r;agc_register divisor;unsigned cycles;hw_div_input(n,&r,&divisor);memset(&machine,0,sizeof machine);*memory(RegA)=(int16_t)r.a;*memory(RegL)=(int16_t)r.l;*memory(RegQ)=(int16_t)divisor;set(RegZ,04000);machine.Fixed[2][0]=000006;machine.Fixed[2][1]=010002;for(cycles=0;cycles<100;++cycles){if(!machine.PendFlag&&!machine.ExtraDelay&&get(RegZ)==04002)break;if(agc_engine(&machine))return 2;}if(cycles==100)return 2;printf("%u %u %u\n",(unsigned)n,(unsigned)*memory(RegA)&0177777u,(unsigned)*memory(RegL)&0177777u);}return 0;}
