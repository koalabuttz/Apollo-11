/* SPDX-License-Identifier: MIT */
#include "cm_dynamic_display_cases.h"
#include "cm_dyndisp_cases.h"
void cm_dynamic_display_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){uint16_t i;cm_dyndisp_input(s,io,n);s->words[01145]=01525;s->words[0100]=0;s->words[01012]=0;s->words[01021]=s->words[01022]=0;s->words[01071]=0;s->words[0167]=(agc_word)(027000u+s->words[0120]);s->words[s->words[0120]-1u]=0;s->words[067]=0;for(i=1;i<7u;++i)s->words[0167u+i*12u]=077777;for(i=0;i<11u;++i)s->words[01023u+i]=073777;s->words[01003]=s->words[01004]=s->words[01005]=0;s->words[01016]=11;s->words[01042]=s->words[01043]=0;io->channel11=(agc_word)(((uint32_t)n*977u)&AGC_MASK);}
