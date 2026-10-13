/* SPDX-License-Identifier: MIT */
/* Compact runtime interpreter (bootstrap: the lmi_ bodies in one unit over a fixed image). */
#include "interp_runtime.h"
#include "interp_runtime_tables.h"
#include "interp_mission.h"
#include "agc_multiword.h"
#include "agc_hardware_divide.h"
#include "rope_image.h"
#include "p66_bias.h"
/* Functions stay out of line: on llvm-mos inlining grows this unit by a
 * third (larger frames and zero-page spills). */
#define NI __attribute__((noinline))
#ifdef __mos__
#define W ((agc_word *)APOLLO_SHARED_ADDRESS)
#else
static agc_word *ic_image;
#define W ic_image
#endif
static p66_bias_io *io;

typedef apollo_overlay_state agc_image;
static NI unsigned ai_ea(unsigned address);
static NI agc_word ai_word(unsigned address);
static NI void ai_put(unsigned address, agc_word);
static NI void ai_ca(unsigned);
static NI void ai_caf(agc_word);
static NI void ai_cs(unsigned);
static NI void ai_csf(agc_word);
static NI void ai_ad(unsigned);
static NI void ai_adf(agc_word);
static NI void ai_su(unsigned);
static NI void ai_mask(unsigned);
static NI void ai_maskf(agc_word);
static NI uint8_t ai_ts(unsigned);
static NI uint8_t ai_ovsk(void);
static NI void ai_ads(unsigned);
static NI void ai_incr(unsigned);
static NI void ai_aug(unsigned);
static NI void ai_dim(unsigned);
static NI void ai_das(unsigned);
static NI void ai_dca(unsigned);
static NI void ai_dcaf(const agc_word[2]);
static NI void ai_dcs(unsigned);
static NI void ai_dcsf(const agc_word[2]);
static NI void ai_dxch(unsigned);
static NI void ai_mp(unsigned);
static NI void ai_mpf(agc_word);
static NI void ai_dv(unsigned);
static NI uint8_t ai_ccs(unsigned);
static NI uint8_t ai_ccsf(agc_word);
static NI uint8_t ai_bzf(void);
static NI uint8_t ai_bzmf(void);
static NI void ai_tc(agc_word return_address);
#define ai_double() ai_ad(0u)
#define ai_com() ai_cs(0u)
#define ai_dcom() ai_dcs(0u)
#define ai_ddoubl() ai_das(0u)
#define ai_square() ai_mp(0u)
#define ai_zl() ai_lxch(7u)
#define ai_zq() ai_qxch(7u)
static NI void lmi_newops(agc_word pair);
static NI void lmi_danzig_next(void);
static NI void lmi_danzig_right(void);
static NI void lmi_noibnksw_right(void);
static NI void lmi_direct(agc_word operand, agc_word target);
static NI void lmi_indexed(agc_word operand, agc_word target);
static NI void lmi_misc(agc_word operand, agc_word target);
static NI void lmi_unary(agc_word target);
static NI void lmi_sload(void);
static NI void lmi_setpd(const agc_word *);
static NI uint8_t lmi_vdef(void);
static NI uint8_t lmi_axt(void);
static NI uint8_t lmi_incr(void);
static NI uint8_t lmi_switch(void);
static NI void lmi_swbranch(agc_word target_word);
static NI void lmi_goto(void);
static NI void lmi_rvq(void);
static NI void lmi_diradres(agc_word operand, agc_word target);
static NI void lmi_diradres_pushup(agc_word next, agc_word target);
static NI void lmi_address_pushup(agc_word next, agc_word target);
static NI void lmi_store_code(agc_word code);
static NI void lmi_stadr(agc_word word);
static NI void lmi_dodload(void);
static NI void lmi_dovload(void);
static NI void lmi_dload_op(const agc_word *k);
static NI void lmi_vload(const agc_word *k);
static NI void lmi_pdvl(const agc_word *k);
static NI void lmi_pddl_op(const agc_word *k);
static NI uint8_t lmi_push(void);
static NI void lmi_dad(const agc_word *k);
static NI void lmi_dsu(const agc_word *k);
static NI void lmi_bdsu(const agc_word *k);
static NI void lmi_dmp(const agc_word *k);
static NI void lmi_dmpr(const agc_word *k);
static NI void lmi_short_dispatch(void);
static NI void lmi_shortt(void);
static NI void lmi_shortv(void);
static NI void lmi_gshift(const agc_word *);
static NI void lmi_genshift(void);
static NI void lmi_tslc(const agc_word *);
static NI uint8_t lmi_comp(void);
static NI void lmi_sign(const agc_word *k);
static NI void lmi_abs(void);
static NI uint8_t lmi_dsq(void);
static NI uint8_t lmi_vsq(void);
static NI void lmi_dot(const agc_word *k);
static NI void lmi_ddv(const agc_word *k);
static NI void lmi_bddv(const agc_word *k);
static NI uint8_t lmi_sqrt(void);
static NI uint8_t lmi_sin(void);
static NI uint8_t lmi_cos(void);
static NI uint8_t lmi_acos(void);
static NI uint8_t lmi_asin(void);
static NI uint8_t lmi_abvalabs(void);
static NI uint8_t lmi_unit(void);
static NI void lmi_vxsc(const agc_word *k);
static NI void lmi_vxv(const agc_word *k);
static NI void lmi_mxv(const agc_word *k);
static NI void lmi_vxm(const agc_word *k);
static NI void lmi_vproj(const agc_word *k);
static NI void lmi_vsc(const agc_word *k);
enum {LMI_BOV_NONE,LMI_BOV_GOTO,LMI_BOV_RTB,LMI_BOV_OUT_OF_SCOPE};
static NI uint8_t lmi_bplbmn(void);
static NI uint8_t lmi_bov(void);
static NI void lmi_rtb(void);
static NI void lmi_swreturn(void);
static NI void lmi_poly(const agc_word *table);
static NI void lmi_vad(const agc_word *k);
static NI void lmi_vsu(const agc_word *k);
static NI void lmi_bvsu(const agc_word *k);
static NI void lmi_tad(const agc_word *k);
static NI uint8_t lmi_round(void);
static NI void lmi_ssp(agc_word word);
static NI uint8_t lmi_xad(void);
static NI uint8_t lmi_xsu(void);
static NI uint8_t lmi_lxa(void);
static NI uint8_t lmi_lxc(void);
static NI uint8_t lmi_sxa(void);
static NI uint8_t lmi_xchx(void);
static NI uint8_t lmi_axc(void);
static NI void lmi_ita(void);
static NI uint8_t lmi_tix(void);
static NI uint8_t lmi_bhiz(void);
static NI void lmi_call(void);
static NI void lmi_goto_any(void);
static NI void lmi_15bitadr(agc_word word, agc_word target);
static NI uint8_t lmi_bzegoto(void);
static NI void lmi_tload(void);
static NI void lmi_tload_fixed(const agc_word *k);
static NI void lmi_sload_fixed(const agc_word *k);
static NI void lmi_exit(void);
static NI void lmi_intpret(void);

/* ---- agc_image_ops ---- */
/* SPDX-License-Identifier: MIT */
/* Register effects of AGC basic instructions on the canonical image; see
 * agc_image_ops.h. Written from the instruction descriptions (one's
 * complement 16-bit accumulator with overflow, editing registers, double
 * word order low-then-high) and checked instruction by instruction against
 * the pinned yaAGC reference by tools/agc_image_ops_differential.py. */
#define M16 0177777u

static NI agc_register add16(agc_register a, agc_register b){
 agc_register t=(agc_register)(a+b);
 if(t<a)++t;   /* end-around carry */
 return t;
}
static NI agc_register sx(agc_word v){return (agc_register)((v&077777u)|(((unsigned)v<<1)&0100000u));}
static NI agc_word oc(agc_register v){return (agc_word)((v&037777u)|((v>>1)&040000u));}
/* +1, -1 (16-bit) or +0 for an accumulator-style value. */
static NI agc_register ovf(agc_register v){
 switch(v&0140000u){case 040000u:return 1u;case 0100000u:return 0177776u;default:return 0u;}
}
/* L overflow-corrected and sign-extended, as after every instruction. */
static NI void endi(void){agc_register l=W[1];W[1]=(agc_register)((l&0137777u)|((l>>1)&040000u));}

static NI unsigned ai_ea(unsigned a){
 a&=01777u; /* the erasable address field; never outside the image */
 if(a<01400u)return a;
 return (unsigned)((W[3]&03400u)+(a-01400u));
}
static NI agc_word ai_word(unsigned a){return (agc_word)(W[ai_ea(a)]&077777u);}
/* A, L and Q are 16-bit registers. Between original instructions Q keeps
 * its sixteenth bit too (TPAGREE parks NEGMAX there); a canonical image at
 * an interpretive frontier holds Q in 15 bits, as the reference rows do. */
