/* SPDX-License-Identifier: MIT */
#include "cm_nv85_cases.h"
#include "cm_clockjob_cases.h"
void cm_nv85_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){uint16_t i;cm_clockjob_input(s,io,(uint16_t)(n+16384u));s->words[01145]=01525;s->words[0100]=0;s->words[01012]=0;s->words[01021]=s->words[01022]=0;s->words[01071]=0;s->words[0167]=027000;s->words[067]=0;for(i=1;i<7u;++i)s->words[0167u+i*12u]=077777;for(i=0;i<11u;++i)s->words[01023u+i]=073777;s->words[01003]=s->words[01004]=s->words[01005]=0;s->words[01016]=11;s->words[01042]=s->words[01043]=0;}
