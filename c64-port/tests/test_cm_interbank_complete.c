/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>
#include "cm_interbank_complete_cases.h"
#include "cm_interbank_complete_target.h"
static int rejects(apollo_overlay_state *s,cm_native_io *io,enum cm_interbank_complete_entry kind){apollo_overlay_state before=*s;cm_native_io prior=*io;return !cm_interbank_complete(s,io,kind,0,0)&&memcmp(s,&before,sizeof before)==0&&memcmp(io,&prior,sizeof prior)==0;}
int main(void){apollo_overlay_state s;cm_native_io io;unsigned completed;uint16_t out=0xbeef;uint32_t address;
 if(cm_interbank_complete_target_check(&completed)||completed!=448u)return 1;
 cm_interbank_complete_input(&s,&io,0);s.words[7]=1;if(!rejects(&s,&io,CM_IC_BANKCALL))return 2;
 cm_interbank_complete_input(&s,&io,0);s.words[2]=07776;if(!rejects(&s,&io,CM_IC_BANKCALL))return 3;
 cm_interbank_complete_input(&s,&io,0);s.words[6]^=1;if(!rejects(&s,&io,CM_IC_BANKCALL))return 4;
 cm_interbank_complete_input(&s,&io,0);io.interrupt_enabled=2;if(!rejects(&s,&io,CM_IC_BANKCALL))return 5;
 cm_interbank_complete_input(&s,&io,0);s.words[5]=0;if(!rejects(&s,&io,CM_IC_BANKCALL))return 6;
 cm_interbank_complete_input(&s,&io,0);s.words[2]=01400;if(!rejects(&s,&io,CM_IC_BANKCALL))return 10;
 cm_interbank_complete_input(&s,&io,2u*65536u);s.words[0133]=010000;if(!rejects(&s,&io,CM_IC_SWRETURN))return 7;
 cm_interbank_complete_input(&s,&io,2u*65536u);s.words[0133]=2;if(!rejects(&s,&io,CM_IC_SWRETURN))return 11;
 cm_interbank_complete_input(&s,&io,9u*65536u);s.words[072]=3;if(!rejects(&s,&io,CM_IC_ISWRETRN))return 12;
 cm_interbank_complete_input(&s,&io,0);if(cm_interbank_complete_data_address(&s,&io,&out)||out!=0xbeef||cm_interbank_complete_data_address(&s,&io,0))return 8;
 for(address=0;address<65536u;++address){out=0xbeef;if(cm_interbank_complete_literal((uint16_t)address,&out)!=(address==04710u)||out!=(address==04710u?CM_INTERBANK_COMPLETE_PRIO12_WORD:0xbeefu)||cm_interbank_complete_literal((uint16_t)address,0))return 9;}
 puts("PASS448 full-source frozen states and unchanged-image/latch preflight rejection checks");return 0;
}