static NI agc_register reg(unsigned a){return W[a];}
static NI void setreg(unsigned a,agc_register v){W[a]=v;}
static NI void ai_put(unsigned a,agc_word v){
 v&=077777u;a=ai_ea(a);
 if(a>=024u){W[a]=v;return;}
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
static NI uint8_t edited(unsigned a){a=ai_ea(a);return a>=020u&&a<=023u;}
static NI agc_word rd(unsigned a){
 agc_word v;
 a=ai_ea(a);v=(agc_word)(W[a]&077777u);
 if(a>=020u&&a<=023u)ai_put(a,v);
 return v;
}

static NI void ai_ca(unsigned a){if(a>=3u)W[0]=sx(rd(a));else if(a)W[0]=reg(a);endi();}
static NI void ai_caf(agc_word k){W[0]=sx(k);endi();}
static NI void ai_cs(unsigned a){
 if(a>=3u)W[0]=sx((agc_word)(~rd(a)&077777u));
 else W[0]=(agc_register)(~reg(a)&M16);
 endi();
}
static NI void ai_csf(agc_word k){W[0]=sx((agc_word)(~k&077777u));endi();}
static NI void ai_ad(unsigned a){W[0]=add16(W[0],a>=3u?sx(rd(a)):reg(a));endi();}
static NI void ai_adf(agc_word k){W[0]=add16(W[0],sx(k));endi();}
static NI void ai_su(unsigned a){
 if(a==0u)W[0]=M16;
 else W[0]=add16(W[0],a>=3u?sx((agc_word)(~rd(a)&077777u)):(agc_register)(~reg(a)&M16));
 endi();
}
static NI void ai_mask(unsigned a){
 if(a<3u)W[0]=(agc_register)(W[0]&reg(a));else W[0]=sx((agc_word)(oc(W[0])&ai_word(a)));
 endi();
}
static NI void ai_maskf(agc_word k){W[0]=sx((agc_word)(oc(W[0])&k));endi();}
static NI uint8_t ai_ovsk(void){return ovf(W[0])!=0u;}
static NI uint8_t ai_ts(unsigned a){
 agc_register acc=W[0],o=ovf(acc);
 if(a==0u){endi();return o!=0u;}
 if(a<3u)setreg(a,acc);else ai_put(a,oc(acc));
 if(o)W[0]=o;
 endi();return o!=0u;
}
/* Register-or-memory operand of the exchanging and modifying instructions
 * (memory read without the editing write-back, written back edited). */
static NI agc_register get16(unsigned a){return a<3u?W[a]:sx(ai_word(a));}
static NI void set16(unsigned a,agc_register v){if(a<3u)W[a]=v;else ai_put(a,oc(v));}
static NI void xchr(unsigned r,unsigned a){agc_register v=get16(a);set16(a,W[r]);W[r]=v;endi();}
#define ai_xch(a) xchr(0u,(a))
#define ai_lxch(a) xchr(1u,(a))
#define ai_qxch(a) xchr(2u,(a))
static NI void ai_ads(unsigned a){W[0]=add16(W[0],get16(a));set16(a,W[0]);endi();}
static NI void ai_incr(unsigned a){set16(a,add16(1u,get16(a)));endi();}
static NI void ai_aug(unsigned a){
 agc_register v=get16(a);
 set16(a,add16((v&0100000u)?M16-1u:1u,v));endi();
}
static NI void ai_dim(unsigned a){
 agc_register v=get16(a);
 if(v!=0u&&v!=M16)set16(a,add16((v&0100000u)?1u:M16-1u,v));
 endi();
}
/* DAS (DDOUBL for A). */
static NI void ai_das(unsigned k){
 agc_register lsw,msw;unsigned lo=k+1u;
 lsw=add16(W[1],get16(lo));msw=add16(W[0],get16(k));
 if((lsw&0140000u)==040000u)msw=add16(msw,1u);else if((lsw&0140000u)==0100000u)msw=add16(msw,M16-1u);
 W[0]=ovf(msw);W[1]=0;
 set16(lo,sx(oc(lsw)));set16(k,msw);
 endi();
}
static NI void reedit(unsigned a){if(a>=020u&&a<=023u)ai_put(a,ai_word(a));}
static NI void ai_dca(unsigned k){
 unsigned lo=k+1u;
 W[1]=sx(oc(get16(lo)));W[0]=get16(k);reedit(lo);reedit(k);endi();
}
static NI void ai_dcaf(const agc_word k[2]){W[1]=sx(k[1]);W[0]=sx(k[0]);endi();}
static NI void ai_dcs(unsigned k){
 unsigned lo=k+1u;
 W[1]=sx(oc((agc_register)(~get16(lo)&M16)));W[0]=(agc_register)(~get16(k)&M16);reedit(lo);reedit(k);endi();
}
static NI void ai_dcsf(const agc_word k[2]){
 W[1]=sx((agc_word)(~k[1]&077777u));W[0]=sx((agc_word)(~k[0]&077777u));endi();
}
static NI void ai_dxch(unsigned k){
 agc_register op=get16(k+1u);
 set16(k+1u,W[1]);W[1]=sx(oc(op));
 op=get16(k);set16(k,W[0]);W[0]=op;
 endi();
}
static NI void ai_mpf(agc_word k){
 agc_register_pair p=agc_multiply(W[0],k);
 W[0]=p.a;W[1]=p.l;endi();
}
static NI void ai_mp(unsigned a){ai_mpf(a<3u?oc(reg(a)):ai_word(a));}
static NI void dv16(agc_register d){
 agc_register_pair p;p.a=W[0];p.l=W[1];
 p=agc_hardware_divide(p,d);W[0]=p.a;W[1]=p.l;endi();
}
static NI void ai_dv(unsigned a){dv16(a>=3u?sx(ai_word(a)):W[2]);}
static NI uint8_t ccs15(agc_word v){
 if(v==0u){W[0]=0;return 1;}
 if(v==077777u){W[0]=0;return 3;}
 if(v&040000u){agc_word m=(agc_word)(~v&037777u);W[0]=(agc_register)(m>1u?m-1u:0u);return 2;}
 W[0]=(agc_register)(v>1u?v-1u:0u);return 0;
}
static NI uint8_t ai_ccs(unsigned a){
 uint8_t c;
 if(a<3u){
  agc_register v=reg(a),m=(v&0100000u)?(agc_register)(~v&M16):v;agc_word o=oc(v);
  W[0]=(agc_register)(m>1u?m-1u:0u);
  if(ovf(v)==1u)c=0;else if(ovf(v))c=2;else if(o==0u)c=1;else if(o==077777u)c=3;else c=(o&040000u)?2:0;
  endi();return c;
 }
 c=ccs15(ai_word(a));
 if(edited(a))ai_put(a,ai_word(a));
 endi();return c;
}
static NI uint8_t ai_ccsf(agc_word k){uint8_t c=ccs15((agc_word)(k&077777u));endi();return c;}
static NI uint8_t ai_bzf(void){return W[0]==0u||W[0]==M16;}
static NI uint8_t ai_bzmf(void){return W[0]==0u||(W[0]&0100000u)!=0u;}
static NI void ai_tc(agc_word r){W[2]=(agc_word)(r&077777u);endi();}


/* ---- steps ---- */
/* SPDX-License-Identifier: MIT */
/* Compiled Luminary099 interpretive dispatch paths and simple order bodies.
 * Source: Luminary099/INTERPRETER.agc (DANZIG, NEWOPS, OPJUMP, ADDRESS,
 * DIRADRES, INDEX, OPJUMP2/3, 15BITADR, DOSTORE/STORE, load/store, index,
 * switch and branch orders). The static helpers below give the register
 * effects of the original basic instructions that these fixed sequences
 * use; they are called only from the compiled sequences in this file. */

/* Fixed constants named in the source. */
#define LOW7 0177u
#define LOW8 0377u
#define LOW10 01777u
#define LOW11 03777u
#define HIGH4 074000u
#define HIGH9 077700u
#define BIT1 1u
#define BIT5 020u
#define BIT15 040000u
#define B12T14 034000u
#define OCT1400 01400u
#define TWO_K 02000u
#define ENDVAC_NEG 077722u /* -ENDVAC DEC -45 */
#define ENDERAS_NEG 074056u /* -ENDERAS DEC -2001 */
#define DEC45 055u

static NI void lmi_danzig_next(void){
 /* DANZIG CA BANKSET, TS BBANK; NOIBNKSW CCS EDOP (+0); CCS NEWJOB (+0);
  * INCR LOC; NEWOPS. */
 ai_ca(0165);(void)ai_ts(6);(void)ai_ccs(023);(void)ai_ccs(067);ai_incr(0164);W[5]=IF_(06070,06037);
}
static NI void opjump(void){
 /* OPJUMP TS CYR; CCS CYR. */
 (void)ai_ts(020);(void)ai_ccs(020);
}
static NI void lmi_newops(agc_word pair){
 /* NEWOPS INDEX LOC, CA 0; CCS A (negative pair); TS EDOP; MASK LOW7.
  * Every instruction ends with L overflow-corrected and sign-extended. */
 W[1]=agc_extend(agc_correct(W[1]));ai_caf(pair);(void)ai_ccs(0);(void)ai_ts(023);ai_maskf(LOW7);opjump();
}
static NI void lmi_danzig_right(void){
 /* DANZIG CA BANKSET, TS BBANK; CCS EDOP (positive) TCF OPJUMP. */
 ai_ca(0165);(void)ai_ts(6);(void)ai_ccs(023);opjump();
}
static NI void indjump(agc_word target){
 /* ITRnn INDEX CYR; 7 (or 3) INDJUMP -1: the transfer reads CYR without
  * rewriting it. */
 W[5]=target;
}
static NI void lmi_direct(agc_word operand,agc_word target){
 /* ADDRESS MASK BIT1; CCS A (+0); DIRADRES INDEX LOC, CS 1; CCS A
  * (negative, an address is given); INCR LOC; TS ADDRWD; AD -ENDVAC;
  * CCS A. */
 ai_maskf(BIT1);(void)ai_ccs(0);ai_csf(operand);(void)ai_ccs(0);ai_incr(0164);(void)ai_ts(0116);ai_adf(ENDVAC_NEG);
 if(ai_ccs(0)==2u){
  /* NETZERO CA FIXLOC; ADS ADDRWD. */
  ai_ca(0120);ai_ads(0116);indjump(target);return;
 }
 /* AD -ENDERAS; IERASTST EXTEND BZMF. */
 ai_adf(ENDERAS_NEG);
 if(W[0]==0||(W[0]&0100000u)){
  /* GEADDR MASK LOW8; AD OCT1400; XCH ADDRWD; TS EBANK. */
  ai_maskf(LOW8);ai_adf(OCT1400);ai_xch(0116);(void)ai_ts(3);indjump(target);return;
 }
 /* MASK LOW10; AD LOW10; XCH ADDRWD; AD INTBIT15; TS FBANK. */
 ai_maskf(LOW10);ai_adf(LOW10);ai_xch(0116);ai_ad(0115);(void)ai_ts(4);indjump(target);
}
static NI void lmi_indexed(agc_word operand,agc_word target){
 /* ADDRESS MASK BIT1; CCS A (positive) TCF INDEX. INDEX CA FIXLOC;
  * TS INDEXLOC; INCR LOC; INDEX LOC, CS 0; CCS A; [INCR INDEXLOC; NOOP;]
  * TS ADDRWD; MASK HIGH4; BZF INDEX2; [CA INTBIT15; ADS ADDRWD;] INDEX2
  * INDEX INDEXLOC, CS X1; ADS ADDRWD; MASK HIGH9; BZF INDWORK; MASK
  * HIGH4; BZF INDERASE; fixed bank otherwise. */
 ai_maskf(BIT1);(void)ai_ccs(0);ai_ca(0120);(void)ai_ts(0130);ai_incr(0164);ai_csf(operand);
 if(ai_ccs(0)==0u)ai_incr(0130);
 (void)ai_ts(0116);ai_maskf(HIGH4);
 if(W[0]){ai_ca(0115);ai_ads(0116);}
 ai_cs((unsigned)(W[0130]+046u));ai_ads(0116);ai_maskf(HIGH9);
 if(!W[0]){ai_ca(0120);ai_ads(0116);indjump(target);return;}
 ai_maskf(HIGH4);
 if(!W[0]){ai_caf(OCT1400);ai_xch(0116);(void)ai_ts(3);ai_maskf(LOW8);ai_ads(0116);indjump(target);return;}
 ai_ca(0116);(void)ai_ts(4);ai_maskf(LOW10);ai_adf(TWO_K);(void)ai_ts(0116);indjump(target);
}
static NI void lmi_misc(agc_word operand,agc_word target){
 /* OPJUMP2 CCS CYR (negative); 15BITADR INCR LOC; INDEX LOC, CA 0;
  * TS POLISH; CAF LOW7+2K; TS FBANK; MASK CYR; INDEX A; TCF MISCJUMP. */
 (void)ai_ccs(020);ai_incr(0164);ai_caf(operand);(void)ai_ts(0117);ai_caf(02177u);(void)ai_ts(4);ai_mask(020);W[5]=target;
}
static NI void lmi_unary(agc_word target){
 /* OPJUMP2 CCS CYR (positive); OPJUMP3 TS FBANK; CCS CYR (positive);
  * INDEX A; TCF UNAJUMP. */
 (void)ai_ccs(020);(void)ai_ts(4);(void)ai_ccs(020);W[5]=target;
}
static NI void danzig_entry(void){W[5]=IF_(06061,06030);}

/* STORE CS ADDRWD; AD DEC45; CCS A; work area CA FIXLOC, otherwise CA
 * OCT1400; XCH ADDRWD; TS EBANK; MASK LOW8; AHEAD5 ADS ADDRWD; STARTSTO
 * EXTEND, DCA MPAC; INDEX ADDRWD, DXCH 0; CCS MODE; TSTORE/VSTORE; TC Q. */
static NI void lmi_sload(void){
 /* SLOAD ZL; INDEX ADDRWD, CA 0; TCF SLOAD2. */
 W[1]=0;ai_ca(W[0116]);ai_dxch(0154);ai_caf(0);(void)ai_ts(0156);(void)ai_ts(0163);danzig_entry();
}
static NI void lmi_setpd(const agc_word *k){(void)k;
 /* SETPD CA ADDRWD; TS PUSHLOC; TCF NOIBNKSW (DANZIG without BBANK). */
 ai_ca(0116);(void)ai_ts(0166);W[5]=IF_(06063,06032);
}
static NI uint8_t lmi_vdef(void){
 /* VDEF CS FOUR; ADS PUSHLOC; EXTEND, INDEX A, DCA 2; DXCH MPAC +3;
  * EXTEND, INDEX PUSHLOC, DCA 0; DXCH MPAC +5; VMODE CS ONE; NEWMODE. */
 unsigned at;
 ai_csf(4);ai_ads(0166);at=(unsigned)agc_correct(W[0]);ai_dca(at+2u);ai_dxch(0157);ai_dca(W[0166]);ai_dxch(0161);ai_csf(1);(void)ai_ts(0163);danzig_entry();
 return 0;
}
/* TAGSUB CA FIXLOC; TS INDEXLOC; CCS CYR; [INCR INDEXLOC]; TC Q. The
 * TC Q return executes the TC held in Q, leaving Q = 3. */
static NI void tagsub(void){
 ai_ca(0120);(void)ai_ts(0130);if(ai_ccs(020)==0u)ai_incr(0130);W[2]=3;
}
static NI uint8_t lmi_axt(void){
 /* AXT TC TAGSUB; CA POLISH; XSTORE INDEX INDEXLOC, TS X1. TAGSUB's
  */
 tagsub();ai_ca(0117);(void)ai_ts((unsigned)(W[0130]+046u));danzig_entry();
 return 0;
}
static NI uint8_t lmi_incr(void){
 /* INCR TC TAGSUB; CA POLISH; XAD2 INDEX INDEXLOC, ADS X1. */
 tagsub();ai_ca(0117);ai_ads((unsigned)(W[0130]+046u));danzig_entry();
 return 0;
}

/* GOTO +4: CA BANKSET; TS BBANK; CA POLISH; TS FBANK; MASK LOW10; AD 2K;
 * TS LOC; TCF INTPRET +3 (CA BBANK; TS BANKSET; MASK BIT15; TS INTBIT15;
 * TS EDOP; TCF NEWOPS). Only fixed-bank targets are compiled here. */
static NI void goto_fixed(void){
 ai_ca(0165);(void)ai_ts(6);ai_ca(0117);(void)ai_ts(4);ai_maskf(LOW10);ai_adf(TWO_K);(void)ai_ts(0164);
 ai_ca(6);(void)ai_ts(0165);ai_maskf(BIT15);(void)ai_ts(0115);(void)ai_ts(023);W[5]=IF_(06070,06037);
}
static NI void lmi_goto(void){
 /* GOTO CA POLISH; +1 MASK HIGH4; EXTEND BZF GOTOERS (fixed: nonzero). */
 ai_ca(0117);ai_maskf(HIGH4);goto_fixed();
}
static NI void lmi_rvq(void){
 /* RVQ INDEX FIXLOC, CA QPRET; TS POLISH; TCF GOTO +4. */
 ai_ca((unsigned)(W[0120]+052u));(void)ai_ts(0117);goto_fixed();
}
/* BRANCH (06723): CCS MPAC, MPAC +1, MPAC +2 in turn. A positive register
 * returns through TC Q (Q = 3), all zero through INDEX Q, TC 1 (Q 06741)
 * and a negative one through INDEX Q, TC 2 (Q 06743). 0 +,1 zero,2 -. */
/* BRANCH: -0 falls through CCS's fourth slot to the next word, as +0. */
/* SWITCHES: SWBIT/SWWORD in BUF +1/BUF, STATE word in Q, set/invert/clear
 * by POLISH bits 7-8 under INHINT, RELINT, then branch/skip/noop by bits
 * 5-6. The INHINT/RELINT pair leaves interrupts enabled. */
static NI uint8_t lmi_switch(void){
 static const agc_word bits[16]={040000,020000,010000,04000,02000,01000,0400,0200,0100,040,020,010,4,2,1,0};
 unsigned op,word;agc_word old;
 ai_caf(017);ai_mask(0117);ai_caf(bits[W[0]&017u]);(void)ai_ts(0131);ai_caf(0100);ai_mpf(W[0117]);(void)ai_ts(0130);
 io->interrupt_enabled=0;word=(unsigned)W[0];ai_ca(074u+word);W[2]=oc(W[0]);old=W[2];
 ai_caf(02000);ai_mpf(W[0117]);ai_maskf(014);op=(unsigned)W[0];
 if(op==0u){ai_ca(0131);W[0]=(agc_register)(W[0]|agc_extend(old));(void)ai_ts(074u+W[0130]);}
 else if(op==4u){ai_ca(0131);W[0]=(agc_register)(W[0]^agc_extend(old));(void)ai_ts(074u+W[0130]);}
 else if(op==010u){ai_cs(0131);ai_maskf(old);(void)ai_ts(074u+W[0130]);}
 io->interrupt_enabled=1;ai_caf(010000);ai_mpf(W[0117]);ai_maskf(014);op=(unsigned)W[0];
 if(op==014u){danzig_entry();return 0;}
 if(op==4u)return 1; /* +5 TCF SWBRANCH */
 W[0]=sx(op==0u?(agc_word)(~W[2]&077777u):W[2]);
 /* TEST MASK SWBIT; CCS A: positive skips, +0 branches. */
 ai_mask(0131);
 if(ai_ccs(0)==1u)return 1;
 ai_incr(0164);danzig_entry();return 0;
}
static NI void lmi_swbranch(agc_word target_word){
 /* SWBRANCH CA BANKSET; TS FBANK; INDEX LOC, CA 1; TS POLISH; TCF GOTO +1. */
 ai_ca(0165);(void)ai_ts(4);ai_caf(target_word);(void)ai_ts(0117);ai_maskf(HIGH4);goto_fixed();
}
static NI void lmi_noibnksw_right(void){
 /* NOIBNKSW CCS EDOP (positive) TCF OPJUMP (SETPD's return). */
 (void)ai_ccs(023);opjump();
}
/* ---- orders ---- */
/* SPDX-License-Identifier: MIT */
/* General compiled Luminary099 interpretive order bodies (INTERPRETER.agc),
 * written instruction for instruction with the agc_image_ops helpers. Each
 * function is one fixed source path family from its body entry (or address
 * formation step) to DANZIG or the next dispatcher step; branches inside
 * follow the original's runtime data tests (CCS, BZF, BZMF, overflow
 * skips) exactly as the original instructions take them. Nothing here
 * reads an order word, decodes, or fetches code: order and operand words
 * of the calling routine arrive as literal arguments from its compiled
 * caller, and fixed operands as literal data. Verified body by body against
 * the pinned yaAGC engine by tools/lmi_body_differential.py. */

/* Erasable assignments (Luminary099 ERASABLE_ASSIGNMENTS). */
enum {
 rA=0,rL=1,rQ=2,rEB=3,rFB=4,rBB=6,rZERO=7,rCYR=020,rSR=021,rCYL=022,rEDOP=023,
 rINTBIT15=0115,rADDRWD=0116,rPOLISH=0117,rFIXLOC=0120,rOVFIND=0121,rVBUF=0122,
 rBUF=0130,rMPTEMP=0135,rDOTINC=0136,rDOTRET=0137,rPOLYCNT=0140,rPOLYRET=0141,
 rMPAC=0154,rMODE=0163,rLOC=0164,rBANKSET=0165,rPUSHLOC=0166,
 rBUF2=0133,rDVSIGN=0136,rDVNORMCT=0137,rMAXDVSW=0140,rMATINC=0140,rTEM1=0141
};
/* Work-area tags relative to FIXLOC. */
enum {tX1=046,tX2=047,tS1=050,tLVSQUARE=042,tLV=044,tQPRET=052};
/* Fixed constants (FIXED_FIXED_CONSTANT_POOL and INTERPRETER). */
#define ZERO 0u
#define ONE 1u
#define TWO 2u
#define FOUR 4u
#define FIVE 5u
#define SIX 6u
#define SEVEN 7u
#define TEN 012u
#define LOW2 3u
#define LOW3 7u
#define LOW7 0177u
#define LOW8 0377u
#define LOW10 01777u
#define LOW11 03777u
#define BIT1 1u
#define BIT5 020u
#define BIT6 040u
#define BIT8 0200u
#define BIT10 01000u
#define BIT14 020000u
#define BIT15 040000u
#define HALF 020000u
#define QUARTER 010000u
#define POSMAX 037777u
#define NEGONE 077776u
#define NEG4 077773u
#define NEG12 077763u
#define B12T14 034000u
#define OCT1400 01400u
#define OCT23 023u
#define OCT176 0176u
#define TWO_K 02000u
#define DEC45 055u
#define ENDVAC_NEG 077722u
#define ENDERAS_NEG 074056u
#define OCT10_NEG 077767u
#define DLOADCOD 040014u
#define VLOADCOD BIT15
#define LVBUF 0122u

#define CA(a) ai_ca((a))
#define CAF(k) ai_caf((k))
#define CS(a) ai_cs((a))
#define CSF(k) ai_csf((k))
#define AD(a) ai_ad((a))
#define ADF(k) ai_adf((k))
#define MASK(a) ai_mask((a))
#define MASKF(k) ai_maskf((k))
#define TS(a) ai_ts((a))
#define XCH(a) ai_xch((a))
#define LXCH(a) ai_lxch((a))
#define ADS(a) ai_ads((a))
#define INCR(a) ai_incr((a))
#define DAS(a) ai_das((a))
#define DCA(a) ai_dca((a))
#define DCS(a) ai_dcs((a))
#define DXCH(a) ai_dxch((a))
#define MP(a) ai_mp((a))
#define MPF(k) ai_mpf((k))
#define CCS(a) ai_ccs((a))
#define AT(a) ((unsigned)(a))
/* TC to a subroutine: Q gets the address after the TC. */
#define TC(ret) ai_tc((ret))
/* TC Q (return): executes the TC held in Q, leaving Q = 3. */
#define RETQ() ai_tc(3u)

static NI void danzig(void){W[5]=IF_(06061,06030);}

/* Operand pair at C(ADDRWD)+off: live erasable, or the caller's fixed data. */
static NI void dca_op(const agc_word *k,unsigned off){
 if(k)ai_dcaf(k+off);else DCA(AT(W[rADDRWD]+off));
}
/* NO.WDS/REVCNT region 06241-06247 as INDEX MODE/A select it. */
static NI agc_word nowds(int index){
 static const agc_word t[7]={2,6,6,2,3,6,010020};
 /* (MODE is -1, +0 or +1 in every real program; a wild one reads the
  * table's first word rather than past it.) */
 return t[index>=-3&&index<=3?index+3:0];
}
static NI int signed_word(agc_word v){return (v&040000u)?-(int)(~v&037777u):(int)v;}

/* DIRADRES from INCR LOC (an address word was given) to the INDJUMP body. */
static NI void diradres_given(agc_word target){
 uint8_t c;
 INCR(rLOC);(void)TS(rADDRWD);ADF(ENDVAC_NEG);c=CCS(rA);
 if(c==2u||c==3u){
  /* NETZERO CA FIXLOC; ADS ADDRWD (or -0 straight to ADS). */
  if(c==2u)CA(rFIXLOC);
  ADS(rADDRWD);W[5]=target;return;
 }
 /* + : AD -ENDERAS; IERASTST EXTEND BZMF GEADDR. */
 if(c==0u)ADF(ENDERAS_NEG);
 if(ai_bzmf()){
  MASKF(LOW8);ADF(OCT1400);XCH(rADDRWD);(void)TS(rEB);W[5]=target;return;
 }
 MASKF(LOW10);ADF(LOW10);XCH(rADDRWD);AD(rINTBIT15);(void)TS(rFB);W[5]=target;
}
/* PUSHUP: operand from the push-down list. */
static NI void pushup(agc_word target){
 uint8_t c;
 CAF(OCT23);MASK(rCYR);ADF(OCT10_NEG);c=CCS(rA);
 if(c==0u)goto regup;
 /* +0 cannot occur (no -0 in the masked code). */
 ADF(NEG4);c=CCS(rA);
 if(c==0u){CSF(nowds(signed_word(agc_correct(W[rA]))));}
 else if(c==1u){CSF(nowds(0));}
 else if(c==3u){CSF(nowds(signed_word(W[rMODE])-2));} /* REVCNT = NO.WDS -2 */
 goto regup2;
regup:
 CSF(nowds(signed_word(W[rMODE])));
regup2:
 ADS(rPUSHLOC);(void)TS(rADDRWD);W[5]=target;
}
static NI void lmi_diradres(agc_word operand,agc_word target){
 /* DIRADRES INDEX LOC, CS 1; CCS A (negative: an address is given). */
 CSF(operand);(void)CCS(rA);diradres_given(target);
}
static NI void lmi_diradres_pushup(agc_word next,agc_word target){
 /* DIRADRES INDEX LOC, CS 1; CCS A (positive: next word is an order). */
 CSF(next);(void)CCS(rA);pushup(target);
}
static NI void lmi_address_pushup(agc_word next,agc_word target){
 /* ADDRESS MASK BIT1; CCS A (+0); DIRADRES as above. */
 MASKF(BIT1);(void)CCS(rA);lmi_diradres_pushup(next,target);
}

/* STORE (from its first instruction) through STARTSTO and TC Q. */
static NI void store(void){
 uint8_t c;unsigned at;
 CS(rADDRWD);ADF(DEC45);c=CCS(rA);
 if(c==0u){CA(rFIXLOC);}
 else if(c>=2u){
  /* - : CA OCT1400; -0 (address 45) enters at the XCH with A = +0. */
  if(c==2u)CAF(OCT1400);
  XCH(rADDRWD);(void)TS(rEB);MASKF(LOW8);
 }
 ADS(rADDRWD);
 DCA(rMPAC);at=W[rADDRWD];DXCH(at);c=CCS(rMODE);
 if(c==0u){CA(rMPAC+2);(void)TS(at+2u);}
 else if(c==2u){DCA(rMPAC+3);DXCH(at+2u);DCA(rMPAC+5);DXCH(at+4u);}
 RETQ();
}
/* DOSTORE (A holds the store code) through STORJUMP's TC and the store. */
static NI void dostore(void){
 static const agc_word next[8]={IF_(06061,06030),IF_(06061,06030),IF_(06061,06030),IF_(06460,06427),IF_(06144,06113),IF_(06463,06432),IF_(06466,06435),0};
 unsigned kind;
 (void)TS(rADDRWD);MASKF(LOW11);XCH(rADDRWD);MASKF(B12T14);MPF(BIT5);
 kind=(unsigned)W[rA]/2u;
 TC((agc_word)(IF_(06373u,06342u)+2u*kind));
 if(kind==1u){CS(AT(W[rFIXLOC]+tX1));ADS(rADDRWD);}
 else if(kind==2u){CS(AT(W[rFIXLOC]+tX2));ADS(rADDRWD);}
 store();
 if(kind==7u){
  /* STOTC: CAF CALLCODE; TS CYR; TCF 15BITADR. */
  CAF(032u);(void)TS(rCYR);W[5]=IF_(06252,06221);return;
 }
 W[5]=next[kind];
}
/* Whether the store `code` (at NEWOPS, before lmi_store_code) would write
 * past the end of the E-bank window (01777): the original then writes into
 * fixed memory and reads it back into A and L, which the image does not
 * hold. Mirrors DOSTORE's indexing and STORE's addressing and MODE count. */
static NI void lmi_store_code(agc_word code){
 /* NEWOPS INDEX LOC, CA 0; CCS A (positive) TCF DOSTORE. */
 CAF(code);(void)CCS(rA);dostore();
}
static NI void lmi_stadr(agc_word word){
 /* STADR CA BANKSET; TS FBANK; INCR LOC; INDEX LOC, CS 0; AD NEGONE. */
 CA(rBANKSET);(void)TS(rFB);INCR(rLOC);CSF(word);ADF(NEGONE);dostore();
}
static NI void lmi_dodload(void){CAF(DLOADCOD);(void)TS(rCYR);W[5]=IF_(06106,06055);}
static NI void lmi_dovload(void){CAF(VLOADCOD);(void)TS(rCYR);W[5]=IF_(06106,06055);}

/* NEWMODE (TS MODE) and VMODE. */
static NI void newmode(void){(void)TS(rMODE);danzig();}
static NI void vmode(void){CSF(ONE);newmode();}
static NI void lmi_dload_op(const agc_word *k){
 dca_op(k,0);DXCH(rMPAC);CAF(ZERO);(void)TS(rMPAC+2);newmode();
}
static NI void endvload4(const agc_word *k){dca_op(k,4);DXCH(rMPAC+5);vmode();}
static NI void endvload(const agc_word *k){dca_op(k,2);DXCH(rMPAC+3);endvload4(k);}
static NI void lmi_vload(const agc_word *k){
 dca_op(k,0);DXCH(rMPAC);endvload(k);
}
static NI void lmi_pdvl(const agc_word *k){
 uint8_t c;
 dca_op(k,0);DXCH(rMPAC);DXCH(W[rPUSHLOC]);CAF(nowds(signed_word(W[rMODE])));ADS(rPUSHLOC);c=CCS(rMODE);
 if(c==0u){
  /* TPDVL */
  dca_op(k,2);DXCH(rMPAC+3);CA(rMPAC+2);(void)TS(AT(W[rPUSHLOC]-1u));endvload4(k);return;
 }
 if(c==1u){endvload(k);return;}
 /* VPDVL */
 dca_op(k,2);DXCH(rMPAC+3);DXCH(AT(W[rPUSHLOC]-4u));dca_op(k,4);DXCH(rMPAC+5);DXCH(AT(W[rPUSHLOC]-2u));danzig();
}
static NI void lmi_pddl_op(const agc_word *k){
 uint8_t c;
 dca_op(k,0);DXCH(rMPAC);DXCH(W[rPUSHLOC]);CAF(nowds(signed_word(W[rMODE])));ADS(rPUSHLOC);c=CCS(rMODE);
 if(c==0u){(void)TS(rMODE);XCH(rMPAC+2);(void)TS(AT(W[rPUSHLOC]-1u));}
 else if(c==1u)(void)TS(rMPAC+2);
 else{(void)TS(rMODE);(void)TS(rMPAC+2);DXCH(rMPAC+3);DXCH(AT(W[rPUSHLOC]-4u));DXCH(rMPAC+5);DXCH(AT(W[rPUSHLOC]-2u));}
 danzig();
}
static NI uint8_t lmi_push(void){
 uint8_t c;
 DCA(rMPAC);DXCH(W[rPUSHLOC]);CAF(nowds(signed_word(W[rMODE])));ADS(rPUSHLOC);c=CCS(rMODE);
 if(c==0u){CA(rMPAC+2);(void)TS(AT(W[rPUSHLOC]-1u));}
 else if(c!=1u){DCA(rMPAC+3);DXCH(AT(W[rPUSHLOC]-4u));DCA(rMPAC+5);DXCH(AT(W[rPUSHLOC]-2u));}
 danzig();
 return 0;
}

/* ---- Shared basic subroutines (INTERPRETER.agc) ---------------------- */
#define ZL() ai_zl()
#define DOUBLE() ai_double()
#define COM() ai_com()
#define SQUARE() ai_square()
#define DDOUBL() ai_ddoubl()
#define AUGA() ai_aug(0u)
/* INDEX v; CAF k for k in FIXED_FIXED 04730-04777 (the bit table runs
 * BIT15 04735 to BIT1 04753), v taken as a one's-complement index. */
static NI agc_word fixed_indexed(unsigned k,agc_word v){
 static const agc_word t[]={01007,02,037777,037777,057777,040000,020000,010000,04000,02000,01000,0400,0200,0100,
#ifdef INTERP_COMANCHE
  /* Comanche055 has no OCT33 and OCT35 here (pool at 04774). */
  040,020,010,04,02,01,077777,0,05,07,013,015,017,030,032,045,046,050,055,060,062,0120,0140,0144,0310,0401};
#else
  040,020,010,04,02,01,077777,0,05,07,013,015,017,030,033,035,032,045,046,050,055,060,062,0120,0140,0144};
#endif
 int i=(int)k-(int)IF_(04730,04774)+signed_word((agc_word)(v&077777u));
 return i>=0&&i<(int)(sizeof t/sizeof t[0])?t[i]:0u;
}
#define BIT15_AT IF_(04735u,05001u)
#define BIT14_AT IF_(04736u,05002u)
/* INDEX r; CAF BIT14. */
static NI agc_word bit14_indexed(unsigned r){return fixed_indexed(BIT14_AT,W[r]);}

