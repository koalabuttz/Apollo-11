/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_quality_cases.h"
int main(void){uint32_t n;while(fread(&n,sizeof n,1,stdin)==1){apollo_overlay_state m;p66_bias_io io;cm_quality_input(&m,&io,n);if(fwrite(&io,sizeof io,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
