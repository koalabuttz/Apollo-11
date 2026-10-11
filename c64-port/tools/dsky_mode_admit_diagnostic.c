/* SPDX-License-Identifier: MIT */
#include "dsky_mode_admit_check.h"
#if defined(DSKY_MODE_ADMIT_TARGET_LM)
#define FIRST 0u
#define MISSION 0u
#elif defined(DSKY_MODE_ADMIT_TARGET_CM)
#define FIRST 32u
#define MISSION 1u
#else
#error Select one source mission for this bounded allword target diagnostic.
#endif
volatile unsigned char dsky_mode_admit_mailbox[5];static apollo_overlay_state image,before;
int main(void){uint16_t errors=0;uint8_t n;for(n=FIRST;n<FIRST+32u;++n)errors=(uint16_t)(errors+dsky_mode_admit_check(n,&image,&before));dsky_mode_admit_mailbox[1]=(unsigned char)errors;dsky_mode_admit_mailbox[2]=(unsigned char)(errors>>8);dsky_mode_admit_mailbox[3]=32;dsky_mode_admit_mailbox[4]=MISSION;dsky_mode_admit_mailbox[0]=0xa5;for(;;){}}
