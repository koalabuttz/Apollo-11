/* SPDX-License-Identifier: MIT */
#include "p66_native_guidance.h"
#include "guidance_radial.h"
#include "guidance_velocity_phased.h"
#include "guidance_elapsed.h"
#include "guidance_store.h"
#include "guidance_acceleration.h"
#include "guidance_projection.h"
#include "guidance_unit_phased.h"
#include "guidance_radial_phased.h"
#include "guidance_acceleration_phased.h"
#include "guidance_acceleration_store.h"
#include "guidance_projection_phased.h"
#include "guidance_control_observer.h"
#include <stddef.h>
#ifndef P66_GUIDANCE_DISPATCH_ONLY
typedef struct {
 p66_preparation p;p66_scale_scratch s;p66_elapsed_state e;
 p66_divide_scratch d;p66_norm_scratch n;p66_pushdown push;
 p66_radial_state radial;p66_trig_scratch trig;p66_rotation_scratch rotation;
 agc_dp afc;
} frame;
typedef struct {uint16_t address,offset;} field;
#define F(a,m) {a,(uint16_t)offsetof(frame,m)}
/* This is a source-data field map, not an instruction/opcode table. Every
 * aliases' sole owner occurs once; the frame is discarded on ABI return. */
static const field fields[]={
 F(03746,p.rate.count),F(03756,p.rate.scale),F(03644,p.rate.desired.high),F(03645,p.rate.desired.low),F(0,p.rate.registers.a),F(1,p.rate.registers.l),
 F(037,p.pipa[0]),F(040,p.pipa[1]),F(041,p.pipa[2]),F(024,p.time.high),F(025,p.time.low),
 F(03763,p.old_pipa[0]),F(03764,p.old_pipa[1]),F(03765,p.old_pipa[2]),F(070,p.ruptreg[0]),F(071,p.ruptreg[1]),F(072,p.ruptreg[2]),F(03761,p.this_piptime.high),F(03762,p.this_piptime.low),
 F(01160,p.pipatmp[0]),F(01161,p.pipatmp[1]),F(01162,p.pipatmp[2]),F(01254,p.tem[0]),F(01255,p.tem[1]),F(01256,p.tem[2]),
 F(0154,p.mpac[0]),F(0155,p.mpac[1]),F(0156,p.mpac[2]),F(0157,p.mpac[3]),F(0160,p.mpac[4]),F(0161,p.mpac[5]),F(0162,p.mpac[6]),F(0163,p.mpac[7]),
 F(03766,p.delvrod[0]),F(03767,p.delvrod[1]),F(03770,p.delvrod[2]),F(03771,p.delvrod[3]),F(03772,p.delvrod[4]),F(03773,p.delvrod[5]),F(0135,s.mptemp),F(0121,s.ovfind),
 F(01234,e.piptime.high),F(01235,e.piptime.low),F(0130,e.buf),F(0131,d.buf_low),F(0136,d.dvsign),F(0137,d.dvnormct),F(0140,d.maxdvsw),
 F(021,n.sr),F(022,n.cyl),F(0116,n.addrwd),F(0132,n.buf_third),F(0141,n.tem1),
 F(0122,n.vbuf[0]),F(0123,n.vbuf[1]),F(0124,n.vbuf[2]),F(0125,n.vbuf[3]),F(0126,n.vbuf[4]),F(0127,n.vbuf[5]),
 F(03473,radial.hdotdisp.high),F(03474,radial.hdotdisp.low),F(03774,radial.hcalc1.high),F(03775,radial.hcalc1.low),
 F(0133,trig.buf2.high),F(0134,trig.buf2.low),F(0117,trig.polish),F(0142,trig.tem2),F(0143,trig.tem3),
 F(0766,trig.cduspot[0]),F(0767,trig.cduspot[1]),F(0770,trig.cduspot[2]),F(0771,trig.cduspot[3]),F(0772,trig.cduspot[4]),F(0773,trig.cduspot[5]),
 F(0736,trig.sin[0]),F(0737,trig.sin[1]),F(0740,trig.sin[2]),F(0741,trig.sin[3]),F(0742,trig.sin[4]),F(0743,trig.sin[5]),
 F(0744,trig.cos[0]),F(0745,trig.cos[1]),F(0746,trig.cos[2]),F(0747,trig.cos[3]),F(0750,trig.cos[4]),F(0751,trig.cos[5]),
 F(0144,rotation.dex2),F(0145,rotation.rtnsaver),F(03463,afc.high),F(03464,afc.low)
};
#undef F
static uint16_t *word(frame *f,uint16_t offset){return (uint16_t *)((unsigned char *)f+offset);}
static void control(apollo_overlay_state *m,uint16_t selector,uint16_t result){
 uint16_t pc=06061,loc=0,q=3,bb=064007,bank=064007;uint8_t write_q=1;
 switch(selector){
 case 103:loc=03360;bb=064002;break;
 case 113:loc=03361;break;
 case 105:loc=03366;break;
 case 115:loc=03370;break;
 case 125:loc=03373;break;
 case 206:case 186:loc=03411;break;
 case 6:loc=03374;bb=7;if(result==P66_GUIDANCE_UNIT_TERMINAL)q=02413;break;
 case 137:loc=03421;break;
 case 7:case 147:loc=03422;break;
 case 8:case 128:loc=03427;q=0140;break;
 case 9:loc=03430;break;
 case 10:loc=03433;break;
 case 106:pc=03031;loc=03374;q=0140;bb=7;break;
 case 116:pc=03043;loc=03374;bb=7;
  if(result==P66_GUIDANCE_UNIT_TERMINAL){pc=06061;q=02413;}
  else if(result==P66_GUIDANCE_ALARM_1302){pc=03402;q=03043;}
  break;
 case 126:loc=03374;bb=7;if(result==P66_GUIDANCE_UNIT_TERMINAL)q=02413;break;
 case 136:loc=03375;break;
 case 146:loc=03377;bb=7;q=0140;break;
 case 156:loc=03401;break;
 case 166:loc=03404;break;
 case 176:loc=03407;bb=064004;break;
 case 107:loc=03414;bb=5;write_q=0;break;
 case 117:loc=03416;bb=7;q=m->words[0116]==0&&m->words[0135]!=077777?02314:3;break;
 case 127:loc=03420;bb=7;write_q=0;break;
 case 108:pc=03663;loc=03605;bb=bank=046007;q=0143;break;
 case 118:pc=03656;loc=03605;bb=bank=046007;q=0146;break;
 default:break;
 }
 if(result==P66_GUIDANCE_ALARM_1302){pc=03402;bb=7;if(selector==7||selector==117){q=03210;loc=03416;}else{q=03043;loc=03374;}write_q=1;}
 m->words[3]=(uint16_t)((bb&7u)<<8);m->words[4]=(uint16_t)(bb&076000u);m->words[6]=bb;
 if(loc!=0){m->words[5]=pc;m->words[0164]=loc;m->words[0165]=bank;if(write_q)m->words[2]=q;}
}
uint16_t p66_native_guidance_entry(apollo_overlay_state *m,uint16_t selector){
 frame f;unsigned i;uint16_t result=P66_NATIVE_OK,base=m->words[0120],mode=0,pointer=0;agc_dp before;
 switch(selector){
 case 103:case 113:pointer=6;break;
 case 105:mode=077776;pointer=8;break;
 case 115:case 125:mode=077776;pointer=6;break;
 case 6:case 206:case 106:case 116:case 126:case 136:case 146:mode=077776;break;
 case 7:case 8:case 107:case 117:case 108:case 156:case 166:case 176:case 186:break;
 case 9:case 10:case 127:case 137:case 147:pointer=2;break;
 case 118:case 128:mode=077776;pointer=2;break;
 default:return P66_NATIVE_UNSUPPORTED;
 }
#ifdef P66_GUIDANCE_ONLY_GROUP
 if(selector!=P66_GUIDANCE_ONLY_GROUP)return P66_NATIVE_UNSUPPORTED;
#endif
 if(base<0401 || base>0661 || (base-0401u)%054u!=0 || m->words[base-1u]!=0 ||
    (m->words[0167]&0777u)!=base || m->words[0166]!=base+pointer)return P66_NATIVE_BAD_VAC;
 if(m->words[0163]!=mode)return P66_NATIVE_BAD_MODE;
 for(i=0;i<sizeof fields/sizeof fields[0];++i)*word(&f,fields[i].offset)=m->words[fields[i].address];
 f.push.base=base;f.push.pushloc=m->words[0166];for(i=0;i<43;++i)f.push.words[i]=m->words[base+i];
 before.high=f.p.mpac[0];before.low=f.p.mpac[1];
 switch(selector){
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==103
 case 103:(void)p66_elapsed_time(&f.p,&f.s,&f.e);f.n.addrwd=01634;break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==113
 case 113:(void)p66_store_elapsed(&f.p,&f.push);f.n.addrwd=(agc_word)(base+30u);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==105
 case 105:(void)p66_velocity_correct_gravity(&f.p,&f.s,&f.e,&f.push,&f.n.addrwd,&m->words[02630]);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==115
 case 115:(void)p66_velocity_add_current(&f.p,&f.s,&f.e,&f.push,&f.n.addrwd,&m->words[03526]);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==125
 case 125:(void)p66_velocity_store_position(&f.p,&f.s,&f.e,&f.push,&f.n.addrwd,&m->words[03520]);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==106
 case 106:(void)p66_unit_square_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==116
 case 116:{enum p66_unit_result status=p66_unit_root_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push);if(status==P66_UNIT_ALARM_1302)result=P66_GUIDANCE_ALARM_1302;if(status==P66_UNIT_OVERFLOW)result=P66_GUIDANCE_UNIT_TERMINAL;break;}
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==126
 case 126:if(p66_unit_finish_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push)==P66_UNIT_OVERFLOW)result=P66_GUIDANCE_UNIT_TERMINAL;break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==136
 case 136:(void)p66_radial_store_unit(&f.p,&f.n,&f.push);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==146
 case 146:(void)p66_radial_dot_velocity(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==156
 case 156:(void)p66_radial_store_rate(&f.p,&f.n,&f.push,&f.radial);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==166
 case 166:(void)p66_radial_predict_distance(&f.p,&f.s,&f.n,&f.push,&f.radial);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==176
 case 176:{agc_dp land={m->words[02332],m->words[02333]};(void)p66_radial_subtract_radius(&f.p,&f.s,&f.e,&f.n,&f.push,&land);break;}
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==186
 case 186:(void)p66_radial_store_altitude(&f.p,&f.n,&f.push,&f.radial);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==107
 case 107:{agc_dp tau={m->words[02540],m->words[02541]};(void)p66_acceleration_rate_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push,&tau);break;}
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==117
 case 117:if(p66_acceleration_gravity_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push,&m->words[01236])==P66_ACCELERATION_ALARM_1302)result=P66_GUIDANCE_ALARM_1302;break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==127
 case 127:(void)p66_acceleration_scale_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==137
 case 137:(void)p66_acceleration_store_phase(&f.p,&f.n,&f.push);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==147
 case 147:(void)p66_acceleration_add_phase(&f.p,&f.s,&f.e,&f.n,&f.push);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==108
 case 108:(void)p66_projection_trig_phase(&f.p,&f.s,&f.d,&f.n,&f.push,&f.trig,&m->words[032]);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==118
 case 118:(void)p66_projection_rotation_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push,&f.trig,&f.rotation);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==128
 case 128:(void)p66_projection_dot_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push,&f.trig);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==6
 case 6:{enum p66_unit_result status=p66_unit_square_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push);
  if(status==P66_UNIT_OK)status=p66_unit_root_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push);
  if(status==P66_UNIT_OK)status=p66_unit_finish_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push);
  if(status==P66_UNIT_ALARM_1302)result=P66_GUIDANCE_ALARM_1302;
  if(status==P66_UNIT_OVERFLOW)result=P66_GUIDANCE_UNIT_TERMINAL;
  break;}
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==206
 case 206:{agc_dp land={m->words[02332],m->words[02333]};
  enum p66_radial_status status=p66_radial_position(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push,&f.radial,&land);
  if(status==P66_RADIAL_ALARM_1302)result=P66_GUIDANCE_ALARM_1302;
  break;}
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==7
 case 7:{agc_dp tau={m->words[02540],m->words[02541]};
  if(p66_acceleration(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push,&tau,&m->words[01236])==P66_ACCELERATION_ALARM_1302)result=P66_GUIDANCE_ALARM_1302;
  break;}
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==8
 case 8:(void)p66_projection_dot(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push,&f.trig,&f.rotation,&m->words[032]);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==9
 case 9:(void)p66_projection_store(&f.p,&f.n,&f.push);break;