/* BRANCH 06723 (Q holds the caller's return): 0 positive (TC Q),
 * 1 zero (INDEX Q, TC 1), 2 negative (INDEX Q, TC 2). -0 is passed over. */
static NI uint8_t branch(void){
 unsigned i;uint8_t c;
 for(i=0;i<3u;++i){
  c=CCS(rMPAC+i);
  if(c==0u){RETQ();return 0;}
  if(c==2u){TC(IF_(06743,06712));return 2;}
 }
 TC(IF_(06741,06710));return 1;
}
/* OVERFLOW 07017 (A = +-1, L = component offset 0, 3 or 5). Returns 1 when
 * the propagated overflow reached OVFIND (SETOVF2). Ends with TC Q. */
static NI uint8_t overflow(void){
 unsigned l;
 CSF(W[rA]==1u?040000u:037777u);(void)TS(rBUF);AUGA();
 l=W[rL];ADS(AT(rMPAC+1u+l));
 if(!TS(rZERO))CAF(ZERO);
 AD(rBUF);ADS(AT(rMPAC+l));
 if(!TS(rZERO)){RETQ();return 0;}
 (void)TS(rOVFIND);RETQ();return 1;
}
static NI void overflwy(void){(void)TS(rL);CAF(3u);XCH(rL);(void)overflow();}
static NI void overflwz(void){(void)TS(rL);CAF(FIVE);XCH(rL);(void)overflow();}
/* ENDVXV: DAS MPAC; BZF DANZIG; else TC OVERFLOW, TCF DANZIG. */
static NI void endvxv(void){
 DAS(rMPAC);
 if(!ai_bzf()){TC(IF_(07004,06753));(void)overflow();}
 danzig();
}
static NI void lmi_dad(const agc_word *k){dca_op(k,0);endvxv();}
static NI void lmi_dsu(const agc_word *k){
 if(k)ai_dcsf(k);else DCS(W[rADDRWD]);
 endvxv();
}
static NI void lmi_bdsu(const agc_word *k){
 dca_op(k,0);DXCH(rMPAC);ai_dcom();endvxv();
}
/* VROUND/ROUNDSUB 07136-07153. `entry` 0 ROUNDSUB, 1 ROUNDSUB +1, 2 VROUND. */
static NI void roundsub(unsigned entry){
 if(entry==0u)CAF(ZERO);
 if(entry<2u)(void)TS(rMODE);
 XCH(rMPAC+2);DOUBLE();
 if(!TS(rL)){RETQ();return;}
 AD(rMPAC+1);
 if(!TS(rMPAC+1)){RETQ();return;}
 AD(rMPAC);
 if(!TS(rMPAC)){RETQ();return;}
 (void)TS(rOVFIND);RETQ();
}
/* DMPSUB 07107 with the operand pair at C(ADDRWD) (or fixed `k`). */
static NI void dmpsub(const agc_word *k){
 if(k)CAF(k[1]);else CA(AT(W[rADDRWD]+1u));
 (void)TS(rMPAC+2);CAF(ZERO);XCH(rMPAC+1);(void)TS(rMPTEMP);
 MP(rMPAC+2);XCH(rMPAC+2);MP(rMPAC);DAS(rMPAC+1);
 if(k)CAF(k[0]);else CA(W[rADDRWD]);
 XCH(rMPTEMP);
 MP(rMPTEMP);DAS(rMPAC+1);XCH(rMPAC);MP(rMPTEMP);DAS(rMPAC);RETQ();
}
static NI void lmi_dmp(const agc_word *k){TC(IF_(07573,07542));dmpsub(k);danzig();}
static NI void lmi_dmpr(const agc_word *k){
 TC(IF_(07575,07544));dmpsub(k);TC(IF_(07576,07545));roundsub(1);danzig();
}
/* TPAGREE 07257 (Q = return); returns via TC L. */
static NI void tpagree(void){
 uint8_t c;
 LXCH(rQ);TC(IF_(07261,07230));c=branch();
 if(c==1u){(void)TS(rMPAC+2);goto argzero2;}
 if(c==2u)CSF(POSMAX);else CAF(POSMAX);
 (void)TS(rQ);AUGA();AD(rMPAC+2);
 if(!TS(rMPAC+2))CAF(ZERO);
 AD(rQ);AD(rMPAC+1);
 if(!TS(rMPAC+1))CAF(ZERO);
 AD(rQ);AD(rMPAC);
argzero2:
 if(!TS(rMPAC))(void)TS(rMPAC+1);
 /* TC L: executes the return TC held in L, leaving Q = 2. */
 ai_tc(2u);
}

