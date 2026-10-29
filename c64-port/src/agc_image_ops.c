/* SPDX-License-Identifier: MIT */
/* Register effects of AGC basic instructions on the canonical image; see
 * agc_image_ops.h. Written from the instruction descriptions (one's
 * complement 16-bit accumulator with overflow, editing registers, double
 * word order low-then-high) and checked instruction by instruction against
 * the pinned yaAGC reference by tools/agc_image_ops_differential.py. */
#include "agc_image_ops.h"
#include "agc_hardware_divide.h"
#define W s->words
#define M16 0177777u

static agc_register add16(agc_register a, agc_register b){
 uint32_t t=(uint32_t)a+(uint32_t)b;
 if(t&0200000u)t=(t+1u)&M16;
 return (agc_register)t;
}
static agc_register sx(agc_word v){return (agc_register)((v&077777u)|(((unsigned)v<<1)&0100000u));}
static agc_word oc(agc_register v){return (agc_word)((v&037777u)|((v>>1)&040000u));}
/* +1, -1 (16-bit) or +0 for an accumulator-style value. */
static agc_register ovf(agc_register v){
 switch(v&0140000u){case 040000u:return 1u;case 0100000u:return 0177776u;default:return 0u;}
}
static void endi(agc_image *s){W[1]=sx(oc(W[1]));}

unsigned ai_ea(const agc_image *s,unsigned a){
 a&=01777u; /* the erasable address field; never outside the image */
 return a>=01400u?(unsigned)(((W[3]>>8)&7u)*0400u+(a-01400u)):a;
}
agc_word ai_word(const agc_image *s,unsigned a){return (agc_word)(W[ai_ea(s,a)]&077777u);}
/* A, L and Q are 16-bit registers. Between original instructions Q keeps
 * its sixteenth bit too (TPAGREE parks NEGMAX there); a canonical image at
 * an interpretive frontier holds Q in 15 bits, as the reference rows do.
 * The known case where yaAGC's Q differs there is SWITCH's TS Q, which
 * leaves a negative STATE word sign-extended: bit 16 then copies bit 15,
 * and no later read needs it (#409, IRC_QWATCH in
 * tools/interp_routine_reference.c). */
static agc_register reg(const agc_image *s,unsigned a){return W[a];}
static void setreg(agc_image *s,unsigned a,agc_register v){W[a]=v;}
void ai_put(agc_image *s,unsigned a,agc_word v){
 v&=077777u;a=ai_ea(s,a);
 switch(a){
 case 0:case 1:W[a]=v;return;
 case 2:W[2]=v;return;
 case 3:W[3]=(agc_word)(v&03400u);W[6]=(agc_word)((W[4]&076000u)|(W[3]>>8));return;
 case 4:W[4]=(agc_word)(v&076000u);W[6]=(agc_word)((W[4]&076000u)|(W[3]>>8));return;
 case 6:W[4]=(agc_word)(v&076000u);W[3]=(agc_word)((v&7u)<<8);W[6]=(agc_word)(W[4]|(v&7u));return;
 case 7:return;
 case 020:W[020]=(agc_word)((v>>1)|((v&1u)<<14));return;
 case 021:W[021]=(agc_word)((v>>1)|(v&040000u));return;
 case 022:W[022]=(agc_word)((((unsigned)v<<1)|((unsigned)v>>14))&077777u);return;
 case 023:W[023]=(agc_word)((v>>7)&0177u);return;
 default:W[a]=v;return;
 }
}
/* Memory operand read with the editing write-back of reading orders (by
 * physical location; DCA/DCS re-edit only when named as 020-023). */
static uint8_t edited(const agc_image *s,unsigned a){a=ai_ea(s,a);return a>=020u&&a<=023u;}
static agc_word rd(agc_image *s,unsigned a){agc_word v=ai_word(s,a);if(edited(s,a))ai_put(s,a,v);return v;}

