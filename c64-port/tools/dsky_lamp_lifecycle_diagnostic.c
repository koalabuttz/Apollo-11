/* SPDX-License-Identifier: MIT */
#include "dsky_lamp_lifecycle_check.h"
#if defined(DSKY_LAMP_LIFECYCLE_TARGET_LM)
#define FIRST 0u
#define MISSION 0u
#elif defined(DSKY_LAMP_LIFECYCLE_TARGET_CM)
#define FIRST 32u
#define MISSION 1u
#else
#error Select one source mission for this bounded allword target diagnostic.
#endif
volatile unsigned char dsky_lamp_lifecycle_mailbox[5];static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;for(n=FIRST;n<FIRST+32u;++n)errors=(uint16_t)(errors+dsky_lamp_lifecycle_check(n,&image,&before));dsky_lamp_lifecycle_mailbox[1]=(unsigned char)errors;dsky_lamp_lifecycle_mailbox[2]=(unsigned char)(errors>>8);dsky_lamp_lifecycle_mailbox[3]=32;dsky_lamp_lifecycle_mailbox[4]=MISSION;dsky_lamp_lifecycle_mailbox[0]=0xa5;for(;;){}}
