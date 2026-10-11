/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "interpreted_sign_cases.h"
int main(void){uint32_t n;for(n=0;n<INTERPRETED_SIGN_CASES;++n){apollo_overlay_state m;agc_word irq;uint16_t row[3];interpreted_sign_input(&m,&irq,INTERPRETED_SIGN_LM,n);row[0]=m.words[0163];row[1]=m.words[0260];row[2]=m.words[0154];if(fwrite(row,sizeof row,1,stdout)!=1)return 2;}return 0;}