void ai_ca(agc_image *s,unsigned a){if(a>=3u)W[0]=sx(rd(s,a));else if(a)W[0]=reg(s,a);endi(s);}
void ai_caf(agc_image *s,agc_word k){W[0]=sx(k);endi(s);}
void ai_cs(agc_image *s,unsigned a){
 if(a>=3u)W[0]=sx((agc_word)(~rd(s,a)&077777u));
 else W[0]=(agc_register)(~reg(s,a)&M16);
 endi(s);
}
void ai_csf(agc_image *s,agc_word k){W[0]=sx((agc_word)(~k&077777u));endi(s);}
void ai_ad(agc_image *s,unsigned a){W[0]=add16(W[0],a>=3u?sx(rd(s,a)):reg(s,a));endi(s);}
void ai_adf(agc_image *s,agc_word k){W[0]=add16(W[0],sx(k));endi(s);}
void ai_su(agc_image *s,unsigned a){
 if(a==0u)W[0]=M16;
 else W[0]=add16(W[0],a>=3u?sx((agc_word)(~rd(s,a)&077777u)):(agc_register)(~reg(s,a)&M16));
 endi(s);
}
void ai_suf(agc_image *s,agc_word k){W[0]=add16(W[0],sx((agc_word)(~k&077777u)));endi(s);}
void ai_mask(agc_image *s,unsigned a){
 if(a<3u)W[0]=(agc_register)(W[0]&reg(s,a));else W[0]=sx((agc_word)(oc(W[0])&ai_word(s,a)));
 endi(s);
}
void ai_maskf(agc_image *s,agc_word k){W[0]=sx((agc_word)(oc(W[0])&k));endi(s);}
uint8_t ai_ovsk(const agc_image *s){return ovf(s->words[0])!=0u;}
uint8_t ai_ts(agc_image *s,unsigned a){
 agc_register acc=W[0],o=ovf(acc);
 if(a==0u){endi(s);return o!=0u;}
 if(a<3u)setreg(s,a,acc);else ai_put(s,a,oc(acc));
 if(o)W[0]=o;
 endi(s);return o!=0u;
}
void ai_xch(agc_image *s,unsigned a){
 agc_register acc=W[0];
 if(a==0u){endi(s);return;}
 if(a<3u){W[0]=reg(s,a);setreg(s,a,acc);}
 else{W[0]=sx(ai_word(s,a));ai_put(s,a,oc(acc));}
 endi(s);
}
void ai_lxch(agc_image *s,unsigned a){
 agc_register l=W[1];
 if(a==1u){endi(s);return;}
 if(a==7u)W[1]=0;
 else if(a<3u){W[1]=reg(s,a);setreg(s,a,l);}
 else{W[1]=sx(ai_word(s,a));ai_put(s,a,oc(l));}
 endi(s);
}
void ai_qxch(agc_image *s,unsigned a){
 agc_register q=reg(s,2);
 if(a==2u){endi(s);return;}
 if(a==7u)W[2]=0;
 else if(a<3u){setreg(s,2,reg(s,a));setreg(s,a,q);}
 else{W[2]=sx(ai_word(s,a));ai_put(s,a,oc(q));}
 endi(s);
}
void ai_ads(agc_image *s,unsigned a){
 if(a==0u)W[0]=add16(W[0],W[0]);
 else if(a<3u){W[0]=add16(W[0],reg(s,a));setreg(s,a,W[0]);}
 else{W[0]=add16(W[0],sx(ai_word(s,a)));ai_put(s,a,oc(W[0]));}
 endi(s);
}
void ai_incr(agc_image *s,unsigned a){
 if(a<3u)setreg(s,a,add16(1u,reg(s,a)));else ai_put(s,a,oc(add16(1u,sx(ai_word(s,a)))));
 endi(s);
}
void ai_aug(agc_image *s,unsigned a){
 agc_register v=a<3u?reg(s,a):sx(ai_word(s,a)),r=add16((v&0100000u)?M16-1u:1u,v);
 if(a<3u)setreg(s,a,r);else ai_put(s,a,oc(r));
 endi(s);
}
void ai_dim(agc_image *s,unsigned a){
 agc_register v=a<3u?reg(s,a):sx(ai_word(s,a)),r;
 if(v!=0u&&v!=M16){
  r=add16((v&0100000u)?1u:M16-1u,v);
  if(a<3u)setreg(s,a,r);else ai_put(s,a,oc(r));
 }
 endi(s);
}
void ai_das(agc_image *s,unsigned k){
 agc_register lsw,msw;unsigned lo=k+1u;
 if(k==0u){
  /* DDOUBL */
  lsw=add16(W[1],W[1]);msw=add16(W[0],W[0]);
  if((lsw&0140000u)==040000u)msw=add16(msw,1u);else if((lsw&0140000u)==0100000u)msw=add16(msw,M16-1u);
  W[0]=msw;W[1]=sx(oc(lsw));endi(s);return;
 }
 lsw=add16(W[1],lo<3u?reg(s,lo):sx(ai_word(s,lo)));
 msw=add16(W[0],k<3u?reg(s,k):sx(ai_word(s,k)));
 if((lsw&0140000u)==040000u)msw=add16(msw,1u);else if((lsw&0140000u)==0100000u)msw=add16(msw,M16-1u);
 W[0]=ovf(msw);W[1]=0;
 if(lo<3u)setreg(s,lo,sx(oc(lsw)));else ai_put(s,lo,oc(lsw));
 if(k<3u)setreg(s,k,msw);else ai_put(s,k,oc(msw));
 endi(s);
}
void ai_dca(agc_image *s,unsigned k){
 unsigned lo=k+1u;
 if(k==0u){endi(s);return;}
 W[1]=sx(oc(lo<3u?reg(s,lo):sx(ai_word(s,lo))));
 W[0]=k<3u?reg(s,k):sx(ai_word(s,k));
 if(lo>=020u&&lo<=023u)ai_put(s,lo,ai_word(s,lo));
 if(k>=020u&&k<=023u)ai_put(s,k,ai_word(s,k));
 endi(s);
}
void ai_dcaf(agc_image *s,const agc_word k[2]){W[1]=sx(k[1]);W[0]=sx(k[0]);endi(s);}
void ai_dcs(agc_image *s,unsigned k){
 unsigned lo=k+1u;
 if(k==0u){W[0]=(agc_register)(~W[0]&M16);W[1]=sx(oc((agc_register)(~W[1]&M16)));endi(s);return;}
 W[1]=sx(oc((agc_register)(~(unsigned)(lo<3u?reg(s,lo):sx(ai_word(s,lo)))&M16)));
 W[0]=(agc_register)(~(unsigned)(k<3u?reg(s,k):sx(ai_word(s,k)))&M16);
 if(lo>=020u&&lo<=023u)ai_put(s,lo,ai_word(s,lo));
 if(k>=020u&&k<=023u)ai_put(s,k,ai_word(s,k));
 endi(s);
}
void ai_dcsf(agc_image *s,const agc_word k[2]){
 W[1]=sx((agc_word)(~k[1]&077777u));W[0]=sx((agc_word)(~k[0]&077777u));endi(s);
}
void ai_dxch(agc_image *s,unsigned k){
 unsigned lo=k+1u;agc_register op;
 if(k==0u){endi(s);return;}
 if(lo<3u){op=reg(s,lo);setreg(s,lo,W[1]);W[1]=op;}
 else{op=sx(ai_word(s,lo));ai_put(s,lo,oc(W[1]));W[1]=op;}
 W[1]=sx(oc(W[1]));
 if(k<3u){op=reg(s,k);setreg(s,k,W[0]);W[0]=op;}
 else{op=sx(ai_word(s,k));ai_put(s,k,oc(W[0]));W[0]=op;}
 endi(s);
}
void ai_mpf(agc_image *s,agc_word k){
 agc_register_pair p=agc_multiply(W[0],k);
 W[0]=p.a;W[1]=p.l;endi(s);
}
void ai_mp(agc_image *s,unsigned a){ai_mpf(s,a<3u?oc(reg(s,a)):ai_word(s,a));}
static void dv16(agc_image *s,agc_register d){
 agc_register_pair p;p.a=W[0];p.l=W[1];
 p=agc_hardware_divide(p,d);W[0]=p.a;W[1]=p.l;endi(s);
}
void ai_dvf(agc_image *s,agc_word k){dv16(s,sx(k));}
void ai_dv(agc_image *s,unsigned a){dv16(s,a>=3u?sx(ai_word(s,a)):W[2]);}
static uint8_t ccs15(agc_image *s,agc_word v){
 if(v==0u){W[0]=0;return 1;}
 if(v==077777u){W[0]=0;return 3;}
 if(v&040000u){agc_word m=(agc_word)(~v&037777u);W[0]=(agc_register)(m>1u?m-1u:0u);return 2;}
 W[0]=(agc_register)(v>1u?v-1u:0u);return 0;
}
uint8_t ai_ccs(agc_image *s,unsigned a){
 uint8_t c;
 if(a<3u){
  agc_register v=reg(s,a),m=(v&0100000u)?(agc_register)(~v&M16):v;agc_word o=oc(v);
  W[0]=(agc_register)(m>1u?m-1u:0u);
  if(ovf(v)==1u)c=0;else if(ovf(v))c=2;else if(o==0u)c=1;else if(o==077777u)c=3;else c=(o&040000u)?2:0;
  endi(s);return c;
 }
 c=ccs15(s,ai_word(s,a));
 if(edited(s,a))ai_put(s,a,ai_word(s,a));
 endi(s);return c;
}
uint8_t ai_ccsf(agc_image *s,agc_word k){uint8_t c=ccs15(s,(agc_word)(k&077777u));endi(s);return c;}
uint8_t ai_bzf(const agc_image *s){return s->words[0]==0u||s->words[0]==M16;}
uint8_t ai_bzmf(const agc_image *s){return s->words[0]==0u||(s->words[0]&0100000u)!=0u;}
void ai_tc(agc_image *s,agc_word r){W[2]=(agc_word)(r&077777u);endi(s);}
#undef W
