/* SPDX-License-Identifier: MIT */
#include "imu_pulse_end.h"
#include "lm_basic_wake.h"
#include "cm_basic_wake.h"
#include <string.h>
static uint8_t wake(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission)
{
 enum exec_status result;
 if(mission==RTB_LM)result=lm_basic_wake(m,&io->latch);
 else{cm_native_io cm;memset(&cm,0,sizeof cm);cm.superbank=io->latch.superbank;cm.interrupt_enabled=io->latch.interrupt_enabled;cm.channel11=io->channel11;result=cm_basic_wake(m,&cm);io->latch.superbank=cm.superbank;io->latch.interrupt_enabled=cm.interrupt_enabled;io->channel11=cm.channel11;}
 return(uint8_t)(result==EXEC_OK||result==EXEC_NOT_FOUND);
}
enum imu_gyro_end_status imu_gyro_end(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_gyro_end_entry entry)
{
 uint16_t *w=m->words,mode,imucadr;agc_word result,cadr;uint8_t bad=(uint8_t)(entry==IMU_GYRO_BAD_END);
 if((unsigned)mission>1u||(unsigned)entry>1u)return IMU_GYRO_END_UNSUPPORTED;
 mode=(uint16_t)(mission==RTB_LM?01302u:01320u);imucadr=(uint16_t)(mission==RTB_LM?01304u:01322u);
 if(w[5]!=(mission==RTB_LM?(entry==IMU_GYRO_WAKE_END?05137u:03637u):(entry==IMU_GYRO_WAKE_END?05201u:03522u))||w[4]!=016000u||w[7]||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(w[4]|(w[3]>>8))||io->latch.interrupt_enabled>1u||(io->latch.superbank&~0160u)||(w[imucadr]>=040000u&&w[imucadr]!=077777u&&w[imucadr]!=077776u))return IMU_GYRO_END_UNSUPPORTED;
 if(!bad){
  if(w[2]!=(mission==RTB_LM?03430u:03313u)||w[0]!=(mission==RTB_LM?017374u:017257u)||!wake(m,io,mission))return IMU_GYRO_END_UNSUPPORTED;
  w[2]=(agc_word)(mission==RTB_LM?03233u:03116u);
  if(w[mode]&040u)bad=1;
  else{w[2]=3;bad=(uint8_t)((io->channel11&1u)!=0);}
 }
 w[0]=0;w[1]=agc_extend(agc_correct(w[1]));w[071]=0;result=(agc_word)(bad?077777u:077776u);w[072]=result;cadr=w[imucadr];
 if(cadr==0){w[0]=agc_extend(result);w[imucadr]=result;}
 else if(mission==RTB_LM&&cadr>=040000u){w[0]=0;w[imucadr]=0;}
 else{
  unsigned at;int displacement;
  w[imucadr]=0;w[0]=agc_extend(cadr);w[2]=(agc_word)(mission==RTB_LM?03702u:03563u);w[5]=(agc_word)(mission==RTB_LM?05137u:05201u);
  if(!wake(m,io,mission))return IMU_GYRO_END_UNSUPPORTED;
  displacement=agc_to_int(w[064]);if(displacement< -1||displacement>84)return IMU_GYRO_END_UNSUPPORTED;
  at=(unsigned)(0164+displacement);w[0]=agc_add_register(agc_extend(agc_negate(w[072])),agc_extend(w[at]));w[at]=agc_correct(w[0]);
 }
 w[5]=(agc_word)(mission==RTB_LM?05261u:05324u);return IMU_GYRO_END_TASKOVER;
}
