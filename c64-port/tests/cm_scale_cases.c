/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "cm_scale_cases.h"
#include "dsky_scale_cases.h"
void cm_scale_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){static const uint16_t locations[3]={02614,02621,02623};dsky_scale_prepare(s,n);memset(io,0,sizeof *io);io->superbank=0100;io->interrupt_enabled=(uint16_t)(n%2u);s->words[5]=locations[n%3u];}
