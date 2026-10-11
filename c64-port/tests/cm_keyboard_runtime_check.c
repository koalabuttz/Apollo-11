/* SPDX-License-Identifier: MIT */
#include "cm_keyboard_runtime_check.h"
#include "cm_keyboard_runtime_vectors.h"
#ifdef __mos__
#include "c64_dsky_keyboard.h"
#endif
uint16_t cm_keyboard_runtime_check(uint8_t n,apollo_overlay_state *s,apollo_overlay_state *want){uint16_t j,i,p,end,errors=0,isr,key,wrote=0,meta[8],number=cm_keyboard_runtime_numbers[n];cm_native_io io;exec_transfer next={{0,0},0};
 cm_keyboard_runtime_input(s,&io,&isr,&key,number);
#ifdef __mos__
 {uint16_t code=077777;uint8_t raw=0,character=(uint8_t)(key==021 ? 'V' : key==037 ? 'N' : key==020 ? '0' : '0'+key);
  *(volatile uint8_t *)0x00c6=1;*(volatile uint8_t *)0x0277=character;
  if(c64_dsky_keyboard_poll(&code,&raw)!=C64_DSKY_KEY_READY || code!=key || raw!=character)++errors;
  key=code;
 }
#endif
 *want=*s;meta[0]=io.superbank;meta[1]=io.interrupt_enabled;meta[2]=io.channel11;meta[3]=isr;for(i=4;i<8;++i)meta[i]=0;
 for(j=0;j<4;++j){uint16_t selector=j==0 ? CM_KEYBOARD_IRQ : j==1 ? CM_KEYBOARD_SELECT : j==3 ? CM_KEYBOARD_FINISH : number%4u<2u ? CM_KEYBOARD_NV : CM_KEYBOARD_NUMBER;enum exec_status status;
 p=cm_keyboard_runtime_offsets[n*4u+j];end=cm_keyboard_runtime_offsets[n*4u+j+1u];for(;p<end;++p){uint16_t a=cm_keyboard_runtime_patch[p][0],v=cm_keyboard_runtime_patch[p][1];if(a<2048u)want->words[a]=v;else meta[a-2048u]=v;}
 status=cm_keyboard_runtime_entry(s,&io,&isr,key,selector,&next,&wrote);if(status!=(j==3 ? EXEC_IDLE : EXEC_OK))++errors;
 for(i=0;i<2048;++i)if(s->words[i]!=want->words[i])++errors;
 if(io.superbank!=meta[0]||io.interrupt_enabled!=meta[1]||io.channel11!=meta[2]||isr!=meta[3]||wrote!=meta[4]||next.address.location!=meta[5]||next.address.bank!=meta[6]||next.interpretive!=meta[7])++errors;
 }return errors;}
