/* SPDX-License-Identifier: MIT */
#include "dsky_load_request_check.h"
#if defined(DSKY_LOAD_REQUEST_TARGET_LM)
#define FIRST 0u
#define MISSION 0u
#elif defined(DSKY_LOAD_REQUEST_TARGET_CM)
#define FIRST 32u
#define MISSION 1u
#else
#error Select one source mission for this bounded allword target diagnostic.
#endif
volatile unsigned char dsky_load_request_mailbox[5];static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;for(n=FIRST;n<FIRST+32u;++n)errors=(uint16_t)(errors+dsky_load_request_check(n,&image,&before));dsky_load_request_mailbox[1]=(unsigned char)errors;dsky_load_request_mailbox[2]=(unsigned char)(errors>>8);dsky_load_request_mailbox[3]=32;dsky_load_request_mailbox[4]=MISSION;dsky_load_request_mailbox[0]=0xa5;for(;;){}}
