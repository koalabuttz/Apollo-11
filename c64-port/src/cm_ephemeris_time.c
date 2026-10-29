/* SPDX-License-Identifier: MIT */
#include "cm_ephemeris_time.h"
#include "ephemeris_time_scale.h"
#include "ephemeris_add.h"
#include "interpreted_sign.h"
static void bank(apollo_overlay_state *m,agc_word eb,agc_word fb){m->words[3]=eb;m->words[4]=fb;m->words[6]=(agc_word)(fb|(eb>>8));}
bool cm_ephemeris_time(apollo_overlay_state *m,const p66_bias_io *io){
 unsigned i,vac;agc_word entry,target;
 if(!m||!io||io->interrupt_enabled>1u||(io->superbank&~0160u)||m->words[7]||m->words[067]||m->words[5]!=06037u||m->words[4]!=054000u||m->words[3]>03400u||(m->words[3]&0377u)||m->words[6]!=(054000u|(m->words[3]>>8))||m->words[0165]!=m->words[6]||m->words[0163]||m->words[023]||m->words[0115]!=040000u)return false;
 vac=m->words[0120];if(vac<0401u||vac>0661u||(vac-0401u)%054u)return false;
 entry=m->words[0164];if(entry!=02110u&&entry!=02115u&&entry!=02120u&&entry!=02123u)return false;
 for(i=2;i<2048u;++i)if(m->words[i]>077777u)return false;
 target=entry==02115u?02162u:(entry==02120u?02173u:02143u);
 if(entry==02110u)m->words[vac+047u]=054161;
 if(entry==02123u)m->words[vac+047u]=m->words[vac+052u];
 m->words[vac+046u]=(agc_word)(054000u|(target&01777u));
 m->words[0130]=m->words[0166]=(agc_word)vac;m->words[0117]=entry==02123u?054143u:054126u;
 /* Frozen scalar kernels restrict their public descriptor producers.
  * This fixed source caller supplies equivalent arithmetic descriptor
  * bits internally, then retains the actual source control word. */
 bank(m,0,0);m->words[0]=0177;m->words[1]=agc_extend(agc_correct(m->words[1]));m->words[2]=3;m->words[020]=020023;m->words[0164]=02130;m->words[0116]=020616;m->words[5]=02214;
 if(!ephemeris_time_downscale(m,APOLLO_FIXED_CM))return false;
 for(i=0;i<2u;++i){
  bank(m,i?02000u:01400u,054000);m->words[0]=i?02033u:01706u;m->words[5]=07040;m->words[020]=020001;m->words[023]=0101;m->words[0116]=i?01433u:01706u;m->words[0164]=i?02134u:02132u;
  if(!ephemeris_triple_add(m,APOLLO_FIXED_CM))return false;
  bank(m,0,0);m->words[0]=7;m->words[1]=agc_extend(agc_correct(m->words[1]));m->words[5]=07637;m->words[020]=4;m->words[023]=0;
  if(interpreted_sign(m,INTERPRETED_SIGN_CM,INTERPRETED_COMP,0)!=INTERPRETED_SIGN_OK)return false;
 }
 m->words[0]=0177;m->words[5]=02214;m->words[020]=020023;m->words[023]=046;m->words[0116]=020220;m->words[0164]=02136;
 if(!ephemeris_scalar_left(m,APOLLO_FIXED_CM))return false;
 /* SSP S1,6 and the compiled GOTO X1 source control sequence. */
 m->words[vac+050u]=6;m->words[0116]=(agc_word)(vac+050u);m->words[023]=0;m->words[020]=030005;m->words[0117]=m->words[vac+046u];
 bank(m,(agc_word)((m->words[0165]&7u)<<8),054000);m->words[0]=0140000;m->words[1]=agc_extend(agc_correct(m->words[1]));m->words[5]=06037;m->words[0164]=target;
 return true;
}
