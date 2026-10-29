/* SPDX-License-Identifier: MIT
 * Real C64 GETIN queue → channel15 mapping → queued native CHARIN jobs. */
#include "dsky_keyboard_number_flow_check.h"
#include "c64_dsky_keyboard.h"
static const uint8_t characters[]={'0','1','2','3','4','5','6','7','8','9','V','R','K','+','-',13,'C','N',20,0xd6,0xd2,0xcb,0xc3,0xce};
static const uint16_t codes[]={020,1,2,3,4,5,6,7,010,011,021,022,031,032,033,034,036,037,036,021,022,031,036,037};
volatile unsigned char dsky_keyboard_number_flow_mailbox[5];
int main(void){uint8_t n,raw;uint16_t failures=0,code;
 __asm__ volatile("cli" : : : "memory");
 for(n=0;n<sizeof characters;++n){code=077777;*(volatile uint8_t *)0xc6=1;*(volatile uint8_t *)0x277=characters[n];if(c64_dsky_keyboard_poll(&code,&raw)!=C64_DSKY_KEY_READY || raw!=characters[n] || code!=codes[n])++failures;}
 code=077777;*(volatile uint8_t *)0xc6=0;if(c64_dsky_keyboard_poll(&code,&raw)!=C64_DSKY_KEY_EMPTY || code!=077777 || raw!=0)++failures;
 *(volatile uint8_t *)0xc6=1;*(volatile uint8_t *)0x277='?';if(c64_dsky_keyboard_poll(&code,&raw)!=C64_DSKY_KEY_UNMAPPED || code!=077777 || raw!='?')++failures;
 for(n=0;n<DSKY_KEYBOARD_NUMBER_FLOW_TARGET_COUNT;++n)failures=(uint16_t)(failures+dsky_keyboard_number_flow_check(n));
 dsky_keyboard_number_flow_mailbox[1]=(unsigned char)failures;dsky_keyboard_number_flow_mailbox[2]=(unsigned char)(failures>>8);dsky_keyboard_number_flow_mailbox[3]=8;dsky_keyboard_number_flow_mailbox[4]=3;dsky_keyboard_number_flow_mailbox[0]=0xa5;for(;;){}}
