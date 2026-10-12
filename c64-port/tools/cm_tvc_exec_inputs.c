/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_tvc_exec_cases.h"
int main(void){uint32_t n;while(fread(&n,sizeof n,1,stdin)==1){agc_image m;agc_word irq,ch11;cm_tvc_exec_input(&m,&irq,&ch11,n);if(fwrite(&irq,sizeof irq,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
