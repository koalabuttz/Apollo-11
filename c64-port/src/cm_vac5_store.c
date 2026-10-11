/* SPDX-License-Identifier: MIT */
#include "cm_vac5_store.h"
enum exec_status cm_vac5_store(apollo_overlay_state *s,cm_native_io *io){agc_word *w;unsigned i;
 if(!s||!io)return EXEC_OUT_OF_SCOPE;
 w=s->words;
 if(w[5]!=02000u||w[4]!=040000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(040000u|(w[3]>>8))||w[7]||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 for(i=0;i<7u;++i){w[0661u+3u*i]=w[0164u+12u*i];w[0662u+3u*i]=w[0165u+12u*i];w[0663u+3u*i]=w[0167u+12u*i];}
 w[3]=01400;w[6]=040003;io->interrupt_enabled=0;
 for(i=12u;i>0u;--i)w[0706u+i-1u]=w[01436u+i-1u];
 for(i=0;i<6u;++i)w[0722u+i]=w[0753u+2u*i];
 w[0730]=w[0157];w[1]=agc_extend(w[0732]);w[0731]=w[0065];w[0732]=w[0066];w[0707]=w[0067];w[0713]=w[0063];w[0]=agc_extend(w[0063]);w[061]=012;w[062]=5;w[2]=02064;w[5]=04675;return EXEC_OK;
}