/* ---- Shifts (bank 0) -------------------------------------------------- */
static NI void lmi_shortv(void);
static NI void mpacshr3(void){
 /* MPACSHR +3 TS MPAC+2; CA MPTEMP; MP MPAC; DXCH MPAC; CA MPTEMP; MP L;
  * DAS MPAC+1; TCF DANZIG. */
 (void)TS(rMPAC+2);CA(rMPTEMP);MP(rMPAC);DXCH(rMPAC);CA(rMPTEMP);MP(rL);DAS(rMPAC+1);danzig();
}
static NI void mpacshr(void){CA(rMPTEMP);MP(rMPAC+2);mpacshr3();}
/* VSHR2 (A = value to double) to TC Q. */
static NI void vshr2(void){
 DOUBLE();
 if(TS(rMPAC+2))ADS(rMPAC+1);
 CAF(ZERO);(void)TS(rMPAC+2);XCH(rMPAC);MP(rMPTEMP);DAS(rMPAC);RETQ();
}
static NI void mpacsrnd(void){
 CA(rMPAC+2);MP(rMPTEMP);XCH(rMPAC+1);MP(rMPTEMP);XCH(rMPAC+1);AD(rL);vshr2();
}
static NI void vshrrnd(void){CA(rMPTEMP);MP(rMPAC+1);(void)TS(rMPAC+1);XCH(rL);vshr2();}
/* RIGHTR TC MPACSRND; TCF NEWMODE (C(A) = 0). */
static NI void rightr(void){TC(02032);mpacsrnd();newmode();}
static NI void vrotatex(void){DXCH(rMPAC);DXCH(rMPAC+5);DXCH(rMPAC+3);DXCH(rMPAC);danzig();}
/* VSSR (MPTEMP = count - 1); `indexed` 0 enters at VSSR +1, past the
 * INDEX MPTEMP (CCS -0 of CYR in SHORTV lands there). */
static NI void vssr(uint8_t indexed){
 CAF(indexed?bit14_indexed(rMPTEMP):BIT14);(void)TS(rMPTEMP);
 TC(02133);vshrrnd();DXCH(rMPAC);DXCH(rMPAC+3);DXCH(rMPAC);
 TC(02137);vshrrnd();DXCH(rMPAC);DXCH(rMPAC+5);DXCH(rMPAC);
 TC(02143);vshrrnd();vrotatex();
}
/* TSSL +2 loop (MPTEMP = count - 1), then the CYR round test. */
static NI void tssl2(void){
 uint8_t c;
 for(;;){
  DCA(rMPAC+1);DAS(rMPAC+1);AD(rMPAC);AD(rMPAC);
  if(TS(rMPAC))(void)TS(rOVFIND);
  if(CCS(rMPTEMP)!=0u)break;
  (void)TS(rMPTEMP);
 }
 c=CCS(rCYR);
 if(c==0u){TC(02117);roundsub(0);}
 /* -0 in CYR (GENSCL's MP leaves -0 for an unrounded count) passes the
  * three exits and runs on into SHORTV, as the original does. */
 if(c==3u){lmi_shortv();return;}
 danzig();
}
/* VSSL loop from VSSL (MPTEMP = count - 1). */
static NI void vssl(void){
 for(;;){
  DCA(rMPAC);DAS(rMPAC);if(!ai_bzf()){TC(02153);(void)overflow();}
  DCA(rMPAC+3);DAS(rMPAC+3);if(!ai_bzf()){TC(02161);overflwy();}
  DCA(rMPAC+5);DAS(rMPAC+5);if(!ai_bzf()){TC(02167);overflwz();}
  if(CCS(rMPTEMP)!=0u)break;
  (void)TS(rMPTEMP);
 }
 danzig();
}
static NI void lmi_short_dispatch(void){
 /* OPJUMP2 CCS CYR (+); OPJUMP3 TS FBANK; CCS CYR (-); CCS MODE: SHORTT
  * for a scalar, SHORTV for a vector. */
 uint8_t c;
 (void)CCS(rCYR);(void)TS(rFB);(void)CCS(rCYR);c=CCS(rMODE);
 W[5]=c==2u?02121:02017;
}
static NI void lmi_shortt(void){
 uint8_t c;
 CAF(6u);MASK(rCYR);(void)TS(rSR);c=CCS(rCYR);
 if(c==0u){
  /* TSSL CA SR; TS MPTEMP. */
  CA(rSR);(void)TS(rMPTEMP);tssl2();return;
 }
 /* TSSR INDEX SR; CAF BIT14 (-0 enters past the INDEX); TS MPTEMP; CCS CYR. */
 CAF(c==3u?BIT14:bit14_indexed(rSR));(void)TS(rMPTEMP);
 switch(CCS(rCYR)){
 case 0:rightr();return;
 case 1:newmode();return;                       /* TCF NEWMODE, C(A) = 0 */
 case 2:mpacshr();return;
 default:MP(rMPAC+2);mpacshr3();return;         /* MPACSHR +1 */
 }
}
static NI void lmi_shortv(void){
 uint8_t c;
 CAF(LOW3);MASK(rCYR);(void)TS(rMPTEMP);c=CCS(rCYR);
 if(c==0u)vssl();else vssr(c!=3u);
}
/* SETROUND 02272 (A = bits to double; L = 0) to TC Q. */
static NI void setround(void){
 DOUBLE();if(!TS(rMPAC+2))CAF(ZERO);XCH(rL);RETQ();
}
/* DOSSHFT and the GENSCR right shift (MPTEMP = count - 1). */
static NI void dosshft(void){
 CAF(bit14_indexed(rMPTEMP));(void)TS(rMPTEMP);CAF(BIT10);MASK(rADDRWD);
 if(CCS(rA)==0u)rightr();else mpacshr();
}
/* CAF BIT10; MASK ADDRWD; CCS A; TC ROUNDSUB; TCF DANZIG (GENSCR +12, or
 * GENSHIFT +3 for a zero count; `ret` is the address after the TC). */
static NI void genscr_tail(agc_word ret){
 CAF(BIT10);MASK(rADDRWD);
 if(CCS(rA)==0u){TC(ret);roundsub(0);}
 danzig();
}
/* GENSCR +1 (A = remaining count): AD NEG12; BZMF DOSSHFT; GENSCR +4 AD
 * NEGONE ... shifting 14 places a pass. `at4` enters at GENSCR +4. */
static NI void genscr1(uint8_t at4){
 for(;;){
  if(!at4){
   ADF(NEG12);
   if(ai_bzmf()){dosshft();return;}
  }
  at4=0;ADF(NEGONE);
  /* GENSCR +4 */
  (void)TS(rMPTEMP);CAF(ZERO);XCH(rMPAC);XCH(rMPAC+1);(void)TS(rMPAC+2);
  switch(CCS(rMPTEMP)){
  case 0:(void)TS(rMPTEMP);TC(02314);break;     /* TS MPTEMP; TC GENSCR +1 */
  case 1:TC(02314);break;                      /* TC GENSCR +1 */
  default:genscr_tail(02321);return;
  }
 }
}
static NI void genscr(void){CA(rMPTEMP);genscr1(0);}
static NI void vright2(void){
 uint8_t c;
 for(;;){
  ADF(NEG12);
  if(ai_bzmf()){vssr(1);return;}
  ADF(NEGONE);(void)TS(rMPTEMP);CAF(ZERO);(void)TS(rL);
  XCH(rMPAC);XCH(rMPAC+1);TC(02254);setround();DAS(rMPAC);
  XCH(rMPAC+3);XCH(rMPAC+4);TC(02260);setround();DAS(rMPAC+3);
  XCH(rMPAC+5);XCH(rMPAC+6);TC(02264);setround();DAS(rMPAC+5);
  /* CCS MPTEMP: + TS MPTEMP; +0 TCF VRIGHT2; -0 (a count of exactly 14
   * more, as AD NEGONE makes it) TCF DANZIG. */
  c=CCS(rMPTEMP);
  if(c==3u){danzig();return;}
  if(c==0u)(void)TS(rMPTEMP);
 }
}
static NI void right(void){
 if(CCS(rMODE)!=2u){genscr();return;}
 CA(rMPTEMP);vright2();
}
static NI void left(void){
 if(CCS(rMODE)==2u){vssl();return;}
 /* GENSCL CS ADDRWD; MP BIT6; TS CYR; TCF TSSL +2. */
 CS(rADDRWD);MPF(BIT6);(void)TS(rCYR);tssl2();
}
static NI void lmi_genshift(void){
 /* GENSHIFT (A = LOW7): MASK ADDRWD; CCS A. */
 MASK(rADDRWD);
 if(CCS(rA)==1u){genscr_tail(02223);return;}
 /* GENSHFT2 TS MPTEMP; CAF BIT8; MP ADDRWD; MASK LOW2; INDEX A; TCF +1. */
 (void)TS(rMPTEMP);CAF(BIT8);MP(rADDRWD);MASKF(LOW2);
 switch(W[rA]){
 case 0:CS(rMPTEMP);ADF(OCT176);(void)TS(rMPTEMP);right();return;   /* RIGHT- */
 case 1:left();return;                                              /* LEFT */
 case 2:CSF(OCT176);AD(rMPTEMP);COM();(void)TS(rMPTEMP);left();return; /* LEFT- */
 default:right();return;                                            /* RIGHT */
 }
}
/* GSHIFT CAF LOW7; TS FBANK; TCF GENSHIFT. */
static NI void lmi_gshift(const agc_word *k){(void)k;CAF(LOW7);(void)TS(rFB);lmi_genshift();}
/* TSLC CAF ZERO; TS FBANK; TSLC2 ... ENDTSLC CS MPTEMP; STORE1. */
static NI void lmi_tslc(const agc_word *k){(void)k;
 uint8_t c;
 CAF(ZERO);(void)TS(rFB);(void)TS(rMPTEMP);TC(02174);c=branch();
 if(c!=1u){
  TC(02177);tpagree();CA(rMPAC);
  for(;;){
   DOUBLE();
   if(ai_ovsk())break;
   INCR(rMPTEMP);DCA(rMPAC+1);DAS(rMPAC+1);AD(rMPAC);ADS(rMPAC);
  }
 }
 CS(rMPTEMP);(void)TS(W[rADDRWD]);danzig();
}

/* ---- Unary scalar/vector orders (bank 0 and 3) ------------------------ */
/* COMP 07670 and DCOMP. */
static NI void comp(void){
 DCS(rMPAC);DXCH(rMPAC);
 if(CCS(rMODE)<2u){CS(rMPAC+2);(void)TS(rMPAC+2);danzig();return;}
 DCS(rMPAC+3);DXCH(rMPAC+3);DCS(rMPAC+5);DXCH(rMPAC+5);danzig();
}
static NI uint8_t lmi_comp(void){comp();return 0;}
static NI void lmi_sign(const agc_word *k){
 uint8_t c=k?ai_ccsf(k[0]):CCS(W[rADDRWD]);
 if(c==0u){danzig();return;}
 if(c==2u){comp();return;}
 c=k?ai_ccsf(k[1]):CCS(AT(W[rADDRWD]+1u));
 if(c==2u)comp();else danzig();
}
static NI void lmi_abs(void){TC(03227);if(branch()==2u)comp();else danzig();}
/* DSQSUB 03300 to TC Q. */
static NI void dsqsub(void){
 CA(rMPAC+1);SQUARE();(void)TS(rMPAC+2);CAF(ZERO);XCH(rMPAC+1);MP(rMPAC);DDOUBL();DAS(rMPAC+1);
 XCH(rMPAC);SQUARE();DAS(rMPAC);RETQ();
}
static NI uint8_t lmi_dsq(void){TC(03175);dsqsub();danzig();return 0;}
/* ENDDOT 07205 to TC DOTRET (Q = DOTRET +1). */
static NI void enddot(void){
 DXCH(rBUF+1);DAS(rMPAC+1);AD(rMPAC);AD(rBUF);
 if(TS(rMPAC)){TC(IF_(07214,07163));(void)overflow();}
 ai_tc(rDOTRET+1u);
}
/* VSQSUB 03317 (QXCH DOTRET ... ENDDOT). */
static NI void vsqsub(void){
 ai_qxch(rDOTRET);
 TC(03322);dsqsub();DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rBUF);CA(rMPAC+2);(void)TS(rBUF+2);
 TC(03330);dsqsub();DXCH(rMPAC+1);DAS(rBUF+1);AD(rMPAC);AD(rBUF);
 if(TS(rBUF))(void)TS(rOVFIND);
 DXCH(rMPAC+5);DXCH(rMPAC);TC(03342);dsqsub();enddot();
}
/* DMODE CAF ZERO; TCF NEWMODE. */
static NI void dmode(void){CAF(ZERO);newmode();}
static NI uint8_t lmi_vsq(void){TC(03246);vsqsub();dmode();return 0;}
/* DOTSUB 07156 (DOTINC set) with the vector operand at C(ADDRWD) or `k`. */
static NI void dotsub(const agc_word *k){
 unsigned base=W[rADDRWD];
 ai_qxch(rDOTRET);
 TC(IF_(07161,07130));dmpsub(k);DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rBUF);CA(rMPAC+2);(void)TS(rBUF+2);
 CA(rDOTINC);ADS(rADDRWD);
 TC(IF_(07171,07140));dmpsub(k?k+(W[rADDRWD]-base):0);DXCH(rMPAC+1);DAS(rBUF+1);AD(rMPAC);AD(rBUF);
 if(TS(rBUF))(void)TS(rOVFIND);
 DXCH(rMPAC+5);DXCH(rMPAC);CA(rDOTINC);ADS(rADDRWD);
 TC(IF_(07205,07154));dmpsub(k?k+(W[rADDRWD]-base):0);enddot();
}
static NI void lmi_dot(const agc_word *k){
 /* DOT TC PREDOT (CAF TWO; TS DOTINC; DOTSUB); DMODE. */
 TC(IF_(07332,07301));CAF(TWO);(void)TS(rDOTINC);dotsub(k);dmode();
}

