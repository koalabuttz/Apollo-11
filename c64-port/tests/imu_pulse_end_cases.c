/* SPDX-License-Identifier: MIT */
#include "imu_pulse_end_cases.h"
#include "imu_pulse_cases.h"
void imu_pulse_end_input(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_gyro_end_entry entry,uint32_t n)
{
 unsigned slot,cores=mission==RTB_LM?8u:7u;agc_word mode,wakeword=(agc_word)(mission==RTB_LM?017374u:017257u);
 imu_pulse_service_input(m,io,mission,n);m->words[5]=(agc_word)(mission==RTB_LM?(entry==IMU_GYRO_WAKE_END?05137u:03637u):(entry==IMU_GYRO_WAKE_END?05201u:03522u));
 m->words[0]=entry==IMU_GYRO_WAKE_END?wakeword:(agc_register)(n*29u&0177777u);m->words[2]=(agc_word)(mission==RTB_LM?03430u:03313u);
 m->words[mission==RTB_LM?01314u:01304u]=0;m->words[mission==RTB_LM?01302u:01320u]=(agc_word)((n&8u)?040u:0u);io->channel11=(agc_word)((n>>4)&077777u);
 mode=(agc_word)(((n>>8)&3u)==0?0u:((n>>8)&3u)==1?077777u:((n>>8)&3u)==2?077776u:02000u+n%3072u);m->words[mission==RTB_LM?01304u:01322u]=mode;
 m->words[067]=077777;m->words[064]=0;
 for(slot=0;slot<cores;++slot){m->words[0167u+slot*12u]=slot==0?02000u:077777u;m->words[0164u+slot*12u]=(agc_word)(02000u+(n+slot*71u)%3072u);m->words[0165u+slot*12u]=(agc_word)(((n+slot*11u)&7u)|(((n>>3)&7u)*020u));}
 if(n&0100u){m->words[0167u+12u]=agc_negate(04000);m->words[0164u+12u]=wakeword;}
 if(n&0200u){m->words[0167u+24u]=agc_negate(06000);m->words[0164u+24u]=mode;}
}
