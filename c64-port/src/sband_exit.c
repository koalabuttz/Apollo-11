/* SPDX-License-Identifier: MIT */
#include "sband_exit.h"
int sband_exit(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission){unsigned i;
 if(!s||!io||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return 0;
 if(s->words[5]!=(mission==APOLLO_FIXED_LM?06070u:06037u)||s->words[0164]!=(mission==APOLLO_FIXED_LM?03745u:03750u)||s->words[7]||s->words[023]||s->words[067]||(s->words[0163]!=0&&s->words[0163]!=077776u)||s->words[4]!=064000u||s->words[3]>03400u||(s->words[3]&0377u)||s->words[6]!=(agc_word)(064000u|(s->words[3]>>8))||s->words[0165]!=s->words[6]||!(io->superbank&0100u)||(io->superbank&~0160u)||io->interrupt_enabled>1u)return 0;
 for(i=2;i<2048u;++i)if(s->words[i]>AGC_MASK)return 0;
 /* NEWOPS reads the original antenna EXIT pair, diminishes its negative
  * value to zero, stores EDOP, and rotates the zero through CYR. */
 s->words[020]=s->words[023]=0;s->words[1]=agc_extend(agc_correct(s->words[1]));
 /* EXIT restores the compiled caller BANKSET then TC LOC+1. */
 s->words[0]=agc_extend(s->words[0165]);s->words[2]=mission==APOLLO_FIXED_LM?06747u:06716u;s->words[5]=mission==APOLLO_FIXED_LM?03746u:03751u;return 1;
}
