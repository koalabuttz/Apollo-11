/* SPDX-License-Identifier: MIT */
#include "dsky_monitor_consumer_check.h"
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==0
#include "dsky_monitor_consumer_vectors_0.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==1
#include "dsky_monitor_consumer_vectors_1.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==2
#include "dsky_monitor_consumer_vectors_2.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==3
#include "dsky_monitor_consumer_vectors_3.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==4
#include "dsky_monitor_consumer_vectors_4.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==6
#include "dsky_monitor_consumer_vectors_6.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==7
#include "dsky_monitor_consumer_vectors_7.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==9
#include "dsky_monitor_consumer_vectors_9.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==10
#include "dsky_monitor_consumer_vectors_10.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==11
#include "dsky_monitor_consumer_vectors_11.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==12
#include "dsky_monitor_consumer_vectors_12.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==13
#include "dsky_monitor_consumer_vectors_13.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==14
#include "dsky_monitor_consumer_vectors_14.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==15
#include "dsky_monitor_consumer_vectors_15.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==16
#include "dsky_monitor_consumer_vectors_16.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==17
#include "dsky_monitor_consumer_vectors_17.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==18
#include "dsky_monitor_consumer_vectors_18.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==19
#include "dsky_monitor_consumer_vectors_19.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==20
#include "dsky_monitor_consumer_vectors_20.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==21
#include "dsky_monitor_consumer_vectors_21.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==22
#include "dsky_monitor_consumer_vectors_22.h"
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==23
#include "dsky_monitor_consumer_vectors_23.h"
#endif
uint16_t dsky_monitor_consumer_checkpoint(uint8_t stage,uint16_t n,apollo_overlay_state *s,apollo_overlay_state *before)
{const uint16_t *numbers=0,*ioff=0,*ooff=0;const uint16_t (*ip)[2]=0,(*op)[2]=0;uint16_t count=0,p,end,f,fail=0;enum dsky_nv_mission mission;enum dsky_monitor_consumer_stage next=(enum dsky_monitor_consumer_stage)99;dsky_charin_io io,old;agc_word channel;switch(stage){
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==0
case 0:count=DSKY_MONC_COUNT_0;numbers=dsky_monitor_consumer_0_numbers;ioff=dsky_monitor_consumer_0_ioff;ooff=dsky_monitor_consumer_0_ooff;ip=dsky_monitor_consumer_0_in;op=dsky_monitor_consumer_0_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==1
case 1:count=DSKY_MONC_COUNT_1;numbers=dsky_monitor_consumer_1_numbers;ioff=dsky_monitor_consumer_1_ioff;ooff=dsky_monitor_consumer_1_ooff;ip=dsky_monitor_consumer_1_in;op=dsky_monitor_consumer_1_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==2
case 2:count=DSKY_MONC_COUNT_2;numbers=dsky_monitor_consumer_2_numbers;ioff=dsky_monitor_consumer_2_ioff;ooff=dsky_monitor_consumer_2_ooff;ip=dsky_monitor_consumer_2_in;op=dsky_monitor_consumer_2_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==3
case 3:count=DSKY_MONC_COUNT_3;numbers=dsky_monitor_consumer_3_numbers;ioff=dsky_monitor_consumer_3_ioff;ooff=dsky_monitor_consumer_3_ooff;ip=dsky_monitor_consumer_3_in;op=dsky_monitor_consumer_3_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==4
case 4:count=DSKY_MONC_COUNT_4;numbers=dsky_monitor_consumer_4_numbers;ioff=dsky_monitor_consumer_4_ioff;ooff=dsky_monitor_consumer_4_ooff;ip=dsky_monitor_consumer_4_in;op=dsky_monitor_consumer_4_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==6
case 6:count=DSKY_MONC_COUNT_6;numbers=dsky_monitor_consumer_6_numbers;ioff=dsky_monitor_consumer_6_ioff;ooff=dsky_monitor_consumer_6_ooff;ip=dsky_monitor_consumer_6_in;op=dsky_monitor_consumer_6_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==7
case 7:count=DSKY_MONC_COUNT_7;numbers=dsky_monitor_consumer_7_numbers;ioff=dsky_monitor_consumer_7_ioff;ooff=dsky_monitor_consumer_7_ooff;ip=dsky_monitor_consumer_7_in;op=dsky_monitor_consumer_7_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==9
case 9:count=DSKY_MONC_COUNT_9;numbers=dsky_monitor_consumer_9_numbers;ioff=dsky_monitor_consumer_9_ioff;ooff=dsky_monitor_consumer_9_ooff;ip=dsky_monitor_consumer_9_in;op=dsky_monitor_consumer_9_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==10
case 10:count=DSKY_MONC_COUNT_10;numbers=dsky_monitor_consumer_10_numbers;ioff=dsky_monitor_consumer_10_ioff;ooff=dsky_monitor_consumer_10_ooff;ip=dsky_monitor_consumer_10_in;op=dsky_monitor_consumer_10_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==11
case 11:count=DSKY_MONC_COUNT_11;numbers=dsky_monitor_consumer_11_numbers;ioff=dsky_monitor_consumer_11_ioff;ooff=dsky_monitor_consumer_11_ooff;ip=dsky_monitor_consumer_11_in;op=dsky_monitor_consumer_11_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==12
case 12:count=DSKY_MONC_COUNT_12;numbers=dsky_monitor_consumer_12_numbers;ioff=dsky_monitor_consumer_12_ioff;ooff=dsky_monitor_consumer_12_ooff;ip=dsky_monitor_consumer_12_in;op=dsky_monitor_consumer_12_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==13
case 13:count=DSKY_MONC_COUNT_13;numbers=dsky_monitor_consumer_13_numbers;ioff=dsky_monitor_consumer_13_ioff;ooff=dsky_monitor_consumer_13_ooff;ip=dsky_monitor_consumer_13_in;op=dsky_monitor_consumer_13_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==14
case 14:count=DSKY_MONC_COUNT_14;numbers=dsky_monitor_consumer_14_numbers;ioff=dsky_monitor_consumer_14_ioff;ooff=dsky_monitor_consumer_14_ooff;ip=dsky_monitor_consumer_14_in;op=dsky_monitor_consumer_14_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==15
case 15:count=DSKY_MONC_COUNT_15;numbers=dsky_monitor_consumer_15_numbers;ioff=dsky_monitor_consumer_15_ioff;ooff=dsky_monitor_consumer_15_ooff;ip=dsky_monitor_consumer_15_in;op=dsky_monitor_consumer_15_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==16
case 16:count=DSKY_MONC_COUNT_16;numbers=dsky_monitor_consumer_16_numbers;ioff=dsky_monitor_consumer_16_ioff;ooff=dsky_monitor_consumer_16_ooff;ip=dsky_monitor_consumer_16_in;op=dsky_monitor_consumer_16_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==17
case 17:count=DSKY_MONC_COUNT_17;numbers=dsky_monitor_consumer_17_numbers;ioff=dsky_monitor_consumer_17_ioff;ooff=dsky_monitor_consumer_17_ooff;ip=dsky_monitor_consumer_17_in;op=dsky_monitor_consumer_17_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==18
case 18:count=DSKY_MONC_COUNT_18;numbers=dsky_monitor_consumer_18_numbers;ioff=dsky_monitor_consumer_18_ioff;ooff=dsky_monitor_consumer_18_ooff;ip=dsky_monitor_consumer_18_in;op=dsky_monitor_consumer_18_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==19
case 19:count=DSKY_MONC_COUNT_19;numbers=dsky_monitor_consumer_19_numbers;ioff=dsky_monitor_consumer_19_ioff;ooff=dsky_monitor_consumer_19_ooff;ip=dsky_monitor_consumer_19_in;op=dsky_monitor_consumer_19_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==20
case 20:count=DSKY_MONC_COUNT_20;numbers=dsky_monitor_consumer_20_numbers;ioff=dsky_monitor_consumer_20_ioff;ooff=dsky_monitor_consumer_20_ooff;ip=dsky_monitor_consumer_20_in;op=dsky_monitor_consumer_20_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==21
case 21:count=DSKY_MONC_COUNT_21;numbers=dsky_monitor_consumer_21_numbers;ioff=dsky_monitor_consumer_21_ioff;ooff=dsky_monitor_consumer_21_ooff;ip=dsky_monitor_consumer_21_in;op=dsky_monitor_consumer_21_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==22
case 22:count=DSKY_MONC_COUNT_22;numbers=dsky_monitor_consumer_22_numbers;ioff=dsky_monitor_consumer_22_ioff;ooff=dsky_monitor_consumer_22_ooff;ip=dsky_monitor_consumer_22_in;op=dsky_monitor_consumer_22_out;break;
#endif
#if !defined(DSKY_MONITOR_CONSUMER_ONLY_STAGE) || DSKY_MONITOR_CONSUMER_ONLY_STAGE==23
case 23:count=DSKY_MONC_COUNT_23;numbers=dsky_monitor_consumer_23_numbers;ioff=dsky_monitor_consumer_23_ioff;ooff=dsky_monitor_consumer_23_ooff;ip=dsky_monitor_consumer_23_in;op=dsky_monitor_consumer_23_out;break;
#endif
default:return 1;}if(n>=count)return 1;dsky_monitor_consumer_input(s,&io,numbers[n],&mission,&channel);p=ioff[n];end=ioff[n+1u];while(p<end){f=ip[p][0];if(f<2048u)s->words[f]=ip[p][1];else if(f==2048u)io.control.superbank=ip[p][1];else if(f==2049u)io.control.interrupt_enabled=ip[p][1];else if(f==2050u)io.channel11=ip[p][1];else io.wrote_channel11=ip[p][1];++p;}*before=*s;old=io;if(dsky_monitor_consumer(s,&io,mission,(enum dsky_monitor_consumer_stage)stage,channel,&next)!=P66_BIAS_OK)++fail;p=ooff[n];end=ooff[n+1u];for(f=0;f<2053u;++f){uint16_t want=f<2048u?before->words[f]:f==2048u?old.control.superbank:f==2049u?old.control.interrupt_enabled:f==2050u?old.channel11:f==2051u?old.wrote_channel11:99,actual=f<2048u?s->words[f]:f==2048u?io.control.superbank:f==2049u?io.control.interrupt_enabled:f==2050u?io.channel11:f==2051u?io.wrote_channel11:(uint16_t)next;if(p<end&&op[p][0]==f)want=op[p++][1];if(want!=actual)++fail;}if(p!=end)++fail;return fail;}
