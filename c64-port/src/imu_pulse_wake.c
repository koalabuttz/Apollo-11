/* SPDX-License-Identifier: MIT */
#include "imu_pulse_wake.h"
#include "lm_basic_sleep.h"
#include "cm_basic_sleep.h"
#include <string.h>
enum imu_gyro_wake_status imu_gyro_wake(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission)
{
 uint16_t *w=m->words;unsigned lg;agc_word oldlo;
 if((unsigned)mission>1u)return IMU_GYRO_WAKE_UNSUPPORTED;
 lg=mission==RTB_LM?01314u:01304u;
 if(w[5]!=(mission==RTB_LM?03374u:03257u)||w[4]!=016000u||w[7]||w[3]>03400u||(w[3]&0377u)||w[6]!=(w[4]|(w[3]>>8))||w[lg]>=040000u||io->latch.interrupt_enabled>1u||(io->latch.superbank&~0160u)||w[035]||w[036]||w[047]||w[050]||w[051]||w[052])return IMU_GYRO_WAKE_UNSUPPORTED;
 w[1]=agc_extend(agc_correct(w[1]));
 if(w[lg]){w[0]=(agc_word)(mission==RTB_LM?017374u:017257u);w[5]=(agc_word)(mission==RTB_LM?05133u:05175u);return IMU_GYRO_WAKE_SLEEP;}
 oldlo=w[0134];w[0156]=0;w[0133]=w[0154];w[0134]=w[0155];w[0]=1;w[1]=agc_extend(oldlo);
 w[2]=(agc_word)(mission==RTB_LM?03335u:03220u);w[5]=(agc_word)(mission==RTB_LM?05203u:05245u);return IMU_GYRO_WAKE_WAITLIST;
}
enum imu_gyro_wake_status imu_gyro_wake_helper(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission)
{
 enum exec_status status;
 if((unsigned)mission>1u)return IMU_GYRO_WAKE_UNSUPPORTED;
 if(m->words[5]==(mission==RTB_LM?05203u:05245u))return imu_pulse_register(m,io,mission)==IMU_PULSE_WAITLIST?IMU_GYRO_WAKE_WAITLIST:IMU_GYRO_WAKE_UNSUPPORTED;
 if(m->words[5]!=(mission==RTB_LM?05133u:05175u))return IMU_GYRO_WAKE_UNSUPPORTED;
 if(mission==RTB_LM){exec_address next;status=lm_basic_sleep(m,&io->latch,&io->channel11,&next);}
 else{cm_native_io cm;exec_transfer next;memset(&cm,0,sizeof cm);cm.superbank=io->latch.superbank;cm.interrupt_enabled=io->latch.interrupt_enabled;cm.channel11=io->channel11;status=cm_basic_sleep(m,&cm,&next);io->latch.superbank=cm.superbank;io->latch.interrupt_enabled=cm.interrupt_enabled;io->channel11=cm.channel11;}
 return status==EXEC_OK?IMU_GYRO_WAKE_TRANSFER:status==EXEC_IDLE?IMU_GYRO_WAKE_IDLE:IMU_GYRO_WAKE_UNSUPPORTED;
}
