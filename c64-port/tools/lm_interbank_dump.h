/* SPDX-License-Identifier: MIT */
#include <stdio.h>
static void lm_interbank_dump(uint32_t n,uint16_t kind,uint16_t rejected,const apollo_overlay_state *s,const p66_bias_io *io){uint16_t record[2054];unsigned char bytes[4108];unsigned i;
 record[0]=(uint16_t)n;record[1]=(uint16_t)(n>>16);record[2]=kind;record[3]=rejected;for(i=0;i<2048u;++i)record[i+4u]=s->words[i];record[2052]=io->superbank;record[2053]=io->interrupt_enabled;
 for(i=0;i<2054u;++i){bytes[2u*i]=(unsigned char)record[i];bytes[2u*i+1u]=(unsigned char)(record[i]>>8);}fwrite(bytes,1,sizeof bytes,stdout);
}