#endif
#if !defined(P66_GUIDANCE_ONLY_GROUP) || P66_GUIDANCE_ONLY_GROUP==10
 case 10:(void)p66_projection_acceleration(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push,&f.afc);break;
#endif
 default:return P66_NATIVE_UNSUPPORTED;
 }
 for(i=0;i<sizeof fields/sizeof fields[0];++i)m->words[fields[i].address]=*word(&f,fields[i].offset);
 for(i=0;i<43;++i)m->words[base+i]=f.push.words[i];
 m->words[0166]=f.push.pushloc;
 m->words[7]=0;m->words[023]=0;
 control(m,selector,result);
 if(selector==107||selector==127){agc_dp tau={m->words[02540],m->words[02541]};
  m->words[2]=p66_acceleration_return_q((uint8_t)(selector==107?0:2),m->words[2],before,f.p.rate.desired,tau,&f.n,f.s.mptemp);
 }
 return result;
}
#endif
#ifndef P66_GUIDANCE_ARITH_ONLY
#ifndef P66_GUIDANCE_DISPATCH_ONLY
static uint16_t linked_invoke(void *unused,apollo_overlay_state *m,uint16_t selector){
 uint16_t result;(void)unused;result=selector<=5 ? p66_native_entry(m,selector) : p66_native_guidance_entry(m,selector);
 return result==P66_GUIDANCE_UNIT_TERMINAL?P66_NATIVE_OK:result;
}
#endif
/* Compiled selectors parallel the first bank's explicit source mappings.
 * Every verified completed LOC is the word immediately preceding the next
 * mapped native entry. No instruction words are fetched or interpreted. */
