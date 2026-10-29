/* SPDX-License-Identifier: MIT */
#include "dsky_operator_response_check.h"
#if defined(DSKY_OPERATOR_RESPONSE_TARGET_LM)
#define FIRST 0u
#define MISSION 0u
#elif defined(DSKY_OPERATOR_RESPONSE_TARGET_CM)
#define FIRST 32u
#define MISSION 1u
#else
#error Select one source mission for this bounded allword target diagnostic.
#endif
volatile unsigned char dsky_operator_response_mailbox[5];static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;for(n=FIRST;n<FIRST+32u;++n)errors=(uint16_t)(errors+dsky_operator_response_check(n,&image,&before));dsky_operator_response_mailbox[1]=(unsigned char)errors;dsky_operator_response_mailbox[2]=(unsigned char)(errors>>8);dsky_operator_response_mailbox[3]=32;dsky_operator_response_mailbox[4]=MISSION;dsky_operator_response_mailbox[0]=0xa5;for(;;){}}
