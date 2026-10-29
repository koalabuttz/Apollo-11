/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_charin_nv_cases.h"
#include "dsky_charin_nv_cases.h"
void cm_charin_nv_input(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,uint16_t n){
 static const uint16_t raw[]={0,077777,037777,040000,0100000,0177777,0140000,1},nout[]={0,077777,037777,040000,1,077776};
 dsky_charin_nv_prepare(s,n);s->words[5]=02000;s->words[1]=raw[(n/32u)%8u];s->words[01016]=nout[(n/256u)%6u];memset(io,0,sizeof *io);io->superbank=0100;io->interrupt_enabled=(uint16_t)(n%2u);io->channel11=dsky_charin_nv_channel11(n);*wrote=0;
}
