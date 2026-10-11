/* SPDX-License-Identifier: MIT */
#include "sband_moon.h"
#include "sband_vector_linear.h"
#include "sband_shift.h"
static void bank(apollo_overlay_state *s,agc_word eb,agc_word fb){s->words[3]=eb;s->words[4]=fb;s->words[6]=(agc_word)(fb|(eb>>8));}
bool sband_moon(apollo_overlay_state *s,p66_bias_io *io,enum apollo_fixed_mission mission){
 static const uint8_t positions[6]={0,1,3,4,5,6};
 /* Original REMDIST DP data and four following untouched source words.
  * Only the first two are used arithmetically; six-word kernel preflight
  * checks the object. Following instruction words are passive unused data,
  * never decoded or executed by this native caller. */
 static const agc_word remdist[6]={026723,000450,035021,005146,032034,004616};
 unsigned i,base;agc_word original_eb,old_zlow;uint8_t cm;
 if(!s||!io||(mission!=APOLLO_FIXED_LM&&mission!=APOLLO_FIXED_CM))return false;
 cm=(uint8_t)(mission==APOLLO_FIXED_CM);base=s->words[0120];
 if(s->words[7]||s->words[3]>03400u||(s->words[3]&0377u)||s->words[4]!=064000u||s->words[6]!=(agc_word)(064000u|(s->words[3]>>8))||s->words[0165]!=s->words[6]||s->words[0115]!=040000u||s->words[023]||s->words[067]||s->words[0163]!=077776u||s->words[5]!=(cm?06037u:06070u)||s->words[0164]!=(cm?03673u:03627u)||base<0401u||base>0661u||(base-0401u)%054u||(io->superbank&~0160u)||!(io->superbank&0100u)||io->interrupt_enabled>1u)return false;
 for(i=2;i<2048u;++i)if(s->words[i]>AGC_MASK)return false;
 original_eb=s->words[3];
 if(!cm){
  s->words[2]=3;
  old_zlow=s->words[0162];for(i=0;i<6u;++i)s->words[0154u+positions[i]]=s->words[02722u+i];
  s->words[1]=agc_extend(old_zlow);s->words[0116]=02000;s->words[0164]=03631;s->words[020]=020003;s->words[5]=07401;bank(s,original_eb,064000);
  (void)sband_vector_linear(s,mission,SBAND_VXSC,remdist);
  s->words[5]=02121;s->words[020]=020000;s->words[0164]=03632;s->words[023]=0122;bank(s,original_eb,0);
  (void)sband_shift_left(s,mission,1);
  s->words[0116]=01604;s->words[5]=06751;s->words[0164]=03633;s->words[020]=020024;s->words[023]=0;bank(s,02000,064000);
  (void)sband_vector_linear(s,mission,SBAND_VAD,0);
  s->words[0117]=065640;s->words[0116]=(agc_word)base;s->words[0166]=(agc_word)base;s->words[0164]=03641;s->words[020]=5;s->words[023]=0;s->words[0]=011;s->words[5]=03023;bank(s,original_eb,0);
 }else{
  s->words[0116]=(agc_word)(base+2u);s->words[5]=06720;s->words[0164]=03674;s->words[020]=020024;s->words[023]=0101;
  (void)sband_vector_linear(s,mission,SBAND_VAD,0);
  old_zlow=s->words[0162];for(i=0;i<6u;++i)s->words[0154u+positions[i]]=agc_negate(s->words[0154u+positions[i]]);
  s->words[1]=agc_extend(old_zlow);s->words[0117]=065701;s->words[0116]=01735;s->words[0166]=(agc_word)(base+2u);s->words[0164]=03703;s->words[020]=020013;s->words[023]=0;s->words[0]=01735;s->words[5]=07303;bank(s,01400,064000);
 }
 return true;
}
