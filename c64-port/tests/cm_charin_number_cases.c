/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_charin_number_cases.h"
#include "dsky_charin_number_cases.h"
void cm_charin_number_input(apollo_overlay_state *s,cm_native_io *io,uint16_t *wrote,uint16_t n){
 dsky_charin_number_prepare(s,n);s->words[5]=02000;memset(io,0,sizeof *io);io->superbank=0100;io->interrupt_enabled=(uint16_t)(n%2u);io->channel11=dsky_charin_number_channel11(n);*wrote=0;
}
