/* SPDX-License-Identifier: MIT */
#include "imu_standby.h"
#include "agc_multiword.h"
static void swap_pair(uint16_t *w,unsigned at){agc_register a=w[0],l=w[1];w[0]=agc_extend(w[at]);w[1]=agc_extend(w[at+1u]);w[at]=agc_correct(a);w[at+1u]=agc_correct(l);}
static void load_pair(uint16_t *w,unsigned at,uint8_t negate){w[0]=agc_extend(negate?agc_negate(w[at]):w[at]);w[1]=agc_extend(negate?agc_negate(w[at+1u]):w[at+1u]);}
static void add_pair(uint16_t *w,unsigned at){agc_register_pair r={w[0],w[1]};agc_dp d={w[at],w[at+1u]};agc_double_add(&r,&d);w[at]=d.high;w[at+1u]=d.low;w[0]=r.a;w[1]=r.l;}
static void postcom(uint16_t *w,enum rtb_mission mission,unsigned saved){load_pair(w,saved,0);add_pair(w,0154);w[2]=(agc_word)(mission==RTB_LM?03767u:03772u);w[5]=(agc_word)(mission==RTB_LM?07257u:07226u);}
enum imu_standby_status imu_standby(apollo_overlay_state *m,imu_standby_io *io,enum rtb_mission mission,enum imu_standby_stage stage){
 static const agc_word entry[2][21]={{03655,03657,03664,03665,03672,03674,03676,03706,03711,03712,03713,03714,03717,03734,03744,03745,03752,03755,03763,03767,03773},{03660,03662,03667,03670,03675,03677,03701,03711,03714,03715,03716,03717,03722,03737,03747,03750,03755,03760,03766,03772,03776}};
 uint16_t *w;unsigned i,saved,scals;agc_word fb,pre_retry,post_retry,agree_q,low;uint8_t special_q;
 if(!m||!io||(unsigned)mission>1u||(unsigned)stage>20u)return IMU_STANDBY_UNSUPPORTED;
 w=m->words;fb=mission==RTB_LM?076000u:054000u;saved=mission==RTB_LM?0314u:01150u;scals=saved+2u;pre_retry=entry[mission][IMU_STANDBY_PRE_RETRY];post_retry=entry[mission][IMU_STANDBY_POST_RETRY];agree_q=mission==RTB_LM?07304u:07273u;
 special_q=(uint8_t)(stage==IMU_STANDBY_SCAL_FINISH||stage==IMU_STANDBY_PRE_RETRY||stage==IMU_STANDBY_PRE_READY||stage==IMU_STANDBY_POST_RETRY||stage==IMU_STANDBY_POST_READY||stage==IMU_STANDBY_POST_SHORT_RETURN);
 if(w[5]!=entry[mission][stage]||w[4]!=fb||w[3]>03400u||(w[3]&0377u)||w[6]!=(fb|(w[3]>>8))||w[7]||(!special_q&&(w[2]<02000u||w[2]>07777u))||io->pulse.latch.interrupt_enabled>1u||(io->pulse.latch.superbank&~0160u)||(mission==RTB_LM&&(io->pulse.latch.superbank&0100u))||io->pulse.channel11>AGC_MASK||io->pulse.channel12>AGC_MASK||io->pulse.channel14>AGC_MASK||io->channel13>AGC_MASK||(io->channel13&034000u)||(io->pulse.channel14&071000u))return IMU_STANDBY_UNSUPPORTED;
 for(i=2;i<2048u;++i)if(w[i]>AGC_MASK)return IMU_STANDBY_UNSUPPORTED;
 if(stage==IMU_STANDBY_SCAL_BEGIN&&(io->pulse.latch.interrupt_enabled||(w[2]!=pre_retry&&w[2]!=post_retry)))return IMU_STANDBY_UNSUPPORTED;
 if(stage==IMU_STANDBY_SCAL_FINISH&&(w[2]!=3u||io->pulse.latch.interrupt_enabled||w[0]>037777u||w[1]>037776u||(w[0156]!=pre_retry&&w[0156]!=post_retry)))return IMU_STANDBY_UNSUPPORTED;
 if(stage==IMU_STANDBY_PRE_RETRY||stage==IMU_STANDBY_PRE_READY||stage==IMU_STANDBY_POST_RETRY||stage==IMU_STANDBY_POST_READY){if(w[2]!=0157u||w[1]||w[0]>036u||w[0156]!=entry[mission][stage])return IMU_STANDBY_UNSUPPORTED;if((stage==IMU_STANDBY_PRE_RETRY||stage==IMU_STANDBY_POST_RETRY)&&w[0])return IMU_STANDBY_UNSUPPORTED;}
 if(stage==IMU_STANDBY_POST_SHORT_RETURN&&(w[2]!=3u||w[1]))return IMU_STANDBY_UNSUPPORTED;
 if((stage==IMU_STANDBY_POST_AGREE_RETURN||stage==IMU_STANDBY_POST_TIME_RETURN)&&(w[2]!=agree_q||w[1]!=entry[mission][stage]||(w[0]!=0u&&w[0]!=1u&&w[0]!=0177776u)))return IMU_STANDBY_UNSUPPORTED;
 w[1]=agc_extend(agc_correct(w[1]));
 if(stage==IMU_STANDBY_SET_FLAG){w[2]=(agc_word)(entry[mission][stage]+1u);w[5]=(agc_word)(mission==RTB_LM?05504u:05546u);return IMU_STANDBY_UPFLAG;}
 if(stage==IMU_STANDBY_PRE_BEGIN){io->pulse.latch.interrupt_enabled=0;load_pair(w,024,0);swap_pair(w,saved);w[2]=pre_retry;w[5]=entry[mission][IMU_STANDBY_SCAL_BEGIN];return IMU_STANDBY_SCALPREP;}
 if(stage==IMU_STANDBY_PRE_RETRY){w[2]=(agc_word)(entry[mission][stage]+1u);w[5]=entry[mission][IMU_STANDBY_PRE_BEGIN];return IMU_STANDBY_PRESTAND;}
 if(stage==IMU_STANDBY_PRE_READY){swap_pair(w,0154);swap_pair(w,scals);io->pulse.latch.interrupt_enabled=0;w[2]=(agc_word)(mission==RTB_LM?03671u:03674u);w[5]=(agc_word)(mission==RTB_LM?04616u:04662u);return IMU_STANDBY_BANKCALL;}
 if(stage==IMU_STANDBY_PRE_REF_RETURN||stage==IMU_STANDBY_PRE_USE_RETURN||stage==IMU_STANDBY_POST_TIME_RETURN){if(stage==IMU_STANDBY_POST_TIME_RETURN){swap_pair(w,0154);add_pair(w,024);}w[2]=stage==IMU_STANDBY_POST_TIME_RETURN?(agc_word)(mission==RTB_LM?03772u:03775u):(agc_word)(entry[mission][stage]+1u);w[5]=(agc_word)(mission==RTB_LM?05516u:05560u);return IMU_STANDBY_DOWNFLAG;}
 if(stage==IMU_STANDBY_PRE_RENDEZ_RETURN){io->channel13|=02000u;w[0]=agc_extend(io->channel13);w[2]=(agc_word)(mission==RTB_LM?03702u:03705u);w[5]=(agc_word)(mission==RTB_LM?05353u:05412u);return IMU_STANDBY_PHASE;}
 if(stage>=IMU_STANDBY_PRE_PHASE_RETURN&&stage<=IMU_STANDBY_PRE_RECYCLE){if(stage==IMU_STANDBY_PRE_PHASE_RETURN){w[0]=062;w[2]=(agc_word)(mission==RTB_LM?03710u:03713u);w[5]=(agc_word)(mission==RTB_LM?04616u:04662u);return IMU_STANDBY_BANKCALL;}w[5]=entry[mission][IMU_STANDBY_PRE_PHASE_RETURN];return IMU_STANDBY_PROMPT;}
 if(stage==IMU_STANDBY_SCAL_BEGIN){agc_word q=w[2];w[2]=w[0156];w[0156]=q;w[2]=(agc_word)(mission==RTB_LM?03717u:03722u);w[5]=(agc_word)(mission==RTB_LM?04103u:04554u);return IMU_STANDBY_FINETIME;}
 if(stage==IMU_STANDBY_SCAL_FINISH){io->pulse.latch.interrupt_enabled=1;swap_pair(w,0154);w[0]=0;w[1]=020;add_pair(w,0154);low=w[0155];w[0155]&=077740u;w[0]=(agc_word)(low&037u);if(w[0]){w[0]=(agc_register)(w[0]-1u);w[0156]=(agc_word)(w[0156]+1u);w[5]=w[0156];w[2]=0157;return IMU_STANDBY_READY;}w[5]=w[0156];w[2]=0157;return IMU_STANDBY_RETRY;}
 if(stage==IMU_STANDBY_POST_BEGIN||stage==IMU_STANDBY_POST_RETRY){if(stage==IMU_STANDBY_POST_BEGIN)io->channel13&=075777u;io->pulse.latch.interrupt_enabled=0;w[0]=w[1]=0;swap_pair(w,024);w[2]=post_retry;w[5]=entry[mission][IMU_STANDBY_SCAL_BEGIN];return IMU_STANDBY_SCALPREP;}
 if(stage==IMU_STANDBY_POST_READY){load_pair(w,scals,1);add_pair(w,0154);w[0]=01000;w[2]=(agc_word)(mission==RTB_LM?03752u:03755u);w[5]=(agc_word)(mission==RTB_LM?07307u:07256u);return IMU_STANDBY_SHORTMP;}
 if(stage==IMU_STANDBY_POST_SHORT_RETURN){w[0]=0;w[0156]=0;w[2]=(agc_word)(mission==RTB_LM?03755u:03760u);w[5]=(agc_word)(mission==RTB_LM?07257u:07226u);return IMU_STANDBY_TPAGREE;}
 if(stage==IMU_STANDBY_POST_AGREE_RETURN||stage==IMU_STANDBY_POSTCOM){if(stage==IMU_STANDBY_POST_AGREE_RETURN&&(w[0154]&AGC_SIGN))w[0154]=agc_correct(agc_add_register(agc_extend(w[0154]),01000));postcom(w,mission,saved);return IMU_STANDBY_TPAGREE;}
 w[2]=(agc_word)(entry[mission][stage]+1u);w[5]=(agc_word)(mission==RTB_LM?06001u:04106u);return IMU_STANDBY_GOTOPOOH;
}

#include "imu_standby_literals.h"
uint8_t imu_standby_literal(enum rtb_mission mission,uint16_t at,agc_word *out){unsigned i;if((unsigned)mission>1u||!out)return 0;for(i=0;i<10u;++i)if(standby_literals[mission][i].address==at){*out=standby_literals[mission][i].value;return 1;}return 0;}
