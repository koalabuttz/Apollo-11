/* SPDX-License-Identifier: MIT */
#include "horizontal_phased_cases.h"
void hhp_input(horizontal_phased_case *c,uint16_t n){static const agc_word control[12]={06061,01030,064000,03400,064007,03514,064007,0,0,040000,0,1};unsigned i;hh_input(&c->source,n);for(i=0;i<12u;++i)c->control[i]=control[i];c->next=0;c->unit_status=0;}
void hhp_words(const horizontal_phased_case *c,uint16_t w[HHP_WORDS]){unsigned i;hh_words(&c->source,w);for(i=0;i<12u;++i)w[HH_WORDS+i]=c->control[i];}
void hhp_from_words(horizontal_phased_case *c,const uint16_t w[HHP_WORDS]){unsigned i;hh_from_words(&c->source,w);for(i=0;i<12u;++i)c->control[i]=w[HH_WORDS+i];c->unit_status=0;}
static void control(horizontal_phased_case *c,agc_word pc,agc_word q,agc_word fb,agc_word eb,agc_word loc,agc_word cyr,agc_word edop){c->control[0]=pc;c->control[1]=q;c->control[2]=fb;c->control[3]=eb;c->control[4]=(agc_word)(fb|(eb>>8));c->control[5]=loc;c->control[7]=cyr;c->control[8]=edop;}
static uint8_t left_overflow(p66_preparation *s,p66_scale_scratch *sc,p66_elapsed_state *el){static const uint8_t at[3]={0,3,5};p66_preparation copy=*s;p66_scale_scratch scale=*sc;p66_elapsed_state elapsed=*el;uint8_t i,j,overflow=0;for(i=0;i<2u;++i){for(j=0;j<3u;++j){agc_dp x={copy.mpac[at[j]],copy.mpac[at[j]+1u]};agc_register_pair r={agc_extend(x.high),agc_extend(x.low)};agc_double_add(&r,&x);if(r.a)overflow=1;}(void)p66_vector_shift_left(&copy,&scale,&elapsed,1,1);}return overflow;}
unsigned hhp_step(horizontal_phased_case *c,uint8_t *stage){horizontal_case *x=&c->source;p66_preparation *s=&x->previous.base.previous.previous.previous.preparation;p66_scale_scratch *sc=&x->previous.base.previous.previous.previous.scratch;p66_elapsed_state *el=&x->previous.base.previous.elapsed;p66_divide_scratch *d=&x->previous.base.scratch;p66_norm_scratch *n=&x->previous.norm;p66_pushdown *p=&x->previous.base.previous.previous.push;unsigned result=0;uint16_t mask;uint8_t overflow;*stage=c->next;switch(c->next){
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==0
case 0:result=hh_apply(x,0);control(c,06061,c->control[1],064000,03400,03516,020000,0);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==1
case 1:overflow=left_overflow(s,sc,el);(void)p66_vector_shift_left(s,sc,el,2,1);mask=p66_vector_add(s,sc,el,x->delvs);n->addrwd=031733;control(c,06061,((mask&0111u)||overflow)?3:c->control[1],064000,03400,03520,020024,0);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==2
case 2:result=hh_apply(x,2);control(c,06061,3,064000,03400,03522,020014,0);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==3
case 3:result=(unsigned)p66_unit_square_phase(s,sc,el,d,n,p);control(c,03031,0140,0,03400,03523,5,0016);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==4
case 4:result=(unsigned)p66_unit_root_phase(s,sc,el,d,n,p);if(result==P66_UNIT_OVERFLOW){*stage=5;c->next=5;c->unit_status=result;control(c,06061,02413,0,03400,03523,5,0016);}else control(c,03043,3,0,03400,03523,5,0016);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==5
case 5:result=(unsigned)p66_unit_finish_phase(s,sc,el,d,n,p);c->unit_status=result;control(c,06061,result==P66_UNIT_OVERFLOW?02413:3,0,03400,03523,5,0016);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==6
case 6:result=(unsigned)p66_horizontal_radial_product(s,sc,n,p,&x->hdotdisp);if(!result)result=c->unit_status;control(c,06061,3,064000,03400,03524,020003,0);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==7
case 7:result=hh_apply(x,4);control(c,06061,3,064000,03400,03525,020026,0);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==8
case 8:result=(unsigned)p66_length_square_phase(s,sc,el,d,n,p);control(c,03202,0140,0,03400,03526,040005,0);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==9
case 9:result=(unsigned)p66_length_root_phase(s,sc,el,d,n,p);control(c,03210,3,0,03400,03526,040005,0);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==10
case 10:result=(unsigned)p66_length_finish_phase(s,sc,el,d,n,p);control(c,06061,3,0,03400,03526,040005,0);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==11
case 11:result=hh_apply(x,6);control(c,06061,3,064000,02000,03527,040005,0);break;
#endif
#if !defined(HORIZONTAL_PHASE_SELECTED) || HORIZONTAL_PHASE_SELECTED==12
case 12:result=hh_apply(x,7);control(c,03531,06747,064000,03400,03530,0,0);break;
#endif
default:return P66_HORIZONTAL_OUT_OF_SCOPE;}
++c->next;return result;}
