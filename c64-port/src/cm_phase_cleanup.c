/* SPDX-License-Identifier: MIT */
#include "cm_phase_cleanup.h"
enum exec_status cm_phase_cleanup(apollo_overlay_state *s,cm_native_io *io){agc_word *w,q,old_first,old_second;unsigned i,start;static const unsigned pairs[6]={0754,0760,0752,0756,0762,0764};
 if(!s||!io)return EXEC_OUT_OF_SCOPE;
 w=s->words;q=w[2];
 if(w[4]!=012000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(012000u|(w[3]>>8))||w[7]||(io->superbank&~0160u)||io->interrupt_enabled>1u||q<0100u||q>07777u)return EXEC_OUT_OF_SCOPE;
 if(w[5]==02501u){start=0;io->interrupt_enabled=0;}else if(w[5]==02505u)start=1;else if(w[5]==02513u)start=3;else return EXEC_OUT_OF_SCOPE;
 old_first=w[0764];old_second=w[0765];for(i=start;i<6u;++i){w[pairs[i]]=AGC_NEG_ZERO;w[pairs[i]+1u]=AGC_POS_ZERO;}w[0]=agc_extend(old_first);w[1]=agc_extend(old_second);w[2]=3;w[5]=q;return EXEC_OK;
}
