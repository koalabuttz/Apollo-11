/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_difeq_cases.h"
static void row(const apollo_overlay_state *m,uint16_t n,uint16_t phase){uint16_t header[2];header[0]=n;header[1]=phase;(void)fwrite(header,sizeof header,1,stdout);(void)fwrite(m->words,sizeof m->words,1,stdout);}
int main(void){uint16_t n;for(n=0;n<CM_DIFEQ_CASES;++n){apollo_overlay_state m;uint8_t p,stage=cm_difeq_stage(n);cm_difeq_input(&m,n);row(&m,n,255);for(p=0;p<cm_difeq_phase_count(stage);++p){if(cm_difeq_phase(&m,stage,p)!=CM_DIFEQ_OK){fprintf(stderr,"Rejected case%u stage%u phase%u MODE%o PUSH%o\n",n,stage,p,m.words[0163],m.words[0166]);return 2;}row(&m,n,p);}}return ferror(stdout)?1:0;}
