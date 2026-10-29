/* SPDX-License-Identifier: MIT */
#include "imu_pulse_cases.h"
void imu_pulse_input(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,uint32_t n)
{
 unsigned i,bank=(unsigned)((n>>4)&7u),context=(unsigned)(n>>15);uint16_t lg=(uint16_t)(mission==RTB_LM?01314u:01304u),mode=(uint16_t)(mission==RTB_LM?01302u:01320u);
 for(i=0;i<2048u;++i)m->words[i]=(agc_word)((i*173u+context*731u+(unsigned)mission*191u+012345u)&077777u);
 m->words[0]=(agc_word)(0400u+(n%1792u));if((m->words[0]&0377u)>0372u)m->words[0]=(agc_word)(m->words[0]-5u);
 m->words[1]=(agc_register)(n*19u&0177777u);m->words[2]=(agc_word)(mission==RTB_LM?04631u:04675u);m->words[3]=(agc_word)(bank*0400u);m->words[4]=016000;m->words[5]=(agc_word)(mission==RTB_LM?03323u:03206u);m->words[6]=(agc_word)(016000u|bank);m->words[7]=0;m->words[035]=0;m->words[036]=0;m->words[047]=0;m->words[050]=0;m->words[051]=0;m->words[052]=0;
 for(i=0;i<7u;++i)m->words[0154+i]=(agc_word)((n*(i*2u+1u)+i*513u)&077777u);
 m->words[lg]=(agc_word)((n&1u)?0u:1u+n%16383u);m->words[mode]=(agc_word)(n&077777u);
 m->words[0133]=(agc_word)(02000u+n%3072u);m->words[0134]=(agc_word)(((n>>1)&31u)*02000u);
 io->latch.interrupt_enabled=(agc_word)(n&1u);io->latch.superbank=(agc_word)(((n>>7)&7u)*020u);io->channel11=(agc_word)((n*5u)&077777u);io->channel12=(agc_word)((n*7u)&077777u);io->channel14=(agc_word)((n*11u)&077777u);
}
void imu_pulse_service_input(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,uint32_t n)
{
 unsigned i;imu_pulse_input(m,io,mission,n);
 m->words[0]=(agc_word)(02000u+n%1024u);if((m->words[0]&0377u)>0372u)m->words[0]=(agc_word)(m->words[0]-5u);
 m->words[mission==RTB_LM?01314u:01304u]=0;m->words[mission==RTB_LM?01302u:01320u]&=077737u;m->words[026]=(agc_word)(037777u-n%512u);
 for(i=0;i<8u;++i)m->words[01400u+i]=057777;
 for(i=0;i<9u;++i){m->words[01410u+2u*i]=(agc_word)(mission==RTB_LM?05240u:05306u);m->words[01411u+2u*i]=04063;}
 for(i=0;i<6u;++i)m->words[m->words[0]+i]=(agc_word)((n*(i*18u+1u)+i*32767u)&077777u);
}
