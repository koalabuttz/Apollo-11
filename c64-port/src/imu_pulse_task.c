/* SPDX-License-Identifier: MIT */
#include "imu_pulse_task.h"
#include "agc_multiword.h"
static void load(uint16_t *w,unsigned at){w[0]=agc_extend(w[at]);w[1]=agc_extend(w[at+1u]);}
static void exchange(uint16_t *w,unsigned at){agc_register a=w[0],l=w[1];load(w,at);w[at]=agc_correct(a);w[at+1u]=agc_correct(l);}
static void mp(uint16_t *w,agc_word operand){agc_register_pair p=agc_multiply(w[0],operand);w[0]=p.a;w[1]=p.l;}
static void ads(uint16_t *w,unsigned at){w[0]=agc_add_register(w[0],agc_extend(w[at]));w[at]=agc_correct(w[0]);}
static void das(uint16_t *w,unsigned at){agc_register_pair r={w[0],w[1]};agc_dp p={w[at],w[at+1u]};agc_double_add(&r,&p);w[0]=r.a;w[1]=r.l;w[at]=p.high;w[at+1u]=p.low;}
static agc_register diminished(agc_word w){agc_word magnitude=(w&040000u)?agc_negate(w):w;return magnitude?(agc_register)(magnitude-1u):0;}
static enum imu_gyro_task_status delay(uint16_t *w,enum rtb_mission mission,uint8_t long_segment)
{
 mp(w,01000);w[0]=agc_add_register(w[0],agc_extend(long_segment?077774u:3u));w[2]=(agc_word)(mission==RTB_LM?(long_segment?03554u:03535u):(long_segment?03437u:03420u));w[5]=(agc_word)(mission==RTB_LM?05203u:05245u);return IMU_GYRO_WAITLIST;
}
enum imu_gyro_task_status imu_gyro_task(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,uint8_t augment)
{
 uint16_t *w=m->words,lg,mode,command;unsigned phase,at;agc_word major,minor;int negative;
 if((unsigned)mission>1u||augment>1u)return IMU_GYRO_UNSUPPORTED;
 lg=(uint16_t)(mission==RTB_LM?01314u:01304u);mode=(uint16_t)(mission==RTB_LM?01302u:01320u);command=(agc_word)(w[lg]&03777u);
 if(w[5]!=(mission==RTB_LM?(augment?03557u:03405u):(augment?03442u:03270u))||w[4]!=016000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[7]||w[lg]>=040000u||command<02000u||command>03772u||(command&0377u)>0366u||w[035]||w[036]||w[047]||w[050]||w[051]||w[052]||io->latch.interrupt_enabled>1u||(io->latch.superbank&~0160u)||io->channel11>077777u||io->channel12>077777u||io->channel14>077777u||(augment&&(io->channel14&01000u)))return IMU_GYRO_UNSUPPORTED;
 w[1]=agc_extend(agc_correct(w[1]));
 if(!augment){io->channel14=(agc_word)(io->channel14&076077u);w[2]=(agc_word)(mission==RTB_LM?03411u:03274u);}
 else w[2]=(agc_word)(mission==RTB_LM?03560u:03443u);
 if(w[mode]&040u){w[0]=037;w[5]=(agc_word)(mission==RTB_LM?03637u:03522u);return IMU_GYRO_BAD;}
 w[0]=0;w[2]=3;
 if(augment){
  w[0]=(agc_word)(io->channel12&010u);w[0]=diminished(agc_correct(w[0]));
  if(io->channel12&010u){w[5]=(agc_word)(mission==RTB_LM?03637u:03522u);return IMU_GYRO_BAD;}
  w[0]=agc_extend(w[lg]);w[3]=(agc_word)(w[lg]&03400u);w[6]=(agc_word)(w[4]|(w[3]>>8));w[061]=(agc_word)(w[lg]&0377u);at=command;
  major=w[at];w[0]=diminished(major);
  if(major>0&&major<040000u){w[at]=agc_correct(w[0]);w[0]=020000;ads(w,047);return delay(w,mission,1);}
  w[0]=020000;ads(w,047);return delay(w,mission,0);
 }
 for(phase=0;phase<4u;++phase){
  unsigned selected=(unsigned)(w[lg]>>12);w[0]=agc_extend(w[lg]);mp(w,010);
  if(selected==3u){w[lg]=0;w[0]=(agc_word)(mission==RTB_LM?017374u:017257u);w[2]=(agc_word)(mission==RTB_LM?03430u:03313u);w[5]=(agc_word)(mission==RTB_LM?05137u:05201u);return IMU_GYRO_JOBWAKE;}
  w[2]=(agc_word)((mission==RTB_LM?03417u:03302u)+selected*2u);
  if(selected==2u){w[0]=0177773;ads(w,lg);}
  w[064]=(agc_word)(selected==0u?0202u:selected==1u?0302u:0100u);
  w[0]=(agc_word)(010000u+(w[064]&7u));ads(w,lg);w[3]=(agc_word)(agc_correct(w[0])&03400u);w[6]=(agc_word)(w[4]|(w[3]>>8));w[061]=(agc_word)(agc_correct(w[0])&0377u);w[064]&=077770u;at=(unsigned)(w[3]+w[061]);load(w,at);exchange(w,070);
  major=w[070];minor=w[071];w[0]=diminished(major);
  negative=(major&040000u)!=0;
  if(major==0||major==077777u){w[0]=diminished(minor);negative=(minor&040000u)!=0;if(minor==0||minor==077777u)continue;w[0]=agc_add_register(w[0],agc_extend(077601));if((w[0]&0100000u)||w[0]==0)continue;}
  w[0]=negative?0177777u:0;w[1]=agc_extend(negative?077743u:034u);das(w,070);
  if(!negative){io->channel14=(agc_word)(io->channel14|w[064]);w[0]=agc_extend(w[071]);w[071]&=0177u;}
  else{io->channel14=(agc_word)(io->channel14|w[064]|0400u);w[070]=agc_negate(w[070]);w[0]=agc_extend(w[071]);w[071]=(agc_word)(077777u^(agc_negate(w[071])&0177u));w[0]^=0177777u;}
  mp(w,0200);w[062]=agc_correct(w[0]);w[0]=agc_extend(w[070]);mp(w,0400);w[070]=agc_correct(w[0]);w[0]=agc_extend(agc_correct(w[1]));mp(w,020000);ads(w,062);
  load(w,070);w[0]=agc_add_register(w[0],0177776);major=agc_correct(w[0]);w[0]=diminished(major);
  if(major>0&&major<040000u){exchange(w,at);w[0]=agc_add_register(020000,agc_extend(w[062]));w[047]=agc_correct(w[0]);return delay(w,mission,1);}
  if(major==077777u){w[0]=020000;ads(w,062);w[0]=0;}
  exchange(w,at);w[0]=agc_extend(w[062]);w[047]=agc_correct(w[0]);return delay(w,mission,0);
 }
 return IMU_GYRO_UNSUPPORTED;
}
