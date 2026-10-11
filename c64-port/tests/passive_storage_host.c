/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <stdint.h>
#if defined(PASSIVE_STORAGE_LM)
#include "lm_fixed_compat.h"
#define read_word lm_fixed_compat_read
#else
#include "cm_fixed_compat.h"
#define read_word cm_fixed_compat_read
#endif
int main(void){
 uint32_t address;uint16_t word;int valid;
 for(address=0;address<65536u;++address){
  word=0xdead;valid=read_word((uint16_t)address,&word);
  if(read_word((uint16_t)address,0))return 1;
  if(!valid&&word!=0xdead)return 2;
  if(putchar(valid)==EOF||putchar((int)(word&255u))==EOF||putchar(word>>8)==EOF)return 3;
 }
 return ferror(stdout)!=0;
}
