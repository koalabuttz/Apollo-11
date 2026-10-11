/* SPDX-License-Identifier: MIT; original source NUCHANG2 recheck. */
#include "cm_idle_recheck.h"
enum exec_status cm_idle_recheck(apollo_overlay_state *s,cm_native_io *io,exec_transfer *next){agc_word flag,loc,bank,priority;uint8_t interpretive;
 if(!s||!io||!next||s->words[5]!=03032u||s->words[4]!=02000u||s->words[3]>03400u||(s->words[3]&0377u)||s->words[6]!=(agc_word)(02000u|(s->words[3]>>8))||s->words[7]||(io->superbank&~0160u)||io->interrupt_enabled>1u||io->channel11>AGC_MASK||io->executive_alarm)return EXEC_OUT_OF_SCOPE;
 flag=s->words[0067];if(flag>72u||flag%12u)return EXEC_OUT_OF_SCOPE;loc=s->words[0164u+flag];bank=s->words[0165u+flag];priority=s->words[0167u+flag];interpretive=(uint8_t)((loc&AGC_SIGN)!=0);
 if(!priority||priority>=040000u||(bank&~076167u)||(!interpretive&&(loc<0100u||loc>07777u))||(interpretive&&(!flag||(agc_negate(loc)<077u||agc_negate(loc)>=07777u))))return EXEC_OUT_OF_SCOPE;
 /* Both source branches join already-certified idle dispatch tails with
  * identical canonical effects. The +0 recheck executes explicit RELINT;
  * positive exchange ends at source ENDPRCHG RELINT. No poll is invented. */
 s->words[5]=03231;io->interrupt_enabled=1;return cm_idle_dispatch(s,io,next);
}
