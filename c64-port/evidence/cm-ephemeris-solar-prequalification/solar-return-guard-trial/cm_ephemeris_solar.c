/* SPDX-License-Identifier: MIT */
#include "cm_ephemeris_solar.h"
#include "sband_scalar.h"
#include "sband_trig.h"
#include "sband_vector_linear.h"
#include "sband_vector.h"
#include "sband_shift.h"
#include "rtb_unit.h"
static const uint8_t offsets[6]={0,1,3,4,5,6};
static void bank(apollo_overlay_state *m,agc_word eb,agc_word fb){m->words[3]=eb;m->words[4]=fb;m->words[6]=(agc_word)(fb|(eb>>8));}
static void push(apollo_overlay_state *m){unsigned i,at=m->words[0166],words=m->words[0163]==077776u?6u:2u;agc_word last=0,low=0;for(i=0;i<words;++i){last=m->words[at+i];low=last;m->words[at+i]=m->words[0154u+offsets[i]];}m->words[0166]=(agc_word)(at+words);m->words[0]=words==6u?agc_extend(m->words[at+4u]):0;m->words[1]=agc_extend(low);(void)last;}
static uint8_t unit(apollo_overlay_state *m,p66_bias_io *io,agc_word eb,agc_word loc){enum rtb_unit_status st;unsigned phase;bank(m,eb,0);m->words[0]=011;m->words[5]=03023;m->words[020]=5;m->words[023]=0;m->words[0164]=loc;if(rtb_unit_begin(m,io,RTB_CM)!=RTB_UNIT_OK)return 0;for(phase=0;phase<3u;++phase){st=rtb_unit_phase(m,io,RTB_CM,(uint8_t)phase);if(st==RTB_UNIT_ABORT_1302)return 2;if(st==RTB_UNIT_OUT_OF_SCOPE)return 0;if(st==RTB_UNIT_OVERFLOW)break;}return 1;}
static uint8_t caller(agc_word q){return q==023377u||q==022557u||q==030231u;}
static uint8_t frontier(apollo_overlay_state *m,p66_bias_io *io,unsigned stage){unsigned i,vac;static const agc_word pc[4]={07350,03023,07427,07427},loc[4]={02147,02152,02154,02155},edop[4]={052,0,0121,5},push_words[4]={4,020,020,012};agc_word eb;if(!m||!io||stage<1u||stage>4u||io->interrupt_enabled>1u||(io->superbank&~0160u))return 0;vac=m->words[0120];eb=(agc_word)((m->words[0165]&7u)<<8);if(vac<0401u||vac>0661u||(vac-0401u)%054u||m->words[5]!=pc[stage-1u]||m->words[0164]!=loc[stage-1u]||m->words[023]!=edop[stage-1u]||m->words[0166]!=vac+push_words[stage-1u]||m->words[0163]!=(stage==1u?0u:077776u)||m->words[7]||m->words[067]||m->words[0115]!=040000u||m->words[4]!=(stage==2u?0u:054000u)||m->words[3]!=(stage==1u||stage==3u?02000u:eb)||m->words[6]!=(m->words[4]|(m->words[3]>>8))||!caller(m->words[vac+052u])||(m->words[vac+047u]!=054161u&&m->words[vac+047u]!=023377u))return 0;for(i=2;i<2048u;++i)if(m->words[i]>AGC_MASK)return 0;return 1;}
uint8_t cm_ephemeris_solar_begin(apollo_overlay_state *m,p66_bias_io *io){unsigned vac;agc_word eb,entry;if(!m)return 0;entry=m->words[0164];if(entry!=02110u&&entry!=02123u)return 0;vac=m->words[0120];if(vac<0401u||vac>0661u||(vac-0401u)%054u||!caller(m->words[vac+052u]))return 0; eb=m->words[3];if(!cm_ephemeris_time(m,io))return 0;
 push(m);bank(m,02000,054000);m->words[0]=02146;m->words[5]=07541;m->words[020]=020036;m->words[023]=0;m->words[0116]=01546;m->words[0164]=02144;
 if(!sband_scalar(m,APOLLO_FIXED_CM,SBAND_DMP,0))return 0;
 push(m);bank(m,eb,0);m->words[5]=03516;m->words[020]=040001;m->words[0164]=02145;
 if(!sband_trig(m,APOLLO_FIXED_CM,1))return 0;
 bank(m,02000,054000);m->words[0]=02132;m->words[5]=07350;m->words[020]=020003;m->words[023]=052;m->words[0116]=01532;m->words[0164]=02147;
 return 1;
}
uint8_t cm_ephemeris_solar_vector(apollo_overlay_state *m,p66_bias_io *io){unsigned vac,i,at;agc_word eb,angle[2],low,old_y[2],old_z[2];if(!frontier(m,io,1))return 0;vac=m->words[0120];eb=(agc_word)((m->words[0165]&7u)<<8); if(!sband_vector_linear(m,APOLLO_FIXED_CM,SBAND_VXSC,0))return 0;
 /* Original PDDL implied pushup: retrieve the saved angle before the
  * six vector exchanges replace it. Retain the intermediate MPAC.Y/Z debris. */
 at=vac+2u;angle[0]=m->words[at];angle[1]=m->words[at+1u];old_y[0]=m->words[at+2u];old_y[1]=m->words[at+3u];old_z[0]=m->words[0161];old_z[1]=m->words[0162];low=m->words[at+5u];
 for(i=0;i<6u;++i)m->words[at+i]=m->words[0154u+offsets[i]];
 m->words[0154]=angle[0];m->words[0155]=angle[1];m->words[0156]=0;m->words[0157]=0;m->words[0160]=angle[1];m->words[0161]=old_y[0];m->words[0162]=old_y[1];m->words[0163]=0;m->words[0166]=(agc_word)(vac+010u);m->words[0116]=(agc_word)at;
 m->words[0]=1;m->words[1]=agc_extend(low);bank(m,eb,0);m->words[5]=03527;m->words[020]=1;m->words[023]=062;m->words[0164]=02150;
 if(!sband_trig(m,APOLLO_FIXED_CM,0))return 0;
 /* PDVL RESO pushes sine, then PUSH saves the unnormalized source vector. */
 at=m->words[0166];m->words[at]=m->words[0154];m->words[at+1u]=m->words[0155];m->words[0166]=(agc_word)(at+2u);
 for(i=0;i<6u;++i){m->words[0154u+offsets[i]]=m->words[02132u+i];}m->words[0163]=077776;m->words[0116]=01532;push(m);
bank(m,eb,0);m->words[0]=011;m->words[5]=03023;m->words[020]=5;m->words[023]=0;m->words[0164]=02152;(void)old_z;return 1;
}
uint8_t cm_ephemeris_solar_unit(apollo_overlay_state *m,p66_bias_io *io){agc_word eb;uint8_t status;if(!frontier(m,io,2))return 0;eb=(agc_word)((m->words[0165]&7u)<<8); status=unit(m,io,eb,02152);if(status!=1)return status;
 bank(m,02000,054000);m->words[0]=02140;m->words[5]=07427;m->words[020]=020030;m->words[023]=0121;m->words[0116]=01540;m->words[0164]=02154;
return 1;
}
uint8_t cm_ephemeris_solar_plane(apollo_overlay_state *m,p66_bias_io *io){unsigned vac;agc_word eb;uint8_t status;if(!frontier(m,io,3))return 0;vac=m->words[0120];eb=(agc_word)((m->words[0165]&7u)<<8); if(!sband_vector_cross(m,APOLLO_FIXED_CM))return 0;
 status=unit(m,io,eb,02154);if(status!=1)return status;
 m->words[0166]=(agc_word)(vac+012u);bank(m,eb,054000);m->words[0]=(agc_word)(vac+012u);m->words[5]=07427;m->words[020]=020030;m->words[023]=5;m->words[0116]=(agc_word)(vac+012u);m->words[0164]=02155;
return 1;
}
uint8_t cm_ephemeris_solar_finish(apollo_overlay_state *m,p66_bias_io *io){unsigned vac;agc_word eb,entry;if(!frontier(m,io,4))return 0;vac=m->words[0120];eb=(agc_word)((m->words[0165]&7u)<<8);entry=m->words[vac+047u]==054161u?02110u:02123u; if(!sband_vector_cross(m,APOLLO_FIXED_CM))return 0;
 bank(m,eb,0);m->words[0]=0;m->words[5]=02121;m->words[020]=020000;m->words[023]=0;
 if(!sband_shift_left(m,APOLLO_FIXED_CM,1))return 0;
 m->words[0166]=(agc_word)(vac+010u);bank(m,eb,054000);m->words[0]=(agc_word)(vac+010u);m->words[5]=07350;m->words[020]=020003;m->words[023]=0122;m->words[0116]=(agc_word)(vac+010u);m->words[0164]=02156;
 if(!sband_vector_linear(m,APOLLO_FIXED_CM,SBAND_VXSC,0))return 0;
 m->words[0166]=(agc_word)(vac+2u);m->words[0]=(agc_word)(vac+2u);m->words[5]=06720;m->words[020]=020024;m->words[023]=0;m->words[0116]=(agc_word)(vac+2u);
 if(!sband_vector_linear(m,APOLLO_FIXED_CM,SBAND_VAD,0))return 0;
 bank(m,eb,0);m->words[0]=0;m->words[5]=02121;m->words[020]=020000;m->words[023]=0127;m->words[0164]=02157;
 if(!sband_shift_left(m,APOLLO_FIXED_CM,1))return 0;
 bank(m,eb,entry==02110u?054000u:022000u);m->words[0]=entry==02110u?0140000u:0u;m->words[0115]=entry==02110u?040000u:0u;m->words[0165]=m->words[6];m->words[5]=06037;m->words[020]=030005;m->words[023]=0;m->words[0117]=entry==02110u?054161u:023377u;m->words[0164]=entry==02110u?02161u:03377u;
return 1;
}
uint8_t cm_ephemeris_solar(apollo_overlay_state *m,p66_bias_io *io){uint8_t status;if(!cm_ephemeris_solar_begin(m,io)||!cm_ephemeris_solar_vector(m,io))return 0;status=cm_ephemeris_solar_unit(m,io);if(status!=1)return status;status=cm_ephemeris_solar_plane(m,io);if(status!=1)return status;return cm_ephemeris_solar_finish(m,io);}
