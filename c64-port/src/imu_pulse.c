/* SPDX-License-Identifier: MIT */
#include "imu_pulse.h"
enum imu_pulse_status imu_pulse_enter(apollo_overlay_state *image,imu_pulse_io *io,enum rtb_mission mission)
{
 uint16_t *w=image->words,entry,command,lg,mode,imucadr;
 if((unsigned)mission>1u)return IMU_PULSE_OUT_OF_SCOPE;
 entry=(uint16_t)(mission==RTB_LM?03323u:03206u);lg=(uint16_t)(mission==RTB_LM?01314u:01304u);mode=(uint16_t)(mission==RTB_LM?01302u:01320u);imucadr=(uint16_t)(mission==RTB_LM?01304u:01322u);command=agc_correct(w[0]);
 if(w[5]!=entry||w[4]!=016000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(w[4]|(w[3]>>8))||w[7]||w[035]||w[036]||w[047]||w[050]||w[051]||w[052]||agc_overflow(w[0])||command<0400u||command>03772u||(command&0377u)>0372u||w[lg]>=040000u||w[0133]<02000u||w[0133]>07777u||(w[0134]&~076000u)||io->latch.interrupt_enabled>1u||(io->latch.superbank&~0160u)||io->channel11>077777u||io->channel12>077777u||io->channel14>077777u)return IMU_PULSE_OUT_OF_SCOPE;
 w[0161]=command;w[1]=agc_extend(agc_correct(w[1]));
 if(w[mode]&040u){
  w[imucadr]=077777;w[0]=0177777;io->latch.interrupt_enabled=1;
  w[4]=w[0134];w[6]=(uint16_t)(w[4]|(w[3]>>8));w[0134]=016000;w[2]=0134;w[5]=w[0133];return IMU_PULSE_CAGED_RETURN;
 }
 if(w[lg]!=0){
  w[0]=(agc_word)(mission==RTB_LM?017374u:017257u);w[1]=agc_extend(w[0155]);
  w[0154]=w[0133];w[0155]=w[0134];w[2]=(uint16_t)(entry+4u);w[5]=(uint16_t)(mission==RTB_LM?05133u:05175u);return IMU_PULSE_SLEEP;
 }
 w[0156]=0;io->channel14=(agc_word)(io->channel14|040u);w[0]=4;
 w[2]=(uint16_t)(mission==RTB_LM?03335u:03220u);w[5]=(uint16_t)(mission==RTB_LM?05203u:05245u);return IMU_PULSE_WAITLIST;
}
