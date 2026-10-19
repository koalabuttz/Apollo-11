/* SPDX-License-Identifier: MIT */
#include "dsky_monitor.h"
static unsigned test(agc_word value,agc_register *a){enum agc_ccs_class c=agc_classify(value);*a=agc_diminished_abs(value);return(unsigned)c;}
static void publish(uint16_t *w,dsky_charin_io *io){agc_register a=w[0],l=w[1];w[0]=agc_extend(w[01020]);w[1]=agc_extend(w[01021]);w[01020]=w[0154];w[01021]=w[0155];w[0154]=agc_correct(a);w[0155]=agc_correct(l);io->control.interrupt_enabled=1;w[2]=03275;w[5]=0136;}
static void kill(uint16_t *w,uint8_t cm){w[0]=0;w[01020]=w[01021]=0;w[2]=03320;w[5]=cm?05324u:05261u;}
enum p66_bias_status dsky_monitor(apollo_overlay_state *s,dsky_charin_io *io,enum dsky_nv_mission mission,enum dsky_monitor_stage stage,enum dsky_monitor_route *route)
{
 static const agc_word pc[11]={03230,03262,03263,03271,03275,03307,03313,03314,03321,03333,03351};
 uint16_t *w;unsigned i,c;uint8_t cm;enum dsky_monitor_route result;agc_register a;
 if(!s||!io||!route||(unsigned)mission>1u||(unsigned)stage>10u)return P66_BIAS_OUT_OF_SCOPE;
#if defined(DSKY_MONITOR_ONLY_STAGE)
 if(stage!=DSKY_MONITOR_ONLY_STAGE)return P66_BIAS_OUT_OF_SCOPE;
#endif
 w=s->words;cm=(uint8_t)(mission==DSKY_NV_CM);
 if(w[5]!=pc[stage]||w[4]!=062000u||w[3]>03400u||(w[3]&0377u)||w[6]!=(agc_word)(062000u|(w[3]>>8))||w[7]||io->control.superbank<0100u||io->control.superbank>0160u||(io->control.superbank&017u)||io->control.interrupt_enabled>1u||io->channel11>AGC_MASK||io->wrote_channel11>1u)return P66_BIAS_OUT_OF_SCOPE;
 /* MONDO is a job entry: Q is whatever the Executive left (16 bits), and
  * every path out of MONDO sets Q with a TC before it reads it. */
 for(i=stage==DSKY_MONITOR_DISPLAY?3u:2u;i<2048u;++i)if(w[i]>AGC_MASK)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));
 if(stage==DSKY_MONITOR_SETUP){agc_word verb=(agc_word)(w[01001]&0177u);int overflow;
  w[0155]=(agc_word)(w[01017]&017777u);a=agc_add_register(agc_extend(agc_negate(w[0136])),cm?05217u:05155u);
  if(a==0177777u){w[0155]=agc_add_word(w[0155],020000);w[01022]=0;}
  w[022]=agc_negate((agc_word)(verb<<7));w[2]=3;a=agc_add_register((agc_word)(verb<<7),agc_extend(w[01002]));w[0154]=agc_correct(a);overflow=agc_overflow(a);w[01012]=overflow>0?1u:overflow<0?077776u:0u;
  c=test(w[01042],&w[0]);if(c==0u){w[2]=03261;w[5]=03262;result=DSKY_MONITOR_CYCLE_ENTRY;}else if(c==1u){w[2]=03262;w[5]=cm?04516u:04502u;result=DSKY_MONITOR_RELEASE;}else{w[5]=c==2u?03262u:03263u;result=c==2u?DSKY_MONITOR_CYCLE_ENTRY:DSKY_MONITOR_RAW_ENTRY;}
 }else if(stage==DSKY_MONITOR_CYCLE||stage==DSKY_MONITOR_CYCLE_RAW){
  if(stage==DSKY_MONITOR_CYCLE)io->control.interrupt_enabled=0;
  c=test(w[01020],&w[0]);if(c==0u){w[2]=03271;publish(w,io);result=DSKY_MONITOR_ENTRET;}else if(c==1u||c==2u){if(c==1u)w[0]=1;w[2]=03267;w[5]=cm?05245u:05203u;result=DSKY_MONITOR_WAITLIST;}else{w[2]=03270;w[5]=03275;result=DSKY_MONITOR_REQUEST_ENTRY;}
 }else if(stage==DSKY_MONITOR_PUBLISH){publish(w,io);result=DSKY_MONITOR_ENTRET;
 }else if(stage==DSKY_MONITOR_REQUEST){w[0]=agc_extend(w[013]);w[1]=agc_extend(w[014]);w[013]=w[024];w[014]=w[025];w[2]=3;
  c=test(w[01021],&w[0]);if(c>=2u){kill(w,cm);result=DSKY_MONITOR_TASKOVER;}else{w[0]=0144;w[2]=03305;w[5]=cm?05245u:05203u;result=DSKY_MONITOR_WAITLIST;}
 }else if(stage==DSKY_MONITOR_REQUEST_REGISTERED){w[0]=030000;w[2]=03311;w[5]=cm?05134u:05072u;result=DSKY_MONITOR_NOVAC;
 }else if(stage==DSKY_MONITOR_REQUEST_ADMITTED){w[2]=03314;w[5]=cm?05324u:05261u;result=DSKY_MONITOR_TASKOVER;
 }else if(stage==DSKY_MONITOR_KILL){kill(w,cm);result=DSKY_MONITOR_TASKOVER;
 }else if(stage==DSKY_MONITOR_DISPLAY){c=test(w[01021],&w[0]);if(c>=2u){w[2]=c==2u?03325u:03326u;w[5]=cm?05217u:05155u;result=DSKY_MONITOR_END;}else{c=test(w[01012],&w[0]);if(c==0u){w[2]=03330;w[5]=03351;result=DSKY_MONITOR_BUSY_ENTRY;}else{if(c==1u)w[0]=0177;if(c!=3u)w[0]=(agc_word)(w[0]&w[01020]);w[2]=03333;w[5]=02317;result=DSKY_MONITOR_UPDATE;}}
 }else if(stage==DSKY_MONITOR_DISPLAY_AFTER){a=agc_add_register((agc_word)(w[01020]&037600u),agc_extend(075377));w[023]=(agc_word)((agc_correct(a)>>7)&0177u);w[01001]=w[023];w[023]=0;w[0136]=cm?04140u:04124u;w[0156]=(agc_word)(w[01021]&017777u);w[0]=w[0156];w[2]=03347;w[5]=02046;result=DSKY_MONITOR_TESTNN;
 }else{io->channel11=(agc_word)(io->channel11|020u);io->wrote_channel11=1;w[0]=agc_extend(io->channel11);w[2]=03353;w[5]=cm?05217u:05155u;result=DSKY_MONITOR_END;}
 *route=result;return P66_BIAS_OK;
}

uint8_t dsky_monitor_literal(enum dsky_nv_mission mission,uint16_t address,agc_word *out)
{static const uint16_t pc[10]={03237,03267,03270,03305,03306,03311,03312,03320,03347,03350};static const agc_word data[10]={060000,03275,062101,03275,062101,03321,062101,0144,075377,04124};unsigned i;if((unsigned)mission>1u||!out)return 0;for(i=0;i<10u;++i)if(address==(uint16_t)(041u*02000u+pc[i]-02000u)){*out=i==9u&&mission==DSKY_NV_CM?04140u:data[i];return 1;}return 0;}
