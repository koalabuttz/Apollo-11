/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "lm_rcs_monitor_cases.h"
int main(void){uint32_t n;while(fread(&n,sizeof n,1,stdin)==1){apollo_overlay_state m;p66_bias_io io;agc_word ch;lm_rcs_monitor_input(&m,&io,&ch,n);if(fwrite(&io,sizeof io,1,stdout)!=1||fwrite(&ch,sizeof ch,1,stdout)!=1||fwrite(m.words,sizeof m.words,1,stdout)!=1)return 2;}return ferror(stdin)?2:0;}
