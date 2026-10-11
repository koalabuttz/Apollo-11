/* SPDX-License-Identifier: MIT */
#include "p66_native_descent.h"
#include "guidance_horizontal.h"
#include "guidance_feedback_pairs.h"
#include "guidance_unit_phased.h"
#include "guidance_length_phased.h"
#include "guidance_horizontal_phased.h"
#include <stddef.h>
#include "p66_descent_view.h"
#if !defined(P66_DESCENT_NUMERIC_ONLY) && (!defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==221)
/* Observe source calls to OVERFLOW without changing the owned vector. If
 * either doubling overflows, Q is3; otherwise the exact doubled component
 * suffices to test the second shift. No numerical context is retained. */
static uint8_t surface_shift_overflow(const p66_preparation *s){
 static const uint8_t at[3]={0,3,5};uint8_t j,i;
 for(j=0;j<3u;++j){agc_dp x={s->mpac[at[j]],s->mpac[at[j]+1u]};
  for(i=0;i<2u;++i){agc_register_pair r={agc_extend(x.high),agc_extend(x.low)};
   agc_double_add(&r,&x);if(r.a)return 1;
  }
 }
 return 0;
}
#endif
uint16_t p66_native_descent_entry(apollo_overlay_state *image,uint16_t selector){
 descent_frame f;unsigned i;uint16_t base=image->words[0120],mode=077776,pointer=0;
 enum p66_horizontal_status status=P66_HORIZONTAL_OK;
 uint16_t result=P66_NATIVE_OK;
 static const uint8_t offsets[20]={0,0,2,4,2,0,2,2,4,2,2,4,6,8,6,6,6,4,0,2};
 switch(selector){
 case 220:case 226:case 227:mode=0;break;
 case 221:case 222:break;
 case 224:pointer=6;break;
 case 210:case 211:case 234:case 235:break;
 case 212:case 213:case 236:mode=0;break;
 case 230:case 231:case 232:case 233:pointer=6;break;
 default:if(selector<190u||selector>209u)return P66_NATIVE_UNSUPPORTED;
  pointer=offsets[selector-190u];mode=selector<192u?077776u:0u;
  if(selector==208u)pointer=(uint16_t)(image->words[0166]-base);
  break;
 }
#ifdef P66_DESCENT_ONLY_GROUP
 if(selector!=P66_DESCENT_ONLY_GROUP)return P66_NATIVE_UNSUPPORTED;
#endif
 if(base<0401||base>0661||(base-0401u)%054u!=0||image->words[base-1u]!=0||
    (image->words[0167]&0777u)!=base||image->words[0166]!=base+pointer)return P66_NATIVE_BAD_VAC;
 if(image->words[0163]!=mode)return P66_NATIVE_BAD_MODE;
 for(i=0;i<sizeof fields/sizeof fields[0];++i)*word(&f,fields[i].offset)=image->words[fields[i].address];
 f.push.base=base;f.push.pushloc=image->words[0166];for(i=0;i<43u;++i)f.push.words[i]=image->words[base+i];
 f.control.interrupt_enabled=0; /* retained external metadata; never erasable */
 if(selector>=190u&&selector<=209u){
#if !defined(P66_DESCENT_ONLY_GROUP)||(P66_DESCENT_ONLY_GROUP>=190&&P66_DESCENT_ONLY_GROUP<=209)
  static const agc_word previous[20]={03433,03436,03440,03443,03444,03446,03451,03454,03457,03461,03464,03467,03472,03474,03476,03477,03502,03503,03504,03506};
  enum p66_feedback_status feedback_status;
  f.control.next_pair=(uint8_t)(selector-190u);
  f.control.loc=previous[selector-190u];
  feedback_status=p66_feedback_pair(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push,
   &f.feedback,&f.inputs,f.bias,&f.control,(uint8_t)(selector-190u));
  if(feedback_status==P66_FEEDBACK_OUT_OF_SCOPE)return P66_NATIVE_BAD_MODE;
  if(feedback_status==P66_FEEDBACK_ALARM_1302)result=01302u;
#else
  return P66_NATIVE_UNSUPPORTED;
#endif
 }else{
 switch(selector){
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==210||P66_DESCENT_ONLY_GROUP==234
 case 210:case 234:
  if(p66_length_square_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push)!=P66_NORM_OK)return P66_NATIVE_BAD_MODE;
  f.control.pc=03202;f.control.q=0140;break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==211||P66_DESCENT_ONLY_GROUP==235
 case 211:case 235:
  if(p66_length_root_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push)==P66_NORM_ALARM_1302){result=01302;f.control.pc=03402;f.control.q=03210;}
  else {f.control.pc=03210;f.control.q=3;}
  break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==212||P66_DESCENT_ONLY_GROUP==236
 case 212:case 236:
  if(p66_length_finish_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push)!=P66_NORM_OK)return P66_NATIVE_BAD_MODE;
  f.control.pc=06061;f.control.q=3;break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==213
 case 213:
  if(p66_feedback_timestamp_after_length(&f.p,&f.n,&f.push,&f.control)!=P66_FEEDBACK_NORMAL)return P66_NATIVE_BAD_MODE;
  break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==230
 case 230:
  if(p66_unit_square_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push)!=P66_UNIT_OK)return P66_NATIVE_BAD_MODE;
  f.control.pc=03031;f.control.q=0140;break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==231
 case 231:{enum p66_unit_result r=p66_unit_root_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push);
  f.control.pc=r==P66_UNIT_OK?03043:r==P66_UNIT_ALARM_1302?03402:06061;
  f.control.q=r==P66_UNIT_ALARM_1302?03043:r==P66_UNIT_OVERFLOW?02413:3;
  if(r==P66_UNIT_ALARM_1302)result=01302;
  if(r==P66_UNIT_OVERFLOW)result=0x8003;
  break;}
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==232
 case 232:{enum p66_unit_result r=p66_unit_finish_phase(&f.p,&f.s,&f.e,&f.d,&f.n,&f.push);
  f.control.pc=06061;f.control.q=r==P66_UNIT_OVERFLOW?02413:3;
  if(r==P66_UNIT_OVERFLOW)result=0x8003;
  break;}
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==233
 case 233:status=p66_horizontal_radial_product(&f.p,&f.s,&f.n,&f.push,&f.hdot);break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==220
 case 220:status=p66_horizontal_load(&f.p,&f.n,&f.push);break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==221
 case 221:
