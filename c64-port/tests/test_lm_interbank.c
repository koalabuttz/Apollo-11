/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "lm_interbank_cases.h"
#include "lm_interbank_target.h"
static int rejects(apollo_overlay_state *s,p66_bias_io *io,enum lm_interbank_entry kind){apollo_overlay_state before=*s;p66_bias_io prior=*io;return !lm_interbank(s,io,kind,0,0)&&memcmp(s,&before,sizeof before)==0&&memcmp(io,&prior,sizeof prior)==0;}
int main(void){apollo_overlay_state s;p66_bias_io io;unsigned completed;uint16_t out=0xbeef;uint32_t address;
 if(lm_interbank_target_check(&completed)||completed!=448u)return 1;
 lm_interbank_input(&s,&io,0);s.words[7]=1;if(!rejects(&s,&io,LM_IB_BANKCALL))return 2;
 lm_interbank_input(&s,&io,0);s.words[2]=07776;if(!rejects(&s,&io,LM_IB_BANKCALL))return 3;
 lm_interbank_input(&s,&io,0);s.words[6]^=1;if(!rejects(&s,&io,LM_IB_BANKCALL))return 4;
 lm_interbank_input(&s,&io,0);io.interrupt_enabled=2;if(!rejects(&s,&io,LM_IB_BANKCALL))return 5;
 lm_interbank_input(&s,&io,0);s.words[5]=0;if(!rejects(&s,&io,LM_IB_BANKCALL))return 6;
 lm_interbank_input(&s,&io,0);s.words[2]=01400;if(!rejects(&s,&io,LM_IB_BANKCALL))return 10;
 lm_interbank_input(&s,&io,2u*65536u);s.words[0133]=010000;if(!rejects(&s,&io,LM_IB_SWRETURN))return 7;
 lm_interbank_input(&s,&io,2u*65536u);s.words[0133]=2;if(!rejects(&s,&io,LM_IB_SWRETURN))return 11;
 lm_interbank_input(&s,&io,9u*65536u);s.words[072]=3;if(!rejects(&s,&io,LM_IB_ISWRETRN))return 12;
 lm_interbank_input(&s,&io,0);if(lm_interbank_data_address(&s,&io,&out)||out!=0xbeef||lm_interbank_data_address(&s,&io,0))return 8;
 for(address=0;address<65536u;++address){out=0xbeef;if(lm_interbank_literal((uint16_t)address,&out)!=(address==04644u)||out!=(address==04644u?LM_INTERBANK_PRIO12_WORD:0xbeefu)||lm_interbank_literal((uint16_t)address,0))return 9;}
 puts("PASS448 full-source frozen states and unchanged-image/latch preflight rejection checks");return 0;
}
