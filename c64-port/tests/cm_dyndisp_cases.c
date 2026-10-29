/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_dyndisp_cases.h"
#include "matrix_cases.h"
void cm_dyndisp_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){agc_word irq,super;matrix_input(s,&irq,&super,n,APOLLO_FIXED_CM,0);memset(io,0,sizeof *io);io->interrupt_enabled=irq;io->superbank=(agc_word)((n%4u)*020u);s->words[3]=03000;s->words[4]=050000;s->words[6]=050006;s->words[5]=03325;s->words[067]=0;s->words[0120]=(agc_word)(0401u+054u*(n%5u));s->words[0166]=s->words[0120];s->words[0163]=077776;}
