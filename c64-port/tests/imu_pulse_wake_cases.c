/* SPDX-License-Identifier: MIT */
#include "imu_pulse_wake_cases.h"
#include "imu_pulse_cases.h"
void imu_pulse_wake_input(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,uint32_t n)
{
 unsigned slot,cores=mission==RTB_LM?8u:7u;
 imu_pulse_service_input(m,io,mission,n);m->words[5]=(agc_word)(mission==RTB_LM?03374u:03257u);m->words[mission==RTB_LM?01314u:01304u]=(agc_word)((n&1u)?1u+((n>>1)%16383u):0u);
 m->words[0154]=(agc_word)(02000u+n%3072u);m->words[0155]=(agc_word)((n%32u)*02000u);m->words[2]=(agc_word)(0100u+n%3968u);
 m->words[067]=077777;m->words[064]=0;
 for(slot=0;slot<cores;++slot){m->words[0167u+slot*12u]=slot==0?02000u:077777u;m->words[0164u+slot*12u]=07777;m->words[0165u+slot*12u]=(agc_word)(((n+slot*11u)&7u)|(((n>>3)&7u)*020u));}
 if(n&2u)m->words[0167u+12u]=06000;
}
