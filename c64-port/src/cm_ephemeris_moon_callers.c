/* SPDX-License-Identifier: MIT */
#include "cm_ephemeris_moon_callers.h"
#include "sband_vector_linear.h"
#include "sband_shift.h"
static void bank(apollo_overlay_state *m,agc_word eb,agc_word fb){m->words[3]=eb;m->words[4]=fb;m->words[6]=(agc_word)(fb|(eb>>8));}
bool cm_ephemeris_moon_callers(apollo_overlay_state *m,const p66_bias_io *io){
 static const uint8_t offsets[6]={0,1,3,4,5,6};unsigned i,j,vac;agc_word eb,old_zlow;
 if(!m||m->words[0164]!=02115u)return false;
 vac=m->words[0120];if(vac<0401u||vac>0661u||(vac-0401u)%054u||(m->words[vac+052u]!=065673u&&m->words[vac+052u]!=062742u&&m->words[vac+052u]!=023327u))return false;
 eb=m->words[3];if(!cm_ephemeris_time(m,io))return false;
 m->words[vac+046u]=066;m->words[0130]=(agc_word)vac;
 m->words[vac]=m->words[0154];m->words[vac+1u]=m->words[0155];m->words[0166]=(agc_word)(vac+2u);
 old_zlow=m->words[0162];for(j=0;j<6u;++j)m->words[0154u+offsets[j]]=m->words[02036u+j];m->words[1]=agc_extend(old_zlow);m->words[0163]=077776;m->words[0117]=066;
 for(i=0;i<9u;++i){
  bank(m,eb,054000);m->words[0]=(agc_word)vac;m->words[5]=07350;m->words[020]=020003;m->words[023]=0124;m->words[0116]=(agc_word)vac;m->words[0164]=02166;
  if(!sband_vector_linear(m,APOLLO_FIXED_CM,SBAND_VXSC,0))return false;
  bank(m,02000,054000);m->words[0]=(agc_word)(01444u+i*6u);m->words[5]=06720;m->words[020]=060024;m->words[023]=0;m->words[0116]=(agc_word)(01444u+i*6u);m->words[0164]=02167;
  if(!sband_vector_linear(m,APOLLO_FIXED_CM,SBAND_VAD,0))return false;
  m->words[0130]=(agc_word)vac;m->words[0117]=054165;
  if(i<8u)m->words[vac+046u]=(agc_word)(066u-(i+1u)*6u);
 }
 bank(m,eb,0);m->words[0]=0;m->words[5]=02121;m->words[020]=020001;m->words[0164]=02171;
 if(!sband_shift_left(m,APOLLO_FIXED_CM,2))return false;
 bank(m,eb,(agc_word)(m->words[vac+052u]&076000u));m->words[0165]=m->words[6];m->words[0]=m->words[4]>=040000u?0140000u:0u;m->words[0115]=m->words[4]>=040000u?040000u:0u;m->words[1]=agc_extend(agc_correct(m->words[1]));m->words[5]=06037;m->words[020]=7;m->words[023]=0;m->words[0117]=m->words[vac+052u];m->words[0164]=(agc_word)(02000u+(m->words[vac+052u]&01777u));
 return true;
}
