/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_orbital_initial_cases.h"
static void row(const apollo_overlay_state *m,agc_word irq,uint16_t n,uint16_t phase){uint16_t header[2];header[0]=n;header[1]=phase;(void)fwrite(header,sizeof header,1,stdout);(void)fwrite(m->words,sizeof m->words,1,stdout);(void)fwrite(&irq,sizeof irq,1,stdout);}
int main(void){uint16_t n;for(n=0;n<CM_ORBITAL_INITIAL_CASES;++n){apollo_overlay_state m;agc_word irq;uint8_t p;cm_orbital_initial_input(&m,&irq,n);row(&m,irq,n,255);for(p=0;p<4u;++p){if(cm_orbital_initial_phase(&m,&irq,p)!=CM_ORBITAL_INITIAL_OK)return 2;row(&m,irq,n,p);}}return ferror(stdout)?1:0;}