/* ---- Division (bank 0) ------------------------------------------------ */
#define MP_HALF() MPF(HALF)
/* FINALDV +3 (A = minor quotient): DVSIGN -0 complements MPAC; TC Q with
 * C(A) = 0. */
static NI void finaldv3(void){
 if(TS(rMPAC+1)){RETQ();return;}
 if(CCS(rDVSIGN)!=3u){RETQ();return;}
 DCS(rMPAC);DXCH(rMPAC);CAF(ZERO);RETQ();
}
static NI void finaldv(void){ZL();ai_dv(rBUF);finaldv3();}
static NI void endmaxdv(void){AD(rBUF);finaldv();}
/* GENDDV, or GENDDV +1 with the dividend already in A,L. */
static NI void genddv(uint8_t plus1){
 if(!plus1)DXCH(rMPAC);
 ai_dv(rBUF);DXCH(rMPAC);CS(rMPAC);MP(rBUF+1);AD(rMPAC+1);
 if(ai_ovsk()){ai_su(rBUF);INCR(rMPAC);}
 else if(ai_bzmf()){
  /* -UP: BZF FINALDV +3; DIM MPAC; ENDMAXDV. */
  if(ai_bzf()){finaldv3();return;}
  ai_dim(rMPAC);endmaxdv();return;
 }
 /* +DOWN SU BUF; BZF +3; BZMF ENDMAXDV; +3 INCR MPAC; TCF FINALDV. */
 ai_su(rBUF);
 if(!ai_bzf()&&ai_bzmf()){endmaxdv();return;}
 INCR(rMPAC);finaldv();
}
static NI void maxdv(void){
 CS(rMPAC);AD(rBUF);
 if(!ai_bzf()){genddv(0);return;}
 CAF(POSMAX);(void)TS(rMPAC);CS(rBUF+1);AD(rMPAC+1);endmaxdv();
}
/* MAXTEST CCS MAXDVSW (-1, or +0 when the majors may be equal). */
static NI void maxtest(void){
 switch(CCS(rMAXDVSW)){
 case 1:maxdv();return;
 case 3:genddv(1);return;
 default:genddv(0);return;   /* - (MAXDVSW is never positive) */
 }
}
/* The dividend normalization chain entered at `e` (02540 is the UNIT
 * prologue -21D; 02547-02563 DDOUBLs; 02564 DXCH MPAC; 02565 MAXTEST), as
 * INDEX DVNORMCT; TC MAXTEST and INDEX MPTEMP; TCF MAXTEST -1 select it. */
static NI void ddv_chain(unsigned e){
 if(e==02540u){LXCH(rSR);MP_HALF();XCH(rL);AD(rSR);XCH(rL);genddv(1);return;}
 if(e<=02564u){for(;e<02564u;++e)DDOUBL();DXCH(rMPAC);}
 maxtest();
}
static NI unsigned chain_at(unsigned base,agc_word v){return (unsigned)((int)base+signed_word((agc_word)(v&077777u)));}
/* DVOVF 02405: +-POSMAX through FINALDV +3, OVFIND set, TC DANZIG. */
static NI void dvovf(void){
 CAF(POSMAX);(void)TS(rMPAC);TC(02410);finaldv3();CAF(ONE);(void)TS(rOVFIND);TC(02413);danzig();
}
/* DDV/BDDV 02353 (dividend MPAC, divisor BUF) to DANZIG. */
static NI void ddv_bddv(void){
 uint8_t c;
 CSF(ONE);(void)TS(rDVSIGN);(void)TS(rDVNORMCT);(void)TS(rMAXDVSW);
 c=CCS(rBUF);
 if(c==0u)goto bufpos;
 if(c==2u)goto bufneg;
bufzero:
 (void)TS(rMPAC+2);TC(02365);tpagree();
 c=CCS(rMPAC);
 if(c==0u)goto ovfplus;
 if(c==2u)goto ovfminus;
 XCH(rBUF+1);XCH(rBUF);XCH(rMPAC+1);XCH(rMPAC);
 c=CCS(rBUF);
 if(c==0u)goto bufplus;
 if(c==2u)goto bufminus;
 CS(rMPAC);
sgndvovf:
 if(!ai_bzmf())INCR(rDVSIGN);
 dvovf();return;
ovfminus:
 INCR(rDVSIGN);
ovfplus:
 CS(rBUF+1);goto sgndvovf;
bufminus:
 DCS(rBUF);DXCH(rBUF);INCR(rDVSIGN);
bufplus:
 c=CCS(rMPAC);
 if(c==1u||c==3u){
  c=CCS(rMPAC+1);
  if(c==1u||c==3u){danzig();return;}  /* zero dividend */
 }
 if(c==2u){DCS(rMPAC);DXCH(rMPAC);INCR(rDVSIGN);}
 /* MPAC+ CS MPAC; AD NEGONE; AD BUF; CCS A. */
 CS(rMPAC);ADF(NEGONE);AD(rBUF);c=CCS(rA);
 if(c==0u)goto dvnorm;
 if(c==1u)AD(rL);              /* -1/2+2 (OCT 60001) runs as AD L */
 CAF(HALF);DOUBLE();AD(rMPAC+1);if(!TS(rMPAC+1))CAF(ZERO);ADF(POSMAX);ADS(rMPAC);
 CAF(HALF);DOUBLE();AD(rBUF+1);if(!TS(rBUF+1))CAF(ZERO);ADF(POSMAX);ADS(rBUF);
 CS(rMPAC);AD(rBUF);c=CCS(rA);
 if(c==0u)goto dvnorm;
 if(c!=3u){dvovf();return;}   /* (+0 cannot occur: the majors differ) */
 (void)TS(rMAXDVSW);CS(rMPAC+1);AD(rBUF+1);
 if(ai_bzmf()){dvovf();return;}
dvnorm:
 for(;;){
  CA(rBUF);DOUBLE();
  if(ai_ovsk())break;
  ai_aug(rDVNORMCT);DCA(rBUF);DAS(rBUF);
 }
 DXCH(rMPAC);TC(02514);ddv_chain(chain_at(02565u,W[rDVNORMCT]));
 (void)TS(rMPAC+2);danzig();return;
bufpos:
 /* BUFPOS CCS A (A = |BUF| - 1). */
 if(CCS(rA)==0u)goto bufplus;
 CS(rBUF+1);
 if(ai_bzmf())goto bufplus;
 CAF(HALF);
bufpos6:
 DOUBLE();ADS(rBUF+1);CAF(ZERO);(void)TS(rBUF);goto bufzero;
bufneg:
 if(CCS(rA)==0u)goto bufminus;
 CA(rBUF+1);
 if(ai_bzmf())goto bufminus;
 CSF(HALF);goto bufpos6;
}
/* DDV/BDDV prologue tail BDDV +4: DXCH BUF; CAF ZERO; TS FBANK. */
static NI void bddv4(void){DXCH(rBUF);CAF(ZERO);(void)TS(rFB);ddv_bddv();}
static NI void lmi_ddv(const agc_word *k){dca_op(k,0);bddv4();}
static NI void lmi_bddv(const agc_word *k){dca_op(k,0);DXCH(rMPAC);bddv4();}

/* ---- Square root and length (bank 0) ---------------------------------- */
#define SLOPEHI 022650u
#define BIASHI 06552u
#define SLOPELO 032506u
#define BIASLO 04604u
#define NEGHALF2 060001u
/* SQRTSUB 03343 to TC Q; 1 when it ends at SQRTABRT's TC POODOO1 (1302). */
static NI uint8_t sqrtsub(void){
 uint8_t c;
 CAF(ZERO);(void)TS(rMPTEMP);
 c=CCS(rMPAC);
 if(c==0u)goto smpac;
 if(c==2u)goto sqrtneg;
 XCH(rMPAC+2);XCH(rMPAC+1);(void)TS(rMPAC);CAF(SEVEN);(void)TS(rMPTEMP);
 c=CCS(rMPAC);
 if(c==0u)goto smpac;
 if(c==2u)goto zeroans;
 XCH(rMPAC+1);(void)TS(rMPAC);CAF(SEVEN);ADS(rMPTEMP);
 c=CCS(rMPAC);
 if(c==0u)goto smpac;
 if(c==1u){RETQ();return 0;}
 if(c==2u)goto zeroans;
 goto fixroot;
sqrtneg:
 c=CCS(rA);
 if(c==0u)goto abrt;
 if(c==2u)goto zeroans;
 if(c==3u)goto fixroot;
 c=CCS(rMPAC+1);
 if(c==0u)goto zeroans;
 if(c==2u)goto abrt;
 goto fixroot;
zeroans:
 CAF(ZERO);
fixroot:
 if(!TS(rMPAC))(void)TS(rMPAC+1);
 RETQ();return 0;
abrt:
 /* SQRTABRT: Luminary DXCH LOC, TC POODOO1; Comanche TC POODOO. */
#ifdef INTERP_COMANCHE
 TC(03403);W[5]=05721;return 1;
#else
 DXCH(rLOC);TC(03404);W[5]=05726;return 1;
#endif
smpac:
 ADF(NEGHALF2);
 if(ai_bzmf())goto srtest;
 DXCH(rMPAC);LXCH(rSR);MP_HALF();DXCH(rMPAC);XCH(rSR);ADS(rMPAC+1);
arghi:
 CAF(SLOPEHI);MP(rMPAC);ADF(BIASHI);
arghi4:
 if(!TS(rBUF))CA(rMPAC);
 ZL();ai_dv(rBUF);MP_HALF();ADS(rBUF);MP_HALF();DXCH(rMPAC);ai_dv(rBUF);
 if(!TS(rBUF+1))CAF(ZERO);
 XCH(rL);ai_dv(rBUF);
 if(!TS(rL))CA(rBUF+1);
 DAS(rMPAC);
 if(ai_bzf()){RETQ();return 0;}
 CAF(POSMAX);goto fixroot;
srtest:
 ADF(QUARTER);
 if(ai_bzmf())goto sqrtnorm;
 DXCH(rMPAC);LXCH(rSR);MP_HALF();DXCH(rMPAC);XCH(rSR);ADS(rMPAC+1);
arglo:
 CAF(SLOPELO);MP(rMPAC);ADF(BIASLO);goto arghi4;
sqrtnm2:
 DCA(rMPAC+1);DAS(rMPAC+1);AD(rMPAC);ADS(rMPAC);
sqrtnorm:
 INCR(rMPTEMP);DCA(rMPAC+1);DAS(rMPAC+1);AD(rMPAC);ADS(rMPAC);DOUBLE();
 /* TS CYL; NORMTEST CCS CYL; CCS CYL (each CCS cycles CYL again). */
 c=TS(rCYL)?0u:CCS(rCYL);
 if(c==0u){
  c=CCS(rCYL);
  if(c==0u)goto sqrtnm2;
  if(c==1u)goto arghi;
  goto arglo;                  /* (-0 would run on into COSINE) */
 }
 if(c==1u)goto sqrtnm2;
 if(c==2u)goto arghi;
 goto arglo;
}
/* SQRT 03207 to DANZIG, or 1 at POODOO1 (alarm 1302). */
static NI uint8_t sqrt_body(void){
 uint8_t c;
 TC(03210);if(sqrtsub())return 1;
 c=CCS(rMPTEMP);
 if(c==1u){danzig();return 0;}
 if(c!=3u){
  ADF(NEG12);
  if(!ai_bzmf()){ZL();LXCH(rADDRWD);genscr1(1);return 0;}
 }
 /* SQRTSHFT INDEX MPTEMP; CAF BIT15; TS MPTEMP; CAF ZERO; MPACSHR +3. */
 CAF(fixed_indexed(BIT15_AT,W[rMPTEMP]));(void)TS(rMPTEMP);CAF(ZERO);mpacshr3();return 0;
}
static NI uint8_t lmi_sqrt(void){return sqrt_body();}
/* ABVALABS 03176: ABS on a scalar, ABVAL (length, LVSQUARE kept) on a vector. */
static NI uint8_t lmi_abvalabs(void){
 if(CCS(rMODE)<2u){lmi_abs();return 0;}
 TC(03202);vsqsub();LXCH(rMODE);DCA(rMPAC);DXCH(AT(W[rFIXLOC]+tLVSQUARE));
 return sqrt_body();
}

/* ---- UNIT (bank 0) ---------------------------------------------------- */
/* ALSIGNAG 07544: sign agreement of A,L, to TC Q. */
static NI void alsignag(void){
 switch(CCS(rA)){
 case 0:XCH(rL);ADF(HALF);ADF(HALF);if(TS(rA))INCR(rL);XCH(rL);RETQ();return;
 case 2:XCH(rL);ADF(BIT15);ADF(NEGONE);if(TS(rA))INCR(rL);XCH(rL);COM();RETQ();return;
 default:RETQ();return;
 }
}
/* VECAGREE 03010 (Q = return): ends with TC A, leaving Q = 1. */
static NI void vecagree(void){
 XCH(rQ);
 DXCH(rMPAC);TC(03013);alsignag();DXCH(rMPAC);
 DXCH(rMPAC+3);TC(03016);alsignag();DXCH(rMPAC+3);
 DXCH(rMPAC+5);TC(03021);alsignag();DXCH(rMPAC+5);
 ai_tc(1u);
}
/* V/SCDV 02750 (Q = return): one component in MPAC,+1 over BUF; 1 when it
 * ends at DVOVF's TC DANZIG. */
static NI uint8_t vscdv(void){
 uint8_t c;
 CA(rVBUF+5);(void)TS(rDVSIGN);
 c=CCS(rMPAC);
 if(c==1u||c==3u){
  c=CCS(rMPAC+1);
  if(c==1u||c==3u){RETQ();return 0;}    /* zero component: TC Q */
 }
 if(c==2u){DCS(rMPAC);DXCH(rMPAC);INCR(rDVSIGN);}
 /* /MPAC+ CS ONE; TS MAXDVSW; CS MPAC; AD BUF2; CCS A. */
 CSF(ONE);(void)TS(rMAXDVSW);CS(rMPAC);AD(rBUF2);c=CCS(rA);
 if(c==2u){dvovf();return 1;}
 if(c!=0u){
  (void)TS(rMAXDVSW);CS(rMPAC+1);AD(rBUF2+1);
  if(ai_bzmf()){dvovf();return 1;}
 }
 /* DDVCALL DXCH MPAC; INDEX DVNORMCT; TCF MAXTEST. */
 DXCH(rMPAC);ddv_chain(chain_at(02565u,W[rDVNORMCT]));
 return 0;
}
/* V/SC 07624 (Comanche 07573), operand at ADDRWD (k when fixed), through
 * V/SC2 02654 and VROTATEX to DANZIG (or DVOVF's). */
