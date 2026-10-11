/* SPDX-License-Identifier: MIT */
#include "guidance_feedback_pairs.h"
#include "guidance_dp_divide.h"
#include "guidance_product.h"
#include "guidance_sum.h"
#include "guidance_vector.h"
static agc_dp pair(const p66_pushdown *p,uint8_t off){agc_dp x={p->words[off],p->words[off+1u]};return x;}
static agc_dp pop(p66_pushdown *p){p->pushloc=(agc_word)(p->pushloc-2u);return pair(p,(uint8_t)(p->pushloc-p->base));}
static void push_load(p66_preparation *s,p66_pushdown *p,agc_dp x,uint8_t do_load){uint8_t off=(uint8_t)(p->pushloc-p->base);agc_word old=p->words[off+1u];p->words[off]=s->mpac[0];p->words[off+1u]=s->mpac[1];p->pushloc=(agc_word)(p->pushloc+2u);if(do_load){s->mpac[0]=x.high;s->mpac[1]=x.low;s->mpac[2]=0;}s->rate.registers.a=0;s->rate.registers.l=agc_extend(old);}
static void load(p66_preparation *s,agc_dp x){agc_word old=s->mpac[1];s->mpac[0]=x.high;s->mpac[1]=x.low;s->mpac[2]=0;s->mpac[7]=0;s->rate.registers.a=0;s->rate.registers.l=agc_extend(old);}
static void sum_q(p66_feedback_pair_control *c,uint16_t mask){if(mask&1u)c->q=3;}
static void divide(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_feedback_pair_control *c,agc_dp x,uint8_t back){agc_word hi=back?s->mpac[0]:x.high,lo=back?s->mpac[1]:x.low;int h=agc_to_int(hi),l=agc_to_int(lo);uint8_t bufzero=(uint8_t)(!h||(h==1&&l<0)||(h==-1&&l>0));enum p66_divide_result r=p66_dp_divide(s,sc,el,d,x,back);if(r==P66_DIVIDE_OVERFLOW)c->q=02413;else if(r==P66_DIVIDE_QUOTIENT)c->q=3;else if(r==P66_DIVIDE_ZERO&&bufzero)c->q=2;}
static int branch(p66_preparation *s,p66_feedback_pair_control *c){uint8_t i;for(i=0;i<3u;++i){int x=agc_to_int(s->mpac[i]);if(x){s->rate.registers.a=agc_diminished_abs(s->mpac[i]);c->q=x<0?06743:3;return x<0?-1:1;}}s->rate.registers.a=0;c->q=06741;return 0;}
static uint8_t valid(const p66_preparation *s,const p66_pushdown *p,const p66_feedback_pair_control *c,uint8_t step){static const uint8_t offsets[20]={0,0,2,4,2,0,2,2,4,2,2,4,6,8,6,6,6,4,0,2};static const agc_word locs[20]={03433,03436,03440,03443,03444,03446,03451,03454,03457,03461,03464,03467,03472,03474,03476,03477,03502,03503,03504,03506};if(step>=20u||c->next_pair!=step||c->loc!=locs[step]||c->edop!=0||s->mpac[7]!=(step<2u?077776:0)||p->base<0401u||p->base>0661u||(p->base-0401u)%054u)return 0;return (uint8_t)(step==18u?(p->pushloc==p->base+2u||p->pushloc==p->base+4u||p->pushloc==p->base+6u):p->pushloc==p->base+offsets[step]);}
static void finish(p66_feedback_pair_control *c,agc_word loc,agc_word fb,agc_word eb,agc_word cyr,uint8_t next){c->pc=loc==03506?06063:06061;c->loc=loc;c->fb=fb;c->eb=eb;c->bb=(agc_word)(fb|(eb>>8));c->cyr=cyr;c->edop=0;c->next_pair=next;}
enum p66_feedback_status p66_feedback_pair(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el,p66_divide_scratch *d,p66_norm_scratch *n,p66_pushdown *p,p66_throttle_feedback *f,const p66_throttle_inputs *in,const agc_word bias[6],p66_feedback_pair_control *c,uint8_t step){static const agc_dp kpip={00321,026706},shft={0,04000},bit1={1,0},scale={00307,017534};agc_dp x;int sign;enum p66_feedback_status status=P66_FEEDBACK_NORMAL;if(!valid(s,p,c,step))return P66_FEEDBACK_OUT_OF_SCOPE;
switch(step){
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==0
case 0:(void)p66_vector_multiply(s,sc,&kpip);(void)p66_vector_add(s,sc,el,bias);c->q=3;n->addrwd=031630;finish(c,03436,064000,02400,020024,1);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==1
case 1:if(p66_vector_length(s,sc,el,d,n,p)!=P66_NORM_OK){finish(c,03437,0,03400,040005,255);c->pc=03402;c->q=03210;c->edop=0052;return P66_FEEDBACK_ALARM_1302;}c->q=3;push_load(s,p,s->this_piptime,1);n->addrwd=01761;finish(c,03440,064000,03400,020012,2);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==2
case 2:sum_q(c,p66_scalar_subtract(s,sc,el,&f->lasttpip));push_load(s,p,s->this_piptime,1);n->addrwd=01761;finish(c,03443,064000,03400,020012,3);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==3
case 3:x=pop(p);f->lasttpip.high=s->mpac[0];f->lasttpip.low=s->mpac[1];load(s,x);n->addrwd=p->pushloc;finish(c,03444,064000,03400,020006,4);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==4
case 4:divide(s,sc,el,d,c,shft,0);x=pop(p);divide(s,sc,el,d,c,x,1);n->addrwd=p->pushloc;finish(c,03446,0,03400,020022,5);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==5
case 5:push_load(s,p,f->fweight,1);(void)p66_scalar_product(s,sc,&bit1);c->q=3;n->addrwd=03533;finish(c,03451,064000,03400,020036,6);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==6
case 6:divide(s,sc,el,d,c,in->mass,0);divide(s,sc,el,d,c,scale,0);n->addrwd=02005;finish(c,03454,0,03400,020021,7);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==7
case 7:x=pair(p,0);sum_q(c,p66_scalar_add(s,sc,el,&x));push_load(s,p,pair(p,20),1);n->addrwd=(agc_word)(p->base+20u);finish(c,03457,064000,03400,020012,8);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==8
case 8:divide(s,sc,el,d,c,pair(p,22),0);x=pop(p);sum_q(c,p66_scalar_subtract(s,sc,el,&x));n->addrwd=p->pushloc;finish(c,03461,064000,03400,020032,9);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==9
case 9:(void)p66_scalar_product(s,sc,&in->lag_tau);c->q=3;sum_q(c,p66_scalar_add(s,sc,el,&f->afc));n->addrwd=01463;finish(c,03464,064000,03400,020034,10);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==10
case 10:push_load(s,p,in->maxforce,1);divide(s,sc,el,d,c,in->mass,0);n->addrwd=01644;finish(c,03467,0,01000,020021,11);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==11
case 11:push_load(s,p,in->minforce,1);divide(s,sc,el,d,c,in->mass,0);n->addrwd=01644;finish(c,03472,0,01000,020021,12);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==12
case 12:push_load(s,p,bit1,0);x=pair(p,2);sum_q(c,p66_scalar_reverse_subtract(s,sc,el,&x));n->addrwd=(agc_word)(p->base+2u);finish(c,03474,064000,03400,020033,13);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==13
case 13:c->polish=065504;sign=branch(s,c);load(s,pop(p));n->addrwd=p->pushloc;if(sign<0){c->polish=065504;status=P66_FEEDBACK_MINIMUM;finish(c,03504,064000,03400,020006,18);}else finish(c,03476,064000,03400,020006,14);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==14
case 14:load(s,pop(p));n->addrwd=p->pushloc;push_load(s,p,bit1,0);finish(c,03477,0,03400,040007,15);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==15
case 15:x=pair(p,2);sum_q(c,p66_scalar_reverse_subtract(s,sc,el,&x));n->addrwd=(agc_word)(p->base+2u);c->polish=065504;sign=branch(s,c);if(sign>=0){c->polish=065504;load(s,pop(p));n->addrwd=p->pushloc;status=P66_FEEDBACK_MAXIMUM;finish(c,03504,064000,03400,020006,18);}else finish(c,03502,02000,03400,050005,16);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==16
case 16:load(s,pop(p));n->addrwd=p->pushloc;finish(c,03503,064000,03400,020006,17);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==17
case 17:load(s,pop(p));n->addrwd=p->pushloc;finish(c,03504,064000,03400,020006,18);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==18
case 18:p->pushloc=(agc_word)(p->base+2u);n->addrwd=p->pushloc;s->rate.registers.a=agc_extend(n->addrwd);finish(c,03506,064000,03400,020037,19);break;
#endif
#if !defined(FEEDBACK_PAIR_SELECTED) || FEEDBACK_PAIR_SELECTED==19
case 19:x=pop(p);f->afc.high=s->mpac[0];f->afc.low=s->mpac[1];load(s,x);c->q=3;n->addrwd=p->pushloc;finish(c,03507,064000,03400,020006,255);break;
#endif
default:return P66_FEEDBACK_OUT_OF_SCOPE;
}return status;}

enum p66_feedback_status p66_feedback_timestamp_after_length(p66_preparation *s,p66_norm_scratch *n,p66_pushdown *p,p66_feedback_pair_control *c){if(s->mpac[7]!=0||p->base<0401u||p->base>0661u||(p->base-0401u)%054u||p->pushloc!=p->base||c->loc!=03437||c->edop!=0052)return P66_FEEDBACK_OUT_OF_SCOPE;push_load(s,p,s->this_piptime,1);n->addrwd=01761;finish(c,03440,064000,03400,020012,2);return P66_FEEDBACK_NORMAL;}
