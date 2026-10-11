/* SPDX-License-Identifier: MIT */
#include "sband_orbit.h"
static void bank(apollo_overlay_state *s,agc_word eb,agc_word fb){s->words[3]=eb;s->words[4]=fb;s->words[6]=(agc_word)(fb|(eb>>8));}
bool sband_orbit(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission)
{
 static const uint8_t positions[6]={0,1,3,4,5,6};unsigned i,base;agc_word sphere,original_eb,old_zlow;bool cm,earth;
 if(!s||!io||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return false;
 cm=mission==APOLLO_FIXED_CM;base=s->words[0120];
 if(s->words[7]||s->words[3]>03400u||(s->words[3]&0377u)||s->words[4]!=064000u||s->words[6]!=(agc_word)(064000u|(s->words[3]>>8))||s->words[0165]!=s->words[6]||s->words[0115]!=040000u||s->words[023]||s->words[067]||s->words[5]!=(cm?06037u:06070u)||s->words[0164]!=(cm?03662u:03616u)||base<0401u||base>0661u||(base-0401u)%054u||(io->superbank&~0160u)||!(io->superbank&0100u)||io->interrupt_enabled>1u)return false;
 for(i=2;i<2048u;++i){if(s->words[i]>AGC_MASK)return false;}
 sphere=s->words[base+047u];earth=sphere==0||sphere==AGC_MASK;original_eb=s->words[3];old_zlow=s->words[0162];
 /* Literal SLOAD/BHIZ then VLOAD: only these six vector slots are replaced. */
 s->words[0156]=0;
 for(i=0;i<6u;++i){s->words[0154u+positions[i]]=s->words[base+i];}
 if(earth){
  if(cm){
   for(i=0;i<6u;++i){s->words[0154u+positions[i]]=agc_negate(s->words[0154u+positions[i]]);}
   s->words[1]=agc_extend(s->words[base+5u]);s->words[0166]=(agc_word)(base+2u);s->words[0116]=01735;s->words[0117]=065677;
   s->words[0164]=03703;s->words[020]=020013;s->words[0]=01735;s->words[5]=07303;bank(s,01400,064000);
  }else{
   s->words[1]=agc_extend(old_zlow);s->words[0166]=(agc_word)base;s->words[0116]=(agc_word)base;s->words[0117]=065636;
   s->words[0164]=03641;s->words[020]=5;s->words[0]=011;s->words[5]=03023;bank(s,original_eb,0);
  }
  s->words[0163]=077776;s->words[023]=0;return true;
 }
 /* Actual source vector STORE precedes TAT DLOAD; CM's overlapping VAC
  * source/destination retains the already loaded vector in MPAC. */
 {unsigned destination=cm?base+2u:02204u;
  for(i=0;i<6u;++i){s->words[destination+i]=s->words[0154u+positions[i]];}
 }
 s->words[1]=agc_extend(s->words[0155]);s->words[0154]=s->words[base+014u];s->words[0155]=s->words[base+015u];s->words[0163]=0;
 s->words[0116]=(agc_word)(base+014u);s->words[0117]=cm?054115u:033664u;s->words[020]=050006;s->words[023]=0;s->words[2]=3;
 s->words[base+052u]=cm?065673u:065627u;
 bank(s,original_eb,cm?054000u:032000u);s->words[0165]=s->words[6];s->words[0115]=cm?040000u:0;s->words[0]=agc_extend(s->words[0115]);s->words[0164]=cm?02115u:03664u;s->words[5]=cm?06037u:06070u;
 return true;
}
