/* SPDX-License-Identifier: MIT */
#include "dsky_mode_job.h"
#include "agc_multiword.h"
static void accumulate(agc_register_pair *r,uint16_t *w,uint16_t a){agc_dp d;d.high=w[a];d.low=w[a+1u];agc_double_add(r,&d);w[a]=d.high;w[a+1u]=d.low;}
static void shortmp(uint16_t *w){agc_register_pair r;agc_word minor=w[0155],major=w[0154];w[0135]=10;r=agc_multiply(10,w[0156]);w[0156]=agc_correct(r.a);w[0155]=0;r=agc_multiply(agc_extend(minor),10);accumulate(&r,w,0155);w[0154]=agc_correct(r.a);r=agc_multiply(agc_extend(major),10);accumulate(&r,w,0154);w[0]=r.a;w[1]=r.l;}
static enum p66_bias_status body(apollo_overlay_state *s,p66_bias_io *io,uint8_t cm,enum dsky_mode_job_route *route){static const uint16_t codes[10]={025,003,031,033,017,036,034,023,035,037};uint16_t *w,mode,saved,start=cm ? 03441u : 03534u;p66_bias_io arithmetic_io;
 if(!s||!io||!route)return P66_BIAS_OUT_OF_SCOPE;
 w=s->words;mode=w[01011];saved=w[0777];
 if(w[5]!=start||w[7]||w[3]>03400u||(w[3]&0377u)||w[4]!=060000u||w[6]!=(uint16_t)(060000u|(w[3]>>8))||io->superbank<0100u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u||(mode<040000u&&mode>99u))return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[0777]=025;w[0140]=saved;arithmetic_io.superbank=0100;arithmetic_io.interrupt_enabled=io->interrupt_enabled;
 if(mode==077777u){w[2]=(uint16_t)(start+8u);(void)dsky_pinball_write(s,&arithmetic_io,DSKY_2BLANK);io->interrupt_enabled=arithmetic_io.interrupt_enabled;}
 else if(mode>=040000u)w[0]=agc_extend(agc_diminished_abs(mode));
 else{w[0]=mode;w[2]=(uint16_t)(start+6u);w[5]=cm ? 03213u : 03306u;
  if(w[01035]==0){agc_register_pair r=agc_multiply(w[0],0244);w[1]=agc_extend(w[0154]);w[0154]=agc_correct(r.l);w[0155]=0;w[0115]=w[2];w[0137]=1;shortmp(w);w[0124]=codes[w[0154]==077777u ? 0u : w[0154]];w[0154]=w[0155];w[0155]=w[0156];w[0156]=0;w[0777]=024;w[0143]=1;w[0124]=(uint16_t)(w[0124]<<5);w[0114]=cm ? 03160u : 03253u;w[021]=10;w[0141]=10;w[0]=0;w[2]=cm ? 03261u : 03354u;w[5]=cm ? 05747u : 05705u;io->interrupt_enabled=0;*route=DSKY_MODE_JOB_CCSHOLE;return P66_BIAS_OK;}
  /* Original formatter math has no SUPER write; wider source caller proof
   * qualifies this frozen arithmetic primitive for each retained latch. */
  if(cm){cm_native_io p={0};p.superbank=arithmetic_io.superbank;p.interrupt_enabled=arithmetic_io.interrupt_enabled;(void)cm_two_digit(s,&p);io->interrupt_enabled=p.interrupt_enabled;}
  else{(void)dsky_two_digit(s,&arithmetic_io);io->interrupt_enabled=arithmetic_io.interrupt_enabled;}
 }
 w[0140]=agc_correct(w[0]);w[0]=agc_extend(saved);w[0777]=saved;w[2]=(uint16_t)(start+11u);w[5]=cm ? 05217u : 05155u;*route=DSKY_MODE_JOB_END;return P66_BIAS_OK;
}
enum p66_bias_status dsky_mode_job(apollo_overlay_state *s,p66_bias_io *io,enum dsky_mode_job_route *route){return body(s,io,0,route);}
enum exec_status cm_mode_job(apollo_overlay_state *s,cm_native_io *io,enum dsky_mode_job_route *route){p66_bias_io p;enum p66_bias_status status;if(!io)return EXEC_OUT_OF_SCOPE;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=body(s,&p,1,route);if(status)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