/* MPACVBUF 07532 to TC Q. */
static NI void mpacvbuf(void){
 DCA(rMPAC);DXCH(rVBUF);DCA(rMPAC+3);DXCH(rVBUF+2);DCA(rMPAC+5);DXCH(rVBUF+4);RETQ();
}
/* UNITDV 03151 (divisor in BUF, MPTEMP the pre-divide count) to TC Q. */
static NI void unitdv(void){
 uint8_t c=CCS(rMPAC);
 if(c==1u||c==3u){
  c=CCS(rMPAC+1);
  if(c==1u||c==3u){RETQ();return;}
 }
 if(c==2u){CSF(ZERO);(void)TS(rDVSIGN);DCS(rMPAC);}
 else{(void)TS(rDVSIGN);DXCH(rMPAC);}
 ddv_chain(chain_at(02564u,W[rMPTEMP]));
}
/* UNIT 03023 to DANZIG (or DVOVF's TC DANZIG); 1 at POODOO1. */
static NI uint8_t lmi_unit(void){
 uint8_t c;
 TC(03024);vecagree();TC(03025);mpacvbuf();
 CAF(ZERO);XCH(rOVFIND);(void)TS(rTEM1);TC(03031);vsqsub();CA(rTEM1);XCH(rOVFIND);
 if(!ai_bzf()){dvovf();return 0;}
 DCA(rMPAC);DXCH(AT(W[rFIXLOC]+tLVSQUARE));
 TC(03043);if(sqrtsub())return 1;
 c=CCS(rMPAC);
 if(c!=0u){
  /* +0 TS L; - INDEX FIXLOC; -0 (unindexed) DXCH LV; TCF DVOVF. */
  if(c==1u)(void)TS(rL);
  DXCH(c==3u?AT(tLV):AT(W[rFIXLOC]+tLV));dvovf();return 0;
 }
 CSF(016u);AD(rMPTEMP);c=CCS(rA);
 if(c==0u)COM();
 if(c<2u){
  /* SMALL TS MPTEMP; shift VBUF left 14; INDEX A (-MPTEMP); CAF BIT14; MP MPAC. */
  (void)TS(rMPTEMP);CAF(ZERO);
  XCH(rVBUF+1);XCH(rVBUF);XCH(rVBUF+3);XCH(rVBUF+2);XCH(rVBUF+5);XCH(rVBUF+4);
  CS(rMPTEMP);CAF(fixed_indexed(BIT14_AT,W[rA]));MP(rMPAC);
  goto small2;
 }
 if(c==3u){CSF(015u);(void)TS(rMPTEMP);CA(rMPAC);goto small2;}
 /* LARGE CCS MPTEMP. */
 c=CCS(rMPTEMP);
 if(c==0u){
  /* LARGE3 COM; TS MPTEMP; COM; INDEX A; CAF BIT14 ... unnormalize. */
  COM();(void)TS(rMPTEMP);COM();CAF(fixed_indexed(BIT14_AT,W[rA]));(void)TS(rBUF);
  MP(rMPAC+1);XCH(rBUF);MP(rMPAC);XCH(rL);AD(rBUF);XCH(rL);goto large2;
 }
 if(c==1u)CSF(024u);           /* CS SRDDV */
 if(c<3u)(void)TS(rMPTEMP);
 DCA(rMPAC);goto large2;
small2:
 (void)TS(rL);CAF(ZERO);
large2:
 DXCH(AT(W[rFIXLOC]+tLV));CSF(ONE);(void)TS(rMAXDVSW);
 DXCH(rVBUF);DXCH(rMPAC);DXCH(rBUF);TC(03122);unitdv();
 DXCH(rVBUF+2);DXCH(rMPAC);DXCH(rMPAC+3);TC(03126);unitdv();
 DXCH(rVBUF+4);DXCH(rMPAC);DXCH(rMPAC+5);TC(03132);unitdv();
 vrotatex();return 0;
}

/* ---- Vector times scalar and cross product (bank 3) ------------------- */
static const agc_word *kat(const agc_word *k,unsigned i){return k?k+i:0;}
/* DVXSC 07430 (Comanche 07377): scalar in MPAC, vector operand at C(ADDRWD), to DANZIG. */
static NI void dvxsc(const agc_word *k){
 DCA(rMPAC);DXCH(rMPAC+3);TC(IF_(07434,07403));dmpsub(k);TC(IF_(07435,07404));roundsub(2);
 CAF(TWO);ADS(rADDRWD);DCA(rMPAC+3);DXCH(rMPAC);DXCH(rMPAC+5);TC(IF_(07444,07413));dmpsub(kat(k,2));TC(IF_(07445,07414));roundsub(2);
 CAF(TWO);ADS(rADDRWD);DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rMPAC+3);TC(IF_(07453,07422));dmpsub(kat(k,4));TC(IF_(07454,07423));roundsub(2);
 DXCH(rMPAC);DXCH(rMPAC+5);DXCH(rMPAC);vmode();
}
static NI void lmi_vxsc(const agc_word *k){
 uint8_t c=CCS(rMODE);
 if(c<2u){dvxsc(k);return;}
 /* VVXSC (-0 enters at its TC VROUND). */
 if(c==2u){TC(IF_(07405,07354));dmpsub(k);}
 TC(IF_(07406,07355));roundsub(2);
 DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rMPAC+3);TC(IF_(07412,07361));dmpsub(k);TC(IF_(07413,07362));roundsub(2);
 DXCH(rMPAC+5);DXCH(rMPAC);DXCH(rMPAC+5);TC(IF_(07417,07366));dmpsub(k);TC(IF_(07420,07367));roundsub(2);
 vrotatex();
}
/* VPROJ 07425 (Comanche 07374): TC PREDOT (DOTINC 2, DOTSUB); CS FOUR; ADS ADDRWD back to
 * the operand; DVXSC. */
static NI void lmi_vproj(const agc_word *k){
 TC(IF_(07426,07375));CAF(TWO);(void)TS(rDOTINC);dotsub(k);CSF(FOUR);ADS(rADDRWD);dvxsc(k);
}
/* V/SC2 02654 (vector in MPAC, scalar in BUF) to DANZIG. */
static NI void vsc2(void){
 uint8_t c;
 CSF(ONE);(void)TS(rDVNORMCT);(void)TS(rVBUF+5);
 TC(02660);vecagree();
 DXCH(rBUF);TC(02662);alsignag();DXCH(rBUF);
 c=CCS(rBUF);
 if(c==0u)goto bufplus;
 if(c==2u)goto bufminus;
 /* Major part zero: shift vector and scalar left 14, DVOVF on overflow. */
 XCH(rBUF+1);XCH(rBUF);XCH(rMPAC+1);XCH(rMPAC);
 if(!ai_bzf()){dvovf();return;}
 XCH(rMPAC+4);XCH(rMPAC+3);
 if(!ai_bzf()){dvovf();return;}
 XCH(rMPAC+6);XCH(rMPAC+5);
 if(!ai_bzf()){dvovf();return;}
 c=CCS(rBUF);
 if(c==0u)goto bufplus;
 if(c!=2u){dvovf();return;}   /* zero divisor */
bufminus:
 DCS(rBUF);DXCH(rBUF);INCR(rVBUF+5);
bufplus:
 DCA(rBUF);DXCH(rBUF2);
 for(;;){
  CA(rBUF);DOUBLE();
  if(ai_ovsk())break;
  ai_aug(rDVNORMCT);DCA(rBUF);DAS(rBUF);
 }
 TC(02737);if(vscdv())return;
 DXCH(rMPAC+3);DXCH(rMPAC);DXCH(rMPAC+3);
 TC(02743);if(vscdv())return;
 DXCH(rMPAC+5);DXCH(rMPAC);DXCH(rMPAC+5);
 TC(02747);if(vscdv())return;
 vrotatex();
}
/* V/SC 07624: VV/SC (vector mode, scalar operand) or DV/SC (scalar mode,
 * vector operand; mode becomes vector), V/SC1 DXCH BUF; FBANK 0; V/SC2. */
static NI void lmi_vsc(const agc_word *k){
 if(CCS(rMODE)<2u){
  dca_op(k,2);DXCH(rMPAC+3);dca_op(k,4);DXCH(rMPAC+5);
  CSF(ONE);(void)TS(rMODE);dca_op(k,0);DXCH(rMPAC);
 }else dca_op(k,0);
 DXCH(rBUF);CAF(ZERO);(void)TS(rFB);vsc2();
}
static NI void lmi_vxv(const agc_word *k){
 DCA(rMPAC+5);DXCH(rMPAC);DXCH(rVBUF);TC(IF_(07465,07434));dmpsub(k);
 DCS(rMPAC+3);DXCH(rMPAC);DXCH(rVBUF+2);TC(IF_(07472,07441));dmpsub(k);
 CAF(TWO);ADS(rADDRWD);
 DCS(rMPAC+5);DXCH(rMPAC);DXCH(rMPAC+5);TC(IF_(07501,07450));dmpsub(kat(k,2));
 DCA(rVBUF);DXCH(rMPAC);DXCH(rVBUF+4);TC(IF_(07506,07455));dmpsub(kat(k,2));
 CAF(TWO);ADS(rADDRWD);
 DCS(rVBUF);DXCH(rMPAC);DAS(rMPAC+5);
 if(!ai_bzf()){TC(IF_(07517,07466));overflwz();}
 TC(IF_(07520,07467));dmpsub(kat(k,4));
 DXCH(rVBUF+2);DXCH(rMPAC+3);DXCH(rMPAC);DAS(rMPAC+3);
 if(!ai_bzf()){TC(IF_(07527,07476));overflwy();}
 TC(IF_(07530,07477));dmpsub(kat(k,4));DXCH(rVBUF+4);endvxv();
}

/* VXM/MXV 07342 (Comanche 07311; DOTINC in A, MATINC set): three DOTSUBs of the MPAC vector
 * (saved in VBUF) with the rows (MXV) or columns (VXM) of the matrix at
 * C(ADDRWD) or `k` (18 words), result in MPAC, to DANZIG. */
static NI void vxmmxv(const agc_word *k){
 unsigned base=W[rADDRWD];
 (void)TS(rDOTINC);TC(IF_(07344,07313));mpacvbuf();
 TC(IF_(07345,07314));dotsub(k);
 DCA(rVBUF);DXCH(rMPAC);DXCH(rBUF2);DCA(rVBUF+2);DXCH(rMPAC+3);DCA(rVBUF+4);DXCH(rMPAC+5);
 CA(rMATINC);ADS(rADDRWD);
 TC(IF_(07362,07331));dotsub(kat(k,(unsigned)(W[rADDRWD]-base)));
 DXCH(rVBUF);DXCH(rMPAC);DXCH(rVBUF+2);DXCH(rMPAC+3);DXCH(rVBUF+4);DXCH(rMPAC+5);
 CA(rMATINC);ADS(rADDRWD);
 TC(IF_(07373,07342));dotsub(kat(k,(unsigned)(W[rADDRWD]-base)));
 DXCH(rBUF2);DXCH(rMPAC);DXCH(rMPAC+5);DXCH(rVBUF+2);DXCH(rMPAC+3);danzig();
}
/* MXV 07334 (Comanche 07303) CAF TWO; TS MATINC; TCF VXM/MXV. */
static NI void lmi_mxv(const agc_word *k){CAF(TWO);(void)TS(rMATINC);vxmmxv(k);}
/* VXM 07337 (Comanche 07306) CS TEN; TS MATINC; CAF SIX; VXM/MXV. */
static NI void lmi_vxm(const agc_word *k){CSF(TEN);(void)TS(rMATINC);CAF(SIX);vxmmxv(k);}

/* ---- BOV(B), RTB and POLY ----------------------------------------------- */
/* BPL/BMN 01,2474: CCS CYR; positive TCF BPL (TC BRANCH; + or zero GOTO),
 * negative BMN (TC BRANCH; only negative GOTO), -0 DANZIG. Returns 1 at GOTO 06646 (the
 * caller continues with lmi_goto), 0 at DANZIG. */
static NI uint8_t lmi_bplbmn(void){
 uint8_t c=CCS(rCYR),b;
 if(c==0u){TC(IF_(02504,02531));b=branch();if(b!=2u){W[5]=IF_(06646,06615);return 1;}danzig();return 0;}
 if(c==3u){danzig();return 0;}
 TC(IF_(02500,02525));b=branch();if(b==2u){W[5]=IF_(06646,06615);return 1;}danzig();return 0;
}
static NI uint8_t lmi_bov(void){
 uint8_t c=CCS(rOVFIND);
 if(c==1u){danzig();return LMI_BOV_NONE;}
 if(c!=3u)(void)TS(rOVFIND);
 c=CCS(rCYR);
 if(c==0u){W[5]=IF_(02450,02475);return LMI_BOV_RTB;}
 if(c==2u){W[5]=IF_(06646,06615);return LMI_BOV_GOTO;}
 W[5]=(agc_word)(c==1u?IF_(02465u,02512u):IF_(02467u,02514u));return LMI_BOV_OUT_OF_SCOPE;
}
static NI void lmi_rtb(void){
 /* RTB CA POLISH; TC SWCALL -1: INCR Q; TS L; LXCH FBANK; MASK LOW10;
  * XCH Q; DXCH BUF2; INDEX Q; TC 2000 (Q = SWRETURN). */
 unsigned sub;
 CA(rPOLISH);TC(IF_(02452,02477));INCR(rQ);(void)TS(rL);LXCH(rFB);MASKF(LOW10);XCH(rQ);DXCH(rBUF2);
 sub=W[rQ]&01777u;TC(IF_(04631,04675));W[5]=(agc_word)(02000u+sub);
}
static NI void lmi_swreturn(void){
 /* SWRETURN XCH BUF2 +1; XCH FBANK; XCH BUF2 +1; TC BUF2 (the TC held
  * there, leaving Q = BUF2 +1, reaches RTB's TCF DANZIG). */
 XCH(rBUF2+1);XCH(rFB);XCH(rBUF2+1);ai_tc(rBUF2+1u);danzig();
}
static NI void lmi_poly(const agc_word *table){
 agc_word q=(agc_word)(W[rQ]&077777u);
 CAF(table[0]);(void)TS(rPOLYCNT);DOUBLE();AD(rQ);(void)TS(rPOLISH);ADF(FIVE);(void)TS(rPOLYRET);
 CAF(LVBUF);(void)TS(rADDRWD);
 ai_dcaf(table+(W[rPOLISH]-q)+3u);DXCH(rMPAC);DXCH(rVBUF);
 for(;;){
  TC(IF_(07246,07215));dmpsub(0);ai_dcaf(table+(W[rPOLISH]-q)+1u);DAS(rMPAC);
  if(CCS(rPOLYCNT)!=0u)break;
  (void)TS(rPOLYCNT);CSF(TWO);ADS(rPOLISH);
 }
 /* TC POLYRET: the TC held there, leaving Q = 0142. */
 W[5]=(agc_word)(W[rPOLYRET]&07777u);ai_tc(rPOLYRET+1u);
}
/* ---- SINE/COSINE (bank 0) ---------------------------------------------- */
/* SN1's POLY table (DEC 3 and the five 2DEC coefficients at 00,3560). */
static const agc_word sin_poly[11]={3,014441,037325,053250,060764,012146,021276,075466,071471,0236,032757};
#define NEG1_2_AT IF_(04734u,05000u)
/* SINE 00,3530 to DANZIG: (1/2)SIN(2 PI MPAC) by Hastings' polynomial. */
static NI void sine(void){
 unsigned i;
 DXCH(rMPAC);DDOUBL();
 if(ai_ovsk())ai_dcom();   /* lost or gained PI: SIN(X-PI) = SIN(-X) */
 DXCH(rMPAC);CA(rMPAC);DOUBLE();
 if(TS(rL)){
  /* Over .5 in magnitude: PI - X, or -PI - X. */
  CAF(fixed_indexed(NEG1_2_AT+1u,W[rA]));DOUBLE();ai_su(rMPAC);(void)TS(rMPAC);CS(rMPAC+1);(void)TS(rMPAC+1);
 }
 /* SN1 */
 DCA(rMPAC);DXCH(rBUF2);TC(IF_(03557,03556));dsqsub();
 TC(IF_(03560,03557));lmi_poly(sin_poly);
 /* CAF LBUF2; TC DMPSUB -1; then shift left 2. */
 CAF(rBUF2);TC(IF_(03575,03574));(void)TS(rADDRWD);dmpsub(0);
 for(i=0;i<2u;++i){DCA(rMPAC+1);DAS(rMPAC+1);AD(rMPAC);ADS(rMPAC);}
 danzig();
}
static NI uint8_t lmi_sin(void){sine();return 0;}
static NI uint8_t lmi_cos(void){
 /* COSINE 00,3517: COS(X) = SIN(PI/2 - ABS(X)). */
 TC(IF_(03520,03517));
 if(branch()==0u){DCS(rMPAC);DXCH(rMPAC);}
 /* PRESINE CAF QUARTER; ADS MPAC. */
 CAF(QUARTER);ADS(rMPAC);sine();
 return 0;
}

