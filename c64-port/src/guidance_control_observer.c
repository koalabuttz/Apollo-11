/* SPDX-License-Identifier: MIT */
#include "guidance_control_observer.h"
#include "agc_multiword.h"
static agc_register add(agc_register a,agc_word b){return agc_add_register(a,agc_extend(b));}
static agc_register carry(int sign){return sign>0?1u:0177776u;}
static void agree(agc_dp *p){
 int sign=agc_to_int(p->high);agc_word limit;agc_register a;int ov;
 if(!sign)sign=agc_to_int(p->low);
 if(!sign){p->high=p->low=0;return;}
 limit=sign>0?037777:040000;
 /* TPAGREE clears the third limb first. Its first carry chain therefore
  * leaves the same carry/limit combination entering the lower DP limb. */
 a=add(agc_extend(limit),carry(sign));a=add(a,0);ov=agc_overflow(a);
 a=add(ov?carry(ov):0,limit);a=add(a,p->low);p->low=agc_correct(a);ov=agc_overflow(a);
 a=add(ov?carry(ov):0,limit);a=add(a,p->high);p->high=agc_correct(a);
 if(!agc_overflow(a))p->low=agc_correct(a);
}
static void canonical(agc_dp *p){agc_register a=add(040000,p->low);int ov=agc_overflow(a);p->low=agc_correct(a);a=add(ov?carry(ov):0,037777);p->high=agc_correct(add(a,p->high));}
agc_word p66_ddv_return_q(agc_word q,agc_dp a,agc_dp b){
 int high=agc_to_int(b.high),low=agc_to_int(b.low);agc_register r;
 if((high==1&&low<0)||(high==-1&&low>0)){b.low=agc_correct(add(high>0?040000:0137777,b.low));b.high=0;high=0;}
 if(!high){agree(&a);q=2;if(agc_to_int(a.high))return 02413;a.high=a.low;a.low=b.high;b.high=b.low;b.low=0;high=agc_to_int(b.high);if(!high)return 02413;}
 if(high<0){b.high=agc_negate(b.high);b.low=agc_negate(b.low);}
 high=agc_to_int(a.high);low=agc_to_int(a.low);
 if(!high&&!low)return q;
 if(high<0||(!high&&low<0)){a.high=agc_negate(a.high);a.low=agc_negate(a.low);}
 r=add(add(agc_extend(agc_negate(a.high)),077776),b.high);
 if(agc_to_int(agc_correct(r))<=0){canonical(&a);canonical(&b);r=add(agc_extend(agc_negate(a.high)),b.high);high=agc_to_int(agc_correct(r));if(high<0)return 02413;if(!high&&agc_to_int(agc_correct(add(agc_extend(agc_negate(a.low)),b.low)))<=0)return 02413;}
 return 3;
}
static agc_dp reverse(agc_dp a,agc_dp b,agc_word *q){
 agc_register_pair r;agc_register x;agc_word limit;int ov;
 r.a=agc_extend(agc_negate(a.high));r.l=agc_extend(agc_negate(a.low));agc_double_add(&r,&b);
 if(r.a){*q=3;limit=r.a==1?037777:040000;x=add(agc_extend(limit),carry(r.a==1?1:-1));x=add(x,b.low);b.low=agc_correct(x);ov=agc_overflow(x);x=add(ov?carry(ov):0,limit);b.high=agc_correct(add(x,b.high));}
 return b;
}
agc_word p66_acceleration_return_q(uint8_t phase,agc_word q,agc_dp mpac,agc_dp desired,agc_dp taurod,const p66_norm_scratch *after,agc_word after_mptemp){
 static const agc_dp scale={01440,0};
 if(phase==0){mpac=reverse(mpac,desired,&q);return p66_ddv_return_q(q,mpac,taurod);}
 if(phase==1)return after->addrwd==0&&after_mptemp!=AGC_MASK?02314:3;
 if(phase==2)return p66_ddv_return_q(q,mpac,scale);
 if(phase==3)return 3;
 return q;
}
