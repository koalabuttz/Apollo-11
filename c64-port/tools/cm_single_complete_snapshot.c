/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_single_return_cases.h"
int main(void){uint16_t n;for(n=0;n<CM_SINGLE_RETURN_CASES;++n){apollo_overlay_state m;agc_word irq;uint16_t header[4]={n,(uint16_t)(n/3072u),1,0};cm_single_return_input(&m,&irq,n);if(cm_single_trig(&m,(uint8_t)(n/3072u))!=CM_TRIG_OK||cm_single_return(&m)!=CM_TRIG_OK)return 2;header[3]=irq;if(fwrite(header,sizeof header,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return 0;}