#ifdef P66_DESCENT_NUMERIC_ONLY
  status=p66_horizontal_surface(&f.p,&f.s,&f.e,&f.n,&f.push,f.delvs);
#else
  {uint8_t overflow=surface_shift_overflow(&f.p);uint16_t mask;
   (void)p66_vector_shift_left(&f.p,&f.s,&f.e,2,1);
   mask=p66_vector_add(&f.p,&f.s,&f.e,f.delvs);f.n.addrwd=031733;
   if(overflow||(mask&0111u))f.control.q=3;
  }
#endif
  break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==222
 case 222:status=p66_horizontal_position(&f.p,&f.s,&f.e,&f.n,&f.push,f.position);break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==224
 case 224:status=p66_horizontal_tangential(&f.p,&f.s,&f.e,&f.n,&f.push);break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==226
 case 226:status=p66_horizontal_store(&f.p,&f.n,&f.push,&f.vhoriz);break;
#endif
#if !defined(P66_DESCENT_ONLY_GROUP)||P66_DESCENT_ONLY_GROUP==227
 case 227:status=p66_horizontal_exit(&f.p,&f.push);break;
#endif
 default:return P66_NATIVE_UNSUPPORTED;
 }
 if((selector>=210u&&selector<=212u)||(selector>=234u&&selector<=236u)){
  f.control.fb=0;f.control.eb=03400;f.control.bb=7;
  f.control.loc=selector<=212u?03437:03526;
  f.control.cyr=040005;f.control.edop=selector<=212u?0052:0;
 }
 if(selector>=230u&&selector<=232u){
  f.control.fb=0;f.control.eb=03400;f.control.bb=7;
  f.control.loc=03523;f.control.cyr=5;f.control.edop=0016;
 }
#ifndef P66_DESCENT_NUMERIC_ONLY
 if(selector==220u||selector==221u||selector==222u||selector==224u||selector==226u||selector==227u||selector==233u){
  f.control.pc=06061;f.control.fb=064000;f.control.eb=03400;
  f.control.bb=064007;f.control.edop=0;
  switch(selector){
  case 220:f.control.loc=03516;f.control.cyr=020000;break;
  case 221:f.control.loc=03520;f.control.cyr=020024;break;
  case 222:f.control.loc=03522;f.control.cyr=020014;f.control.q=3;break;
  case 233:f.control.loc=03524;f.control.cyr=020003;f.control.q=3;break;
  case 224:f.control.loc=03525;f.control.cyr=020026;f.control.q=3;break;
  case 226:f.control.loc=03527;f.control.cyr=040005;f.control.q=3;f.control.eb=02000;f.control.bb=064004;break;
  case 227:f.control.loc=03530;f.control.cyr=0;f.control.q=06747;f.control.pc=03531;break;
  default:break;
  }
 }
#endif
 }
 if(status==P66_HORIZONTAL_OUT_OF_SCOPE)return P66_NATIVE_BAD_MODE;
 for(i=0;i<sizeof fields/sizeof fields[0];++i)image->words[fields[i].address]=*word(&f,fields[i].offset);
 image->words[0166]=f.push.pushloc;for(i=0;i<43u;++i)image->words[base+i]=f.push.words[i];
 return status==P66_HORIZONTAL_ALARM_1302?01302u:result;
}
