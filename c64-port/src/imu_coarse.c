/* SPDX-License-Identifier: MIT */
#include "imu_coarse.h"
#include "agc_multiword.h"
static agc_word diminished(agc_word a){agc_word magnitude=(a&040000u)?agc_negate(a):a;return magnitude?(agc_word)(magnitude-1u):0;}
static agc_word difference(agc_word a,agc_word b){agc_word d=(agc_word)((a+agc_negate(b)+1u)&AGC_MASK);if(d&040000u)--d;return d;}
static void set_bits(uint16_t *w,unsigned at,agc_word bits){w[0]=agc_add_register(agc_extend((agc_word)(agc_negate(w[at])&bits)),agc_extend(w[at]));w[at]=agc_correct(w[0]);}
enum imu_coarse_status imu_coarse(apollo_overlay_state *m,imu_pulse_io *io,enum rtb_mission mission,enum imu_coarse_stage stage)
{
 static const agc_word pc[2][4]={{03144,03010,03040,03072},{03027,02673,02723,02755}};
 uint16_t *w;unsigned i,index,mode,theta;agc_word saved;
 if(!m||!io||(unsigned)mission>1u||(unsigned)stage>3u)return IMU_COARSE_UNSUPPORTED;
 w=m->words;mode=mission==RTB_LM?01302u:01320u;theta=mission==RTB_LM?0321u:01155u;
 if(w[5]!=pc[mission][stage]||w[4]!=016000u||w[3]!=01400u||w[6]!=016003u||w[7]||w[2]<02000u||w[2]>07777u||io->latch.interrupt_enabled>1u||(io->latch.superbank&~0160u)||io->channel11>AGC_MASK||io->channel12>AGC_MASK||io->channel14>AGC_MASK||w[035]||w[036]||(w[047]&&w[047]!=077777u)||(w[050]&&w[050]!=077777u)||(w[051]&&w[051]!=077777u)||(w[052]&&w[052]!=077777u)||(w[047]==077777u&&(io->channel14&01000u))||(io->channel14&070000u))return IMU_COARSE_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return IMU_COARSE_UNSUPPORTED;
 saved=w[2];w[1]=agc_extend(agc_correct(w[1]));
 if(stage==IMU_COARSE_SET){
  if(io->channel12&010u){w[0]=7;w[5]=saved;w[2]=3;return IMU_COARSE_RETURN;}
  io->channel12=(agc_word)((io->channel12&077737u)|010u);io->channel14=(agc_word)(io->channel14&076777u);w[047]=077777;
  set_bits(w,01036,040010);set_bits(w,mode+1u,040);set_bits(w,mode,010);
  w[075]=(agc_word)(w[075]&077757u);w[076]=(agc_word)(w[076]&037777u);w[077]=(agc_word)(w[077]&067777u);w[0]=agc_extend(w[077]);w[5]=saved;w[2]=3;return IMU_COARSE_RETURN;
 }
 if(stage!=IMU_COARSE_CHECK){w[2]=(agc_word)(pc[mission][stage]+1u);if(w[mode]&040u){w[0]=037;w[5]=(agc_word)(mission==RTB_LM?03637u:03522u);return IMU_COARSE_BAD;}w[0]=0;w[2]=3;}
 if(stage==IMU_COARSE_CALCULATE){
  io->channel12=(agc_word)(io->channel12|040u);
  for(index=3u;index--!=0u;){agc_register_pair product=agc_multiply(agc_extend(difference(w[theta+index],w[032u+index])),010000);agc_register doubled=agc_add_register(product.l,product.l);int overflow=agc_overflow(doubled);w[01474]=(agc_word)index;w[061]=agc_correct(doubled);if(overflow)product.a=agc_add_register(product.a,overflow>0?1u:0177776u);w[1]=agc_extend(w[01471u+index]);w[01471u+index]=agc_correct(product.a);}
  w[0]=2;w[2]=(agc_word)(mission==RTB_LM?03040u:02723u);w[5]=(agc_word)(mission==RTB_LM?05224u:05272u);return IMU_COARSE_DELAY;
 }
 if(stage==IMU_COARSE_SEGMENT){
  w[061]=0;
  for(index=3u;index--!=0u;){agc_word command=w[01471u+index],magnitude=diminished(command);w[01474]=(agc_word)index;
   if(command==0||command==077777u){w[2]=(agc_word)(pc[mission][stage]+(command?012u:010u));w[0]=0;}
   else{agc_register residual=agc_add_register(magnitude,agc_extend(077500));w[2]=(agc_word)(pc[mission][stage]+(command&040000u?011u:7u));if((residual&0100000u)||residual==0||residual==0177777u){w[01471u+index]=0;w[0]=agc_extend(command);w[2]=(agc_word)(mission==RTB_LM?03133u:03016u);}else{w[01471u+index]=(command&040000u)?agc_negate(agc_correct(residual)):agc_correct(residual);w[0]=(command&040000u)?agc_extend(077477):0300;if(command&040000u)w[2]=(agc_word)(mission==RTB_LM?03127u:03012u);}w[061]=agc_add_word(w[061],1);}
   w[0]=agc_add_register(w[0],0177777);w[050u+index]=agc_correct(w[0]);
  }
  w[0]=diminished(w[061]);if(w[061]){w[5]=(agc_word)(mission==RTB_LM?03133u:03016u);return IMU_COARSE_SEND;}
  w[2]=(agc_word)(mission==RTB_LM?03071u:02754u);w[5]=(agc_word)(mission==RTB_LM?05221u:05267u);return IMU_COARSE_SETTLE;
 }
 for(index=3u;index--!=0u;){agc_word delta=difference(w[032u+index],w[theta+index]);agc_register a;w[061]=(agc_word)index;w[0]=diminished(delta);if(delta&&delta!=077777u){a=agc_add_register(w[0],agc_extend(077511));w[0]=a;if(!(a&0100000u)&&a!=0&&a!=0177777u){w[2]=(agc_word)(mission==RTB_LM?03114u:02777u);w[5]=(agc_word)(mission==RTB_LM?05567u:05650u);return IMU_COARSE_ALARM;}}
 }
 w[0]=0;w[5]=(agc_word)(mission==RTB_LM?03631u:03514u);return IMU_COARSE_END;
}

uint8_t imu_coarse_literal(enum rtb_mission mission,uint16_t address,agc_word *out)
{
 static const uint16_t at[2][6]={{017071,017114,017116,017207,017745,017746},{016754,016777,017001,017072,017624,017625}};
 static const agc_word values[6]={0226,0211,077511,040010,077500,077477};unsigned i;
 if((unsigned)mission>1u||!out)return 0;
 for(i=0;i<6u;++i)if(address==at[mission][i]){*out=values[i];return 1;}
 return 0;
}
