/* SPDX-License-Identifier: MIT */
#include "imu_coarse_cases.h"
#include "imu_pulse_cases.h"
void imu_coarse_input(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_coarse_stage stage,uint32_t n)
{
 static const agc_word pc[2][4]={{03144,03010,03040,03072},{03027,02673,02723,02755}};
 static const agc_word corner[16]={0,077777,1,077776,0277,0300,0301,077500,077477,077476,0266,0267,0270,075777,02000,037777};unsigned i,theta=mission==RTB_LM?0321u:01155u;
 imu_pulse_service_input(m,io,mission,n);m->words[0]=(agc_register)(n*977u&0177777u);m->words[2]=07770;m->words[3]=01400;m->words[4]=016000;m->words[6]=016003;m->words[5]=pc[mission][stage];io->latch.interrupt_enabled=0;io->channel14&=007777u;
 if(n&16u){m->words[047]=077777;io->channel14&=076777u;}
 for(i=0;i<3u;++i)if(n&(1u<<(i+5u)))m->words[050u+i]=077777;
 m->words[mission==RTB_LM?01302u:01320u]=(agc_word)((n>=32768u)?040u:0u);
 for(i=0;i<3u;++i){m->words[032u+i]=(agc_word)((n*(i*6u+1u)+i*1021u)&AGC_MASK);m->words[theta+i]=(agc_word)((n*(i*2u+3u)+i*1237u)&AGC_MASK);m->words[01471u+i]=n<256u?corner[(n+i*3u)%16u]:(agc_word)((n*(i*8u+1u)+i*32767u)&AGC_MASK);}
 if(stage==IMU_COARSE_CHECK&&n<256u)for(i=0;i<3u;++i){m->words[theta+i]=0;m->words[032u+i]=corner[(n+i*3u)%16u];}
}
