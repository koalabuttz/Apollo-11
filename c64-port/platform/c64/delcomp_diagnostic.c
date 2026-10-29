/* SPDX-License-Identifier: MIT */
#include "delcomp_cases.h"
#define DELCOMP_FROZEN_IMPLEMENTATION
#include "delcomp_vectors.h"
static apollo_overlay_state state,before;
static uint16_t expected_words[52];
int main(void){volatile unsigned char *mailbox=(volatile unsigned char *)0xc000;
 gimbal_io io;frozen_words reader;uint16_t row,n,i,k,base,matrix,expected,checked=0,super,irq;
 /* The larger fixture places BSS beneath BASIC ROM. Reveal its RAM before
  * reading it and exclude C64 IRQ producers while the canonical image runs. */
 __asm__("sei");*(volatile unsigned char *)1u&=0xfeu;mailbox[0]=0xff;
 for(row=0;row<DELCOMP_FROZEN_ROWS;++row){n=delcomp_target_ids[row];delcomp_prepare(&state,&io,n);before=state;
 base=state.words[0120];matrix=n<5120u?02230:03425;
 if(!delcomp_reference_begin(&reader,row))goto failed;
 for(k=0;k<52u;++k)if(!frozen_words_next(&reader,&expected_words[k]))goto failed;
 if(!frozen_words_next(&reader,&super)||!frozen_words_next(&reader,&irq)||!frozen_words_finished(&reader))goto failed;
 if(!delcomp_native(&state,&io,delcomp_mission(n)))goto failed;
 k=0;for(i=0;i<2048u;++i){expected=before.words[i];
 if(k<24u&&i==delcomp_changed[k])expected=expected_words[k++];
 if(i>=base&&i<base+10u)expected=expected_words[24u+i-base];
 if(i>=matrix&&i<matrix+18u)expected=expected_words[34u+i-matrix];
 if(state.words[i]!=expected)goto failed;}
 if(k!=24u||io.superbank!=super||io.interrupt_enabled!=irq)goto failed;++checked;}
 mailbox[1]=(unsigned char)checked;mailbox[2]=(unsigned char)(checked>>8);mailbox[0]=0xa5;for(;;){}
 failed:mailbox[1]=(unsigned char)checked;mailbox[2]=(unsigned char)(checked>>8);mailbox[0]=0xee;for(;;){}
}