/* ---- ARCSIN/ARCCOS (bank 0) -------------------------------------------- */
#define rESCAPE 0136u   /* ESCAPE = DOTINC, ESCAPE2 = DOTRET */
#define rESCAPE2 0137u
/* The TCF words LASINEX, LDANZIG and TCSUBTR that ARCSIN/ARCCOS store in
 * ESCAPE and ESCAPE2, and ACOSSHR's TC VSHRRND return (ACOSABRT differs
 * between the programs, so these are written per program rather than as
 * checked IF_ address pairs). */
#ifdef INTERP_COMANCHE
#define W_LASINEX 013706u
#define W_LDANZIG 016030u
#define W_TCSUBTR 013701u
#define ACOSSHR_RET 03717u
#else
#define W_LASINEX 013707u
#define W_LDANZIG 016061u
#define W_TCSUBTR 013702u
#define ACOSSHR_RET 03720u
#endif
/* ACOS3's POLY table (DEC 6 and the eight 2DEC coefficients at 00,3656). */
static const agc_word acos_poly[17]={6,013240,023630,074721,047775,02440,020237,075067,070742,
 03436,026756,074037,057640,03046,07143,076654,042244};
/* ASINEX: PI/2 - ARCCOS(X); LDANZIG TCF DANZIG. */
static NI void asinex(void){DCS(rMPAC);ADF(QUARTER);DXCH(rMPAC);danzig();}
/* TC ESCAPE (`ret` the address after it): run the TCF this routine stored
 * there. SUBTR's TC ESCAPE2 reaches ASINEX or DANZIG the same way. */
static NI void escape(agc_word ret){
 agc_word w=W[rESCAPE];
 TC(ret);
 if(w==W_TCSUBTR){
  /* SUBTR: PI - ARCCOS(-X). */
  DCS(rMPAC);ADF(HALF);DXCH(rMPAC);TC(IF_(03707,03706));w=W[rESCAPE2];
 }
 if(w==W_LASINEX)asinex();else danzig();
}
/* ACOS=0 (A = the answer's major part; `minor` 0 enters at ACOS=0 +1). */
static NI void acos0(uint8_t minor){
 if(minor)(void)TS(rMPAC+1);
 (void)TS(rMPAC);escape(IF_(03642,03641));
}
/* ARCSIN 00,3610 / ARCCOS 00,3612 (UNAJUMP) to DANZIG (0), to the alarm
 * service with alarm 1301 (1: Luminary TC ALARM1, Comanche TC ALARM, Q at
 * the OCT 1301), or to SQRTSUB's abort (2, as SQRT). */
static NI uint8_t arccos_body(uint8_t asin){
 uint8_t c;
 CAF(asin?W_LASINEX:W_LDANZIG);(void)TS(rESCAPE);
 TC(IF_(03615,03614));c=branch();
 if(c==1u){CAF(QUARTER);acos0(0);return 0;}       /* ACOSZERO */
 if(c==2u){
  /* Negative: ARCCOS(X) = PI - ARCCOS(-X). */
  DCS(rMPAC);DXCH(rMPAC);CAF(W_TCSUBTR);XCH(rESCAPE);(void)TS(rESCAPE2);
 }
 /* ACOSST CS HALF; AD MPAC; CCS A. */
 CSF(HALF);AD(rMPAC);c=CCS(rA);
 if(c==0u){
  /* ACOSOVF: one more than .5 is zero; else ACOSABRT. */
  if(ai_bzf()){acos0(1);return 0;}
#ifdef INTERP_COMANCHE
  TC(03723);W[5]=05650;
#else
  DCA(rLOC);TC(03726);W[5]=05732;
#endif
  return 1;
 }
 if(c==1u){asinex();return 0;}                     /* LASINEX TCF ASINEX */
 if(c==3u){
  /* Major part .5: answer zero unless the minor part is negative. */
  c=CCS(rMPAC+1);
  if(c==0u)CAF(ZERO);
  if(c!=2u){acos0(1);return 0;}
 }
 /* ACOSST2: SQRT(1-X)P(X) (Hastings). */
 DCS(rMPAC);ADF(HALF);DXCH(rMPAC);DXCH(rBUF2);
 TC(IF_(03650,03647));if(sqrtsub())return 2;
 if(CCS(rMPTEMP)==0u){
  /* ACOSSHR INDEX A; CAF BIT14; TS MPTEMP; TC VSHRRND. */
  CAF(bit14_indexed(rA));(void)TS(rMPTEMP);TC(ACOSSHR_RET);vshrrnd();
 }
 /* ACOS3 (MPTEMP is never negative after SQRTSUB). */
 DXCH(rMPAC);DXCH(rBUF2);DXCH(rMPAC);
 TC(IF_(03656,03655));lmi_poly(acos_poly);
 /* CAF LBUF2; TC DMPSUB -1 (TS ADDRWD); TC ESCAPE. */
 CAF(rBUF2);TC(IF_(03701,03700));(void)TS(rADDRWD);dmpsub(0);
 escape(IF_(03702,03701));return 0;
}
static NI uint8_t lmi_acos(void){return arccos_body(0);}
static NI uint8_t lmi_asin(void){return arccos_body(1);}
/* ---- VAD/VSU, BVSU, TAD, ROUND, SSP --------------------------------------- */
/* VSU 06747 / VAD 06751: ADS ADDRWD with BIT15 or PRIO30 turns the indexed
 * READs into DCS or DCA of the operand's +2, +4 and +0 words; ADDRWD keeps
 * the sum. Ends through ENDVXV at DANZIG. */
static NI void vaddsub(const agc_word *k,uint8_t sub){
 unsigned a=W[rADDRWD];unsigned i;static const unsigned off[3]={2u,4u,0u};
 if(sub)CAF(BIT15);else CAF(030000u);
 ADS(rADDRWD);
 for(i=0;i<3u;++i){
  if(sub){if(k)ai_dcsf(k+off[i]);else DCS(AT(a+off[i]));}
  else{if(k)ai_dcaf(k+off[i]);else DCA(AT(a+off[i]));}
  if(i==2u)break;
  DAS(AT(rMPAC+3u+2u*i));
  if(!ai_bzf()){if(i==0u){TC(IF_(06762,06731));overflwy();}else{TC(IF_(06771,06740));overflwz();}}
 }
 endvxv();
}
static NI void lmi_vad(const agc_word *k){vaddsub(k,0);}
static NI void lmi_vsu(const agc_word *k){vaddsub(k,1);}
static NI void lmi_bvsu(const agc_word *k){
 dca_op(k,2);DXCH(rMPAC+3);ai_dcom();DAS(rMPAC+3);
 if(!ai_bzf()){TC(IF_(07050,07017));overflwy();}
 dca_op(k,4);DXCH(rMPAC+5);ai_dcom();DAS(rMPAC+5);
 if(!ai_bzf()){TC(IF_(07062,07031));overflwz();}
 dca_op(k,0);DXCH(rMPAC);ai_dcom();endvxv();
}
static NI void lmi_tad(const agc_word *k){
 /* TAD 07071: DCA 1; DAS MPAC +1; AD 0; AD MPAC; TS MPAC (skip on
  * overflow to TCF SETOVF). */
 dca_op(k,1);DAS(rMPAC+1);
 if(k)ADF(k[0]);else AD(AT(W[rADDRWD]));
 AD(rMPAC);
 if(!TS(rMPAC)){danzig();return;}
 TC(IF_(07004,06753));(void)overflow();danzig();
}
static NI uint8_t lmi_round(void){
 /* ROUND 02116 (bank 0): TC ROUNDSUB; TCF DANZIG. */
 TC(02117);roundsub(0);danzig();
 return 0;
}
static NI void lmi_ssp(agc_word word){
 /* SSP 06620: INCR LOC; INDEX LOC, CA 0 (the word given); STORE1 INDEX
  * ADDRWD, TS 0; TCF DANZIG. */
 INCR(rLOC);CAF(word);(void)TS(AT(W[rADDRWD]));danzig();
}

/* ---- Index orders, BHIZ, CALL, indirect GOTO, EXIT and INTPRET ------------ */
#define rINDEXLOC 0130u
/* 15ADRERS 01,2427 into TAGSUB and back through TC Q. */
static NI void adrers15(void){
 uint8_t c;
 CS(rPOLISH);ADF(DEC45);c=CCS(rA);
 if(c==0u){CA(rFIXLOC);}
 else if(c>=2u){if(c==2u)CAF(OCT1400);XCH(rPOLISH);(void)TS(rEB);MASKF(LOW8);}
 ADS(rPOLISH);
 CA(rFIXLOC);(void)TS(rINDEXLOC);
 if(CCS(rCYR)==0u)INCR(rINDEXLOC);
 RETQ();
}
static NI void xad2(void){ADS(AT(W[rINDEXLOC]+tX1));danzig();}
static NI uint8_t lmi_xad(void){TC(IF_(02401,02426));adrers15();CA(AT(W[rPOLISH]));xad2();return 0;}
static NI uint8_t lmi_xsu(void){TC(IF_(02412,02437));adrers15();CS(AT(W[rPOLISH]));xad2();return 0;}
static NI uint8_t lmi_lxa(void){TC(IF_(02355,02402));adrers15();CA(AT(W[rPOLISH]));(void)TS(AT(W[rINDEXLOC]+tX1));danzig();return 0;}
/* AXC 01,2351: TC TAGSUB (INDEXLOC = FIXLOC, +1 for X2); CS POLISH; TC
 * XSTORE; XSTORE INDEX INDEXLOC, TS X1; TCF DANZIG. */
static NI uint8_t lmi_axc(void){
 CA(rFIXLOC);(void)TS(rINDEXLOC);if(CCS(rCYR)==0u)INCR(rINDEXLOC);W[2]=3;
 CS(rPOLISH);TC(IF_(02354,02401));(void)TS(AT(W[rINDEXLOC]+tX1));danzig();
 return 0;
}
/* LXC 01,2360 (Comanche 01,2405): TC 15ADRERS; INDEX POLISH, CS 0; XSTORE. */
static NI uint8_t lmi_lxc(void){TC(IF_(02361,02406));adrers15();CS(AT(W[rPOLISH]));(void)TS(AT(W[rINDEXLOC]+tX1));danzig();return 0;}
static NI uint8_t lmi_sxa(void){TC(IF_(02365,02412));adrers15();CA(AT(W[rINDEXLOC]+tX1));(void)TS(AT(W[rPOLISH]));danzig();return 0;}
static NI uint8_t lmi_xchx(void){
 /* XCHX TC 15ADRERS; INDEX POLISH, CA 0; INDEX INDEXLOC, XCH X1;
  * MSTORE1 INDEX POLISH, TS 0; TCF DANZIG. */
 TC(IF_(02373,02420));adrers15();CA(AT(W[rPOLISH]));XCH(AT(W[rINDEXLOC]+tX1));(void)TS(AT(W[rPOLISH]));danzig();
 return 0;
}
static NI void lmi_ita(void){
 /* CALL/ITA CCS CYR (negative: ITA/STQ); TC 15ADRERS; INDEX FIXLOC,
  * CA QPRET; MSTORE1 INDEX POLISH, TS 0; TCF DANZIG. */
 (void)CCS(rCYR);TC(IF_(02513,02540));adrers15();CA(AT(W[rFIXLOC]+tQPRET));(void)TS(AT(W[rPOLISH]));danzig();
}
static NI uint8_t lmi_tix(void){
 /* TIX 01,2415: TC TAGSUB; INDEX INDEXLOC, CS S1; INDEX INDEXLOC, AD X1;
  * EXTEND, BZMF DANZIG; DOTIXBR INDEX INDEXLOC, XCH X1; TCF GOTO 06646. */
 TC(IF_(02416,02443));CA(rFIXLOC);(void)TS(rINDEXLOC);if(CCS(rCYR)==0u)INCR(rINDEXLOC);RETQ();
 CS(AT(W[rINDEXLOC]+tS1));AD(AT(W[rINDEXLOC]+tX1));
 if(ai_bzmf()){danzig();return 0;}
 XCH(AT(W[rINDEXLOC]+tX1));W[5]=IF_(06646,06615);return 1;
}
static NI uint8_t lmi_bhiz(void){
 /* BHIZ 01,2452: CCS MPAC; + and - DANZIG, +0 and -0 GOTO 06646. */
 uint8_t c=CCS(rMPAC);
 if(c==1u||c==3u){W[5]=IF_(06646,06615);return 1;}
 danzig();return 0;
}
static NI void lmi_call(void){
 /* CALL/ITA CCS CYR (positive) TCF CALL; CALL CA BANKSET; MASK BANKMASK;
  * AD BANKMASK; AD LOC; INDEX FIXLOC, TS QPRET; GOTO (fixed target). */
 (void)CCS(rCYR);CA(rBANKSET);MASKF(076000u);ADF(076000u);AD(rLOC);(void)TS(AT(W[rFIXLOC]+tQPRET));
 W[5]=IF_(06646,06615);
}
static NI void lmi_goto_any(void){
 /* GOTO CA POLISH; MASK HIGH4; BZF GOTOERS (an erasable CADR is fetched,
  * through the work area or the EBANK it names, until a fixed one). */
 for(;;){
  uint8_t c;
  CA(rPOLISH);MASKF(074000u);
  if(!ai_bzf())break;
  CA(rPOLISH);ADF(ENDVAC_NEG);c=CCS(rA);
  if(c==0u||c==1u){
   if(c==0u)CA(rPOLISH);
   (void)TS(rEB);MASKF(LOW8);CA(AT(01400u+W[rA]));
  }else{
   if(c==2u)CA(rFIXLOC);
   AD(rPOLISH);CA(AT(W[rA]));
  }
  (void)TS(rPOLISH);
 }
 lmi_goto();
}
/* CGOTO 06703 (ADDRWD names the modifier; `list` is the CADR-list word after
 * the operand and cadrs its n words): INDEX LOC, CA 1; INDEX ADDRWD, AD 0;
 * TS FBANK; MASK LOW10; INDEX A, CA 10000; TS POLISH; TCF GOTO +1, to the
 * target's NEWOPS. 0, with the image unchanged, when the modifier selects
 * a word outside the list. */
