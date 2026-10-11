/* SPDX-License-Identifier: MIT */
#include "imu_mode_transfer.h"
static unsigned ccs(agc_word value,agc_register *a){if(!value){*a=0;return 1;}if(value==AGC_MASK){*a=0;return 3;}if(value&AGC_SIGN){*a=(agc_word)(agc_negate(value)-1u);return 2;}*a=(agc_word)(value-1u);return 0;}
enum imu_transfer_status imu_mode_transfer(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_transfer_stage stage){
 static const agc_word entry[2][11]={{03133,03140,03115,03712,03714,03716,03717,03726,03731,03735,03741},{03016,03023,03000,03573,03575,03577,03600,03607,03612,03616,03622}};
 uint16_t *w;unsigned i,device,skip;agc_word mode;
 if(!m||!io||(unsigned)mission>1u||(unsigned)stage>10u)return IMU_TRANSFER_UNSUPPORTED;
 w=m->words;mode=mission==RTB_LM?01304u:01322u;
 if(w[5]!=entry[mission][stage]||w[4]!=016000u||w[3]!=01400u||w[6]!=016003u||w[7]||w[2]<02000u||w[2]>07777u||io->latch.interrupt_enabled>1u||(io->latch.superbank&~0160u)||io->channel11>AGC_MASK||io->channel12>AGC_MASK||io->channel14>AGC_MASK||(io->channel14&071000u))return IMU_TRANSFER_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return IMU_TRANSFER_UNSUPPORTED;
 if(stage==IMU_TRANSFER_SEND&&(w[050]||w[051]||w[052]))return IMU_TRANSFER_UNSUPPORTED;
 if(stage>=IMU_TRANSFER_AOT){if(w[0133]<02000u||w[0133]>07777u||(w[0134]&01777u)||w[0134]>076000u)return IMU_TRANSFER_UNSUPPORTED;}
 if(stage==IMU_TRANSFER_STALL&&w[0]>3u)return IMU_TRANSFER_UNSUPPORTED;
 if((stage==IMU_TRANSFER_FAILED||stage==IMU_TRANSFER_GOOD||stage==IMU_TRANSFER_SLEEP)&&w[071]>3u)return IMU_TRANSFER_UNSUPPORTED;
 if(stage==IMU_TRANSFER_FAILED&&w[0])return IMU_TRANSFER_UNSUPPORTED;
 if(stage==IMU_TRANSFER_GOOD&&w[0]>037777u)return IMU_TRANSFER_UNSUPPORTED;
 w[1]=agc_extend(agc_correct(w[1]));
 if(stage==IMU_TRANSFER_SEND){io->channel14|=070000u;w[0]=074;w[2]=(agc_word)(mission==RTB_LM?03040u:02723u);w[5]=(agc_word)(mission==RTB_LM?05224u:05272u);return IMU_TRANSFER_DELAY;}
 if(stage==IMU_TRANSFER_ENABLE){io->channel12|=040u;w[0]=agc_extend(io->channel12);w[2]=(agc_word)(entry[mission][stage]+4u);w[5]=(agc_word)(mission==RTB_LM?05261u:05324u);return IMU_TRANSFER_TASKOVER;}
 if(stage==IMU_TRANSFER_COARSE_ERROR){w[5]=(agc_word)(mission==RTB_LM?03637u:03522u);return IMU_TRANSFER_BAD;}
 if(stage==IMU_TRANSFER_AOT||stage==IMU_TRANSFER_RADAR||stage==IMU_TRANSFER_IMU||stage==IMU_TRANSFER_STALL){
  device=stage==IMU_TRANSFER_AOT?1u:stage==IMU_TRANSFER_RADAR?2u:stage==IMU_TRANSFER_IMU?0u:w[0];w[0]=(agc_register)device;if(stage==IMU_TRANSFER_AOT)w[2]=(agc_word)(entry[mission][stage]+2u);io->latch.interrupt_enabled=0;w[071]=(agc_word)device;
  skip=ccs(w[mode+device],&w[0]);stage=skip==0u?IMU_TRANSFER_ABORT:skip==1u?IMU_TRANSFER_SLEEP:skip==2u?IMU_TRANSFER_GOOD:IMU_TRANSFER_FAILED;
 }
 if(stage==IMU_TRANSFER_GOOD){skip=ccs(agc_correct(w[0]),&w[0]);if(skip==0u)stage=IMU_TRANSFER_ABORT;else{w[0133]=agc_correct(agc_add_register(agc_extend(w[0133]),1));stage=IMU_TRANSFER_FAILED;}}
 if(stage==IMU_TRANSFER_FAILED){w[mode+w[071]]=agc_correct(w[0]);w[5]=(agc_word)(mission==RTB_LM?02755u:02640u);return IMU_TRANSFER_EXIT;}
 if(stage==IMU_TRANSFER_SLEEP){w[0]=agc_add_register(agc_extend((agc_word)(w[0133]&01777u)),agc_extend(w[0134]));w[mode+w[071]]=agc_correct(w[0]);w[2]=3;w[5]=(agc_word)(mission==RTB_LM?05133u:05175u);return IMU_TRANSFER_JOB_SLEEP;}
 if(mission==RTB_LM){agc_register a=w[0],l=w[1];w[0]=agc_extend(w[0133]);w[1]=agc_extend(w[0134]);w[0133]=agc_correct(a);w[0134]=agc_correct(l);}
 w[2]=(agc_word)(mission==RTB_LM?03743u:03623u);w[5]=(agc_word)(mission==RTB_LM?05716u:05721u);return IMU_TRANSFER_BAILOUT;
}

uint8_t imu_transfer_literal(enum rtb_mission mission,uint16_t at,agc_word *out){
 if((unsigned)mission>1u||!out)return 0;
 if(at==(mission==RTB_LM?017747u:017626u)){*out=074;return 1;}
 if(at==(mission==RTB_LM?017743u:017623u)){*out=01210;return 1;}
 return 0;
}
