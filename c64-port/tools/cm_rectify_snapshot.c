/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_rectify_cases.h"
static void row(const apollo_overlay_state *m,uint16_t n,uint16_t phase){uint16_t header[2];header[0]=n;header[1]=phase;(void)fwrite(header,sizeof header,1,stdout);(void)fwrite(m->words,sizeof m->words,1,stdout);}
int main(void){uint16_t n;for(n=0;n<CM_RECTIFY_CASES;++n){apollo_overlay_state m;uint8_t p;cm_rectify_input(&m,n);row(&m,n,12);for(p=cm_rectify_first(n);p<12u;++p){if(cm_rectify_phase(&m,p)!=CM_RECTIFY_OK)return 2;row(&m,n,p);}}return ferror(stdout)?1:0;}
