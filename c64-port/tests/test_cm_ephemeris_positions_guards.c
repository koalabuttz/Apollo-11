/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_ephemeris_positions.h"
#include "cm_ephemeris_time_cases.h"
static apollo_overlay_state state,before;
int main(void){unsigned i,j;static const agc_word invalid_q[4]={0,023377,023557,077777};p66_bias_io io,previous;
 for(j=0;j<4u;++j){cm_ephemeris_time_input(&state,&io,j);state.words[0164]=02110;state.words[state.words[0120]+052u]=invalid_q[j];before=state;previous=io;if(cm_ephemeris_positions(&state,&io))return 1;for(i=0;i<2048u;++i)if(state.words[i]!=before.words[i])return 2;if(io.interrupt_enabled!=previous.interrupt_enabled||io.superbank!=previous.superbank)return 3;}
 if(cm_ephemeris_positions(0,&io)||cm_ephemeris_positions(&state,0))return 4;
 puts("PASS four unsupported whole LSPOS callers rejected without image or metadata mutation and two null guards");return 0;
}
