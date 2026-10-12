/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_tvc_massprop_cases.h"
int main(void){uint32_t n;while(fread(&n,sizeof n,1,stdin)==1){agc_image m;agc_word irq;cm_tvc_massprop_input(&m,&irq,n);if(fwrite(&irq,sizeof irq,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