static const uint8_t selectors[]={1,2,103,113,4,105,115,125,6,136,146,156,166,176,186,107,117,127,137,147,8,9,10};
static enum exec_action continuation(executive *s,void *user,exec_address *resume){
 p66_native_binding *b=user;uint8_t i;
 for(i=0;i<sizeof selectors;++i){
  if(p66_native_guidance_mappings[i].address.location==b->runtime->current.location){
   agc_word next=p66_native_guidance_mappings[i+1u].address.location;
   return p66_native_run_group(s,b,resume,selectors[i],(agc_word)(next-1u),next);
  }
 }
 return p66_native_run_group(s,b,resume,0,0,resume->location);
}
#define MAPS(bank) \
 {{03275,bank},EXEC_CONTEXT_BASIC,continuation},\
 {{03354,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03357,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03361,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03362,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03365,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03367,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03371,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03374,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03375,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03376,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03400,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03402,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03405,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03410,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03412,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03415,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03417,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03421,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03422,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03423,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03430,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03431,bank},EXEC_CONTEXT_INTERPRETIVE,continuation},\
 {{03434,bank},EXEC_CONTEXT_INTERPRETIVE,continuation}
const exec_context_mapping p66_native_guidance_mappings[P66_GUIDANCE_MAPPING_COUNT]={MAPS(064007),MAPS(064067)};
#undef MAPS
enum exec_status p66_native_guidance_init(exec_native *r,executive *s,p66_native_binding *b){
#ifndef P66_GUIDANCE_DISPATCH_ONLY
 if(b->invoke==0)b->invoke=linked_invoke;
#endif
 return exec_native_context_init(r,s,p66_native_guidance_mappings,P66_GUIDANCE_MAPPING_COUNT,b);
}
#endif