static NI void lmi_15bitadr(agc_word word,agc_word target){
 /* 15BITADR 06252 (from STCALL's STOTC): INCR LOC; INDEX LOC, CA 0;
  * TS POLISH; CAF LOW7+2K; TS FBANK; MASK CYR; INDEX A, TCF MISCJUMP. */
 INCR(rLOC);CAF(word);(void)TS(rPOLISH);CAF(02177u);(void)TS(rFB);MASK(rCYR);W[5]=target;
}
static NI uint8_t lmi_bzegoto(void){
 /* BZE/GOTO 01,2467: CCS CYR (positive) TC BRANCH; zero TCF GOTO (1, at
  * GOTO 06646 for lmi_goto or lmi_goto_any), else DANZIG (0); GOTO when
  * CYR is not positive (1). */
 if(CCS(rCYR)!=0u){W[5]=IF_(06646,06615);return 1;}
 TC(IF_(02471,02516));
 if(branch()==1u){W[5]=IF_(06646,06615);return 1;}
 danzig();return 0;
}
static NI void lmi_exit(void){
 /* EXIT 06743: CA BANKSET; TS BBANK; INDEX LOC, TC 1 (to the basic code
  * after the order; Q = 06747). */
 unsigned loc;
 CA(rBANKSET);(void)TS(rBB);loc=W[rLOC];TC(IF_(06747,06716));W[5]=(agc_word)((loc+1u)&07777u);
}
static NI void lmi_intpret(void){
 /* TC INTPRET (Q = the word after the TC): RELINT; QXCH LOC; CA BBANK;
  * TS BANKSET; MASK BIT15; TS INTBIT15; TS EDOP; TCF NEWOPS. */
 io->interrupt_enabled=1;ai_qxch(rLOC);CA(rBB);(void)TS(rBANKSET);MASKF(BIT15);(void)TS(rINTBIT15);(void)TS(rEDOP);W[5]=IF_(06070,06037);
}
static NI void lmi_tload(void){
 /* TLOAD 06470 (erasable operand at ADDRWD): INDEX ADDRWD, CA 2; TS MPAC+2;
  * EXTEND; INDEX ADDRWD, DCA 0; DXCH MPAC; TMODE CAF ONE; TCF NEWMODE. */
 CA(AT(W[rADDRWD]+2u));(void)TS(rMPAC+2);DCA(AT(W[rADDRWD]));DXCH(rMPAC);CAF(ONE);newmode();
}
/* TLOAD with a fixed operand: the same path, the three words passed in k. */
static NI void lmi_tload_fixed(const agc_word *k){
 CAF(k[2]);(void)TS(rMPAC+2);ai_dcaf(k);DXCH(rMPAC);CAF(ONE);newmode();
}
/* SLOAD with a fixed operand (Comanche P40-P47's SLOAD of a rope
 * constant): ZL; CA of the word passed in k; TCF SLOAD2. */
static NI void lmi_sload_fixed(const agc_word *k){
 W[rL]=0;CAF(k[0]);DXCH(rMPAC);CAF(ZERO);(void)TS(rMPAC+2);newmode();
}
/* ---- runtime ---- */
#define MISSION IF_(0u,1u)
enum { rNEWJOB = 067 };
enum { DONE = 0, MORE = 1, REFUSE = 2 };
static const agc_word t_indjump[32] = IRT_INDJUMP, miscjump[16] = IRT_MISCJUMP, unajump[15] = IRT_UNAJUMP;

/* Physical rope address of fixed logical address a under bank register b
 * (FB or BANKSET's bits 14-11) and the channel-7 superbank bits; 0xffff for
 * erasable or outside the rope. */
static NI uint16_t phys(unsigned a, agc_word b)
{
    unsigned bank, sb = io->superbank & 0160u;
    if (a >= 04000u) return (uint16_t)(2048u + (a & 07777u) - 04000u);
    if (a < 02000u) return 0xffffu;
    bank = (b >> 10) & 037u;
    if (bank >= 030u && sb) {
        if (sb != 0100u) return 0xffffu;
        bank += 010u;
    }
    return (uint16_t)(bank * 1024u + (a & 01777u));
}
static NI uint8_t rope(uint16_t p, agc_word *w) { return p != 0xffffu && apollo_rope_read(MISSION, p, w); }
/* The word off words after LOC in the interpretive program, into cw. */
static agc_word cw;
static NI uint8_t code_word(unsigned off)
{
    return rope(phys((W[rLOC] + off) & 077777u, W[rBANKSET]), &cw);
}
/* Fixed operand words at ADDRWD under FB (n words; past the rope's end
 * reads as zero, which no order uses). Returns them, or 0 for an erasable
 * operand (the bodies then read C(ADDRWD) from the image). */
static agc_word kw[18];
static NI const agc_word *operand(unsigned n)
{
    unsigned i, a = W[rADDRWD] & 077777u;
    if (a < 02000u) return 0;
    for (i = 0; i < n; ++i) if (!rope(phys(a + i, W[4]), &kw[i])) kw[i] = 0;
    return kw;
}

/* DANZIG's job switch: NEWJOB positive goes to CHANG2 (CCS EDOP +0; CCS
 * NEWJOB; TCF CHANG2), from DANZIG (bank) or NOIBNKSW (no bank). */
static NI void chang2(uint8_t bank)
{
    if (bank) { ai_ca(rBANKSET); (void)ai_ts(6); }
    (void)ai_ccs(rEDOP); (void)ai_ccs(rNEWJOB); W[5] = IRT_CHANG2;
}

/* CGOTO 04 (ADDRWD names the modifier, the word after the operand the CADR
 * list): INDEX LOC, CA 1; INDEX ADDRWD, AD 0; TS FBANK; MASK LOW10; INDEX A,
 * CA 10000; TS POLISH; TCF GOTO +1. */
static NI uint8_t cgoto(void)
{
    agc_word list, v;
    int i;
    if (!code_word(1u)) return REFUSE;
    list = cw;
    v = ai_word(W[rADDRWD]);
    i = (v & 040000u) ? -(int)(~v & 037777u) : (int)v;
    if (i < 0 || (list & 01777u) + (unsigned)i > 01777u) return REFUSE;
    if (!rope(phys(02000u + ((list & 01777u) + (unsigned)i), list), &cw)) return REFUSE;
    ai_caf(list); ai_ad(W[rADDRWD]); (void)ai_ts(4); ai_maskf(01777u);
    ai_caf(cw); (void)ai_ts(rPOLISH); lmi_goto_any();
    return DONE;
}

/* The address orders (INDJUMP) after address formation, by table index;
 * 004 CGOTO and 011 SSP read a further code word, 015 CCALL (no program
 * uses it) and 035 (unassigned) refuse. */
typedef void (*addr_order)(const agc_word *);
static NI void tload_any(const agc_word *k) { if (k) lmi_tload_fixed(k); else lmi_tload(); }
static NI void sload_any(const agc_word *k) { if (k) lmi_sload_fixed(k); else lmi_sload(); }
static const addr_order addr_orders[32] = {
    lmi_vload, lmi_tad, lmi_sign, lmi_vxsc, 0, tload_any, lmi_dload_op, lmi_vsc,
    sload_any, 0, lmi_pddl_op, lmi_mxv, lmi_pdvl, 0, lmi_vxm, lmi_tslc,
    lmi_dmpr, lmi_ddv, lmi_bddv, lmi_gshift, lmi_vad, lmi_vsu, lmi_bvsu, lmi_dot,
    lmi_vxv, lmi_vproj, lmi_dsu, lmi_bdsu, lmi_dad, 0, lmi_dmp, lmi_setpd
};
static NI uint8_t addr_body(unsigned idx)
{
    const agc_word *k = operand(idx == 013u || idx == 016u ? 18u : 6u);
    if (idx == 004u) return cgoto();
    if (idx == 011u) {
        if (!code_word(1u)) return REFUSE;
        lmi_ssp(cw); return MORE;
    }
    if (!addr_orders[idx]) return REFUSE;
    addr_orders[idx](k);
    return MORE;
}

/* Erasable branch word (a switch order's or GOTO's CADR in erasable). */
static NI void swbranch(agc_word w)
{
    if (w & 074000u) { lmi_swbranch(w); return; }
    ai_ca(rBANKSET); (void)ai_ts(4); ai_caf(w); (void)ai_ts(rPOLISH); lmi_goto_any();
}

/* MISCJUMP 000-013 and UNAJUMP 000-013 bodies: nonzero is the branch
 * (GOTO) for the miscellaneous orders, the end of the slot for the unary
 * ones. */
typedef uint8_t (*plain_order)(void);
static const plain_order misc_orders[12] = {
    lmi_axt, lmi_axc, lmi_lxa, lmi_lxc, lmi_sxa, lmi_xchx, lmi_incr, lmi_tix,
    lmi_xad, lmi_xsu, lmi_bzegoto, lmi_bplbmn
};
static const plain_order unary_orders[12] = {
    lmi_sqrt, lmi_sin, lmi_cos, lmi_asin, lmi_acos, lmi_dsq, lmi_round, lmi_comp,
    lmi_vdef, lmi_unit, lmi_abvalabs, lmi_vsq
};

/* One order with code c (OPJUMP done: CYR holds it, A its CCS). */
static NI uint8_t order(unsigned c)
{
    unsigned idx;
    uint8_t g;
    if (c == 0u) { lmi_exit(); return DONE; }
    if (c & 1u) {
        idx = c >> 2;
        if (!code_word(1u)) return REFUSE;
        if (c & 2u) lmi_indexed(cw, t_indjump[idx]);
        else if (cw & 040000u) {
            if (idx == 037u || idx == 004u || idx == 011u) return REFUSE;   /* SETPD, CGOTO, SSP push up: never assembled */
            lmi_address_pushup(cw, t_indjump[idx]);
        } else lmi_direct(cw, t_indjump[idx]);
        return addr_body(idx);
    }
    if (c & 2u) {
        idx = c >> 3;
        if (!code_word(1u)) return REFUSE;
        lmi_misc(cw, miscjump[idx]);
        if (idx < 014u) g = misc_orders[idx]();
        else if (idx == 014u) {
            (void)ai_ccs(rCYR);
            if (!(c & 4u)) { lmi_rtb(); return DONE; }
            g = lmi_bhiz();
        } else if (idx == 015u) {
            if (c & 4u) { lmi_ita(); return MORE; }
            lmi_call(); g = 1;
        } else if (idx == 016u) {
            if (!lmi_switch()) return MORE;
            if (!code_word(1u)) return REFUSE;
            swbranch(cw); return DONE;
        } else {
            g = lmi_bov();
            if (g == LMI_BOV_RTB) { lmi_rtb(); return DONE; }
            if (g == LMI_BOV_OUT_OF_SCOPE) return DONE;
        }
        if (g) { lmi_goto_any(); return DONE; }
        return MORE;
    }
    if (c & 4u) {
        lmi_short_dispatch();
        if (W[5] == IRT_SHORTT) lmi_shortt(); else lmi_shortv();
        return MORE;
    }
    idx = (c >> 3) - 1u;
    lmi_unary(unajump[idx]);
    if (idx < 014u) return unary_orders[idx]() ? DONE : MORE;
    if (idx == 014u) {
        if (!code_word(1u)) return REFUSE;
        lmi_stadr(cw); return MORE;
    }
    if (idx == 015u) { lmi_rvq(); return DONE; }
    lmi_push(); return MORE;
}

/* From the image's Z through the dispatch paths and bodies to the end of
 * the slot. */
static NI enum exec_status run(void)
{
    uint8_t r = MORE, load = 0;
    unsigned guard;
    for (guard = 0; guard < 16u && r == MORE; ++guard) {
        switch (W[5]) {
        case IRT_NEWOPS:
            if (!code_word(0u)) return EXEC_OUT_OF_SCOPE;
            if (!(cw & 040000u)) { lmi_store_code(cw); break; }
            lmi_newops(cw);
            r = order(((~(unsigned)cw & 077777u) - 1u) & 0177u); break;
        case IRT_DANZIG:
            if (W[rEDOP]) { unsigned c = W[rEDOP] - 1u; lmi_danzig_right(); r = order(c); break; }
            if (W[rNEWJOB]) { chang2(1); return EXEC_OK; }
            lmi_danzig_next(); return EXEC_OK;
        case IRT_NOIBNKSW:
            if (W[rEDOP]) { unsigned c = W[rEDOP] - 1u; lmi_noibnksw_right(); r = order(c); break; }
            if (W[rNEWJOB]) { chang2(0); return EXEC_OK; }
            (void)ai_ccs(rEDOP); (void)ai_ccs(rNEWJOB); ai_incr(rLOC); W[5] = IRT_NEWOPS; return EXEC_OK;
        case IRT_DODLOAD: lmi_dodload(); load = 0; break;
        case IRT_DOVLOAD: lmi_dovload(); load = 1; break;
        case IRT_DIRADRES:
            if (!code_word(1u)) return EXEC_OUT_OF_SCOPE;
            if (cw & 040000u) lmi_diradres_pushup(cw, t_indjump[load ? 000 : 006]);
            else lmi_diradres(cw, t_indjump[load ? 000 : 006]);
            r = addr_body(load ? 000 : 006); break;
        case IRT_DODLOADX: case IRT_DOVLOADX:
            load = W[5] == IRT_DOVLOADX;
            if (!code_word(1u)) return EXEC_OUT_OF_SCOPE;
            ai_caf(load ? 040001u : IRT_DLOADX_WORD); (void)ai_ts(rCYR);
            lmi_indexed(cw, t_indjump[load ? 000 : 006]);
            r = addr_body(load ? 000 : 006); break;
        case IRT_B15BITADR:
            if (!code_word(1u)) return EXEC_OUT_OF_SCOPE;
            lmi_15bitadr(cw, miscjump[015]); lmi_call(); lmi_goto_any(); return EXEC_OK;
        case IRT_GOTO: lmi_goto_any(); return EXEC_OK;
        case IRT_SWRETURN: lmi_swreturn(); break;
        case IRT_INTPRET: lmi_intpret(); return EXEC_OK;
        default: return guard ? EXEC_OK : EXEC_UNMAPPED;   /* outside the interpreter */
        }
    }
    return r == REFUSE ? EXEC_OUT_OF_SCOPE : EXEC_OK;
}

/* The image as the bodies need it (the compiled routines' sane()). */
static NI uint8_t sane(void)
{
    unsigned vac = W[rFIXLOC];
    if (W[7] || W[rNEWJOB] > 037777u || W[3] > 03400u || (W[3] & 0377u) || W[6] != (agc_word)((W[4] & 076000u) | (W[3] >> 8))) return 0;
    if (io->interrupt_enabled > 1u || (io->superbank & ~0160u)) return 0;
    if (vac < 0401u || vac > 0661u || (vac - 0401u) % 054u) return 0;
    return 1;
}

/* The image is always the canonical one at $7000 on the C64 (fixed
 * addressing is what keeps the bodies small); the host runs any image. */
static NI uint8_t bind(apollo_overlay_state *s)
{
#ifdef __mos__
    return s == (apollo_overlay_state *)APOLLO_SHARED_ADDRESS;
#else
    ic_image = s ? s->words : 0;
    return s != 0;
#endif
}
#ifdef INTERP_COMANCHE
#include "cm_native_image.h"
static NI enum exec_status entry(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    cm_native_io *cio = v;
    static p66_bias_io l;
    enum exec_status st;
    (void)p;
    if (!bind(s) || !cio) return EXEC_OUT_OF_SCOPE;
    l.superbank = cio->superbank; l.interrupt_enabled = cio->interrupt_enabled;
    io = &l;
    if (!sane()) return EXEC_OUT_OF_SCOPE;
    st = run();
    cio->superbank = l.superbank; cio->interrupt_enabled = l.interrupt_enabled;
    return st;
}
#else
static NI enum exec_status entry(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    (void)p;
    io = v;
    if (!bind(s) || !io || !sane()) return EXEC_OUT_OF_SCOPE;
    return run();
}
#endif
const agc_routine lm_interp_core = entry;
#define PH(z) (uint16_t)((z) >= 04000u ? 2048u + (z) - 04000u : (z) - 02000u)
static const agc_routine_entry basic[3] = {
    { PH(IRT_SWRETURN), entry }, { PH(IRT_INTPRET), entry }, { PH(IRT_DANZIG), entry }
};
const agc_routine_table lm_interp_core_basic = { basic, 3u };
#undef W
