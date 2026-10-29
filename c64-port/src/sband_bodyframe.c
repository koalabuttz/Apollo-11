/* SPDX-License-Identifier: MIT */
#include "sband_bodyframe.h"
#include "lm_powered_interface.h"
#include "lm_powered_axis.h"
#include "cm_powered_interface.h"
#include "cm_powered_axis.h"
static void bank(apollo_overlay_state *s,agc_word eb,agc_word fb){s->words[3]=eb;s->words[4]=fb;s->words[6]=(agc_word)(fb|(eb>>8));}
bool sband_bodyframe(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission){static const uint8_t pos[6]={0,1,3,4,5,6};unsigned i,base,stack;agc_word low,eb;uint8_t cm;
 if(!s||!io||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return false;
 cm=(uint8_t)(mission==APOLLO_FIXED_CM);base=s->words[0120];eb=(agc_word)((s->words[0165]&7u)<<8);stack=base+(cm?2u:0u);
 if(s->words[5]!=(cm?06030u:06061u)||s->words[0164]!=(cm?03704u:03645u)||s->words[023]!=(cm?052u:0)||s->words[0163]!=077776u||s->words[4]||s->words[3]!=eb||s->words[6]!=(eb>>8)||(s->words[0165]&077770u)!=064000u||s->words[0115]!=040000u||s->words[067]||s->words[7]||base<0401u||base>0661u||(base-0401u)%054u||s->words[0166]!=stack||!(io->superbank&0100u)||(io->superbank&~0160u)||io->interrupt_enabled>1u)return false;
 for(i=2;i<2048u;++i)if(s->words[i]>AGC_MASK)return false;
 /* The compiled PUSH/PDDL and following zero-angle STORE/STOVL tuples.
  * PDDL moves the old stack Y pair into MPAC.Z before reloading RCM;
  * its low debris is therefore stack+3, rather than the input MPAC.Z low. */
 low=cm?s->words[stack+3u]:s->words[0162];
 for(i=0;i<6u;++i)s->words[stack+i]=s->words[0154u+pos[i]];
 if(cm){s->words[base+024u]=0;s->words[base+025u]=0;s->words[0166]=(agc_word)(base+010u);}
 else{for(i=0;i<4u;++i)s->words[02200u+i]=0;s->words[0166]=(agc_word)base;}
 s->words[0156]=0;s->words[1]=agc_extend(low);s->words[2]=3;s->words[0116]=(agc_word)stack;
 s->words[base+052u]=cm?065712u:065654u;s->words[0117]=cm?047644u:047671u;s->words[0164]=cm?03644u:03671u;s->words[020]=050006;s->words[023]=0;
 bank(s,eb,046000);s->words[0165]=s->words[6];s->words[0]=agc_extend(040000);s->words[5]=cm?06037u:06070u;
 /* Permanent source *SMNB* wrapper, actual AX return, TC INTPRET reentry,
  * source VLOAD and true fixed RVQ. Every callee contract is now fixed. */
 if(cm){(void)cm_powered_interface_start(s,6);(void)cm_powered_axis(s);(void)cm_powered_axis_return(s);(void)cm_powered_interface_reentry(s,&io->interrupt_enabled);(void)cm_powered_interface_load(s);(void)cm_powered_interface_return(s);}
 else{(void)lm_powered_interface_start(s,6);(void)lm_powered_axis(s);(void)lm_powered_axis_return(s);(void)lm_powered_interface_reentry(s,&io->interrupt_enabled);(void)lm_powered_interface_load(s);(void)lm_powered_interface_return(s);}
 return true;
}
