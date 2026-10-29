/* SPDX-License-Identifier: MIT */
/* Fast forms of the agc_image_ops helpers for plain erasable cells; see
 * agc_fast_ops.h. Each helper has the effects of the ai_* helper of the same
 * name on the same image when the operand is a plain cell (no central,
 * bank or editing register), so the register and editing cases of
 * agc_image_ops.c are dropped and the operand is the cell itself. A and L
 * stay words 0 and 1 of the image. Checked against agc_image_ops and the
 * pinned yaAGC engine by tools/agc_fast_ops_differential.py. */
#include "agc_fast_ops.h"
#include "agc_hardware_divide.h"
#if !RK_FIXED_IMAGE
agc_image *afo_s;
#endif
#define W (AFO_S->words)
#define A W[0]
#define L W[1]
#define Q W[2]
#define M16 0177777u
#define AFO_IN static inline __attribute__((always_inline))
/* In flight builds (RK_FIXED_IMAGE) every helper stays one out-of-line
 * copy: link-time inlining of the small ones (afo_caf, afo_bzmf, ...) into
 * each call site made the LM DAP's per-pass set about a fifth larger
 * (27,326 -> 21,916 bytes) for 2% fewer cycles. Other builds are unchanged. */
#if RK_FIXED_IMAGE
#define AFO_NI __attribute__((noinline))
#else
#define AFO_NI
#endif

/* One's complement 16-bit add with end-around carry. */
AFO_IN agc_register add16(agc_register a,agc_register b){
 agc_register t=(agc_register)(a+b);
 if(t<a)++t;
 return t;
}
AFO_IN agc_register sx(agc_word v){return (agc_register)((v&077777u)|(((unsigned)v<<1)&0100000u));}
AFO_IN agc_word oc(agc_register v){return (agc_word)((v&037777u)|((v>>1)&040000u));}
AFO_IN agc_register ovf(agc_register v){
 switch(v&0140000u){case 040000u:return 1u;case 0100000u:return 0177776u;default:return 0u;}
}
/* L overflow-corrected and sign-extended, as after every instruction. */
AFO_IN void endi(void){agc_register l=L;L=(agc_register)((l&0137777u)|((l>>1)&040000u));}
AFO_IN agc_word cell(const afo_cell *p){return (agc_word)(*p&077777u);}

AFO_NI void afo_caf(agc_word k){A=sx(k);endi();}
AFO_NI void afo_ca(const afo_cell *p){afo_caf(cell(p));}
AFO_NI void afo_csf(agc_word k){A=sx((agc_word)(~k&077777u));endi();}
AFO_NI void afo_cs(const afo_cell *p){afo_csf(cell(p));}
AFO_NI void afo_adf(agc_word k){A=add16(A,sx(k));endi();}
AFO_NI void afo_ad(const afo_cell *p){afo_adf(cell(p));}
AFO_NI void afo_suf(agc_word k){A=add16(A,sx((agc_word)(~k&077777u)));endi();}
AFO_NI void afo_su(const afo_cell *p){afo_suf(cell(p));}
AFO_NI void afo_maskf(agc_word k){A=sx((agc_word)(oc(A)&k));endi();}
AFO_NI void afo_mask(const afo_cell *p){afo_maskf(cell(p));}
AFO_NI uint8_t afo_ts(afo_cell *p){
 agc_register acc=A,o=ovf(acc);
 *p=oc(acc);
 if(o)A=o;
 endi();return o!=0u;
}
AFO_NI void afo_xch(afo_cell *p){agc_register acc=A;A=sx(cell(p));*p=oc(acc);endi();}
AFO_NI void afo_lxch(afo_cell *p){agc_register l=L;L=sx(cell(p));*p=oc(l);endi();}
AFO_NI void afo_qxch(afo_cell *p){agc_register q=Q;Q=sx(cell(p));*p=oc(q);endi();}
AFO_NI void afo_ads(afo_cell *p){A=add16(A,sx(cell(p)));*p=oc(A);endi();}
AFO_NI void afo_incr(afo_cell *p){*p=oc(add16(1u,sx(cell(p))));endi();}
AFO_NI void afo_aug(afo_cell *p){
 agc_register v=sx(cell(p));
 *p=oc(add16((v&0100000u)?M16-1u:1u,v));endi();
}
AFO_NI void afo_dim(afo_cell *p){
 agc_register v=sx(cell(p));
 if(v!=0u&&v!=M16)*p=oc(add16((v&0100000u)?1u:M16-1u,v));
 endi();
}
AFO_NI void afo_das(afo_cell *p){
 agc_register lsw=add16(L,sx(cell(p+1))),msw=add16(A,sx(cell(p)));
 if((lsw&0140000u)==040000u)msw=add16(msw,1u);else if((lsw&0140000u)==0100000u)msw=add16(msw,M16-1u);
 A=ovf(msw);L=0;
 p[1]=oc(lsw);p[0]=oc(msw);
}
AFO_NI void afo_dca(const afo_cell *p){L=sx(cell(p+1));A=sx(cell(p));}
AFO_NI void afo_dcaf(const agc_word k[2]){L=sx(k[1]);A=sx(k[0]);}
AFO_NI void afo_dcs(const afo_cell *p){L=sx((agc_word)(~cell(p+1)&077777u));A=sx((agc_word)(~cell(p)&077777u));}
AFO_NI void afo_dcsf(const agc_word k[2]){L=sx((agc_word)(~k[1]&077777u));A=sx((agc_word)(~k[0]&077777u));}
AFO_NI void afo_dxch(afo_cell *p){
 agc_register op=sx(cell(p+1));p[1]=oc(L);L=op;
 op=sx(cell(p));p[0]=oc(A);A=op;
}
/* MP and DV: registers here, the arithmetic in afo_mpcore, afo_mul14
 * and afo_dvloop (6502 assembly below, kept in this file so every build
 * that links agc_fast_ops.c has it; the same steps in C elsewhere). */
uint32_t afo_mul14(uint16_t x,uint16_t y);
uint32_t afo_mpcore(uint16_t a,uint16_t k);
uint32_t afo_dvloop(uint16_t residual,uint16_t payload,uint16_t divisor);
#ifdef __mos__
/* The 6502 cores, value-only (the C side resolves the image).
 * llvm-mos calling convention: first 16-bit argument in A/X, then rc2/rc3,
 * rc4/rc5; a 32-bit result in A/X (low word) and rc2/rc3 (high word);
 * rc2-rc19 and Y are caller-saved. */
/* uint32_t afo_mpcore(uint16_t a, uint16_t k): MP with A = a and memory
 * operand k (k a 15-bit word other than +0 and -0); returns the new A in
 * the low word and the new L in the high word, as agc_multiply leaves them
 * (each sign-extended). Signs here, magnitudes in afo_mul14. */
__asm__(
 "\t.section .text.afo_mpcore,\"ax\",@progbits\n"
 "\t.globl afo_mpcore\n"
 "afo_mpcore:\n"
 "\tsta mos8(__rc8)\n"
 "\ttxa\n"
 "\tand #$3f\n"
 "\tsta mos8(__rc9)\n"
 "\ttxa\n"
 "\tlsr a\n"
 "\tand #$40\n"
 "\tora mos8(__rc9)\n"
 "\tsta mos8(__rc9)\n"
 "\teor mos8(__rc3)\n"
 "\tand #$40\n"
 "\tsta mos8(__rc10)\n"
 "\tbit mos8(__rc9)\n"
 "\tbvc .Lmp_opos\n"
 "\tlda mos8(__rc8)\n"
 "\teor #$ff\n"
 "\tsta mos8(__rc8)\n"
 "\tlda mos8(__rc9)\n"
 "\teor #$7f\n"
 "\tsta mos8(__rc9)\n"
 ".Lmp_opos:\n"
 "\tbit mos8(__rc3)\n"
 "\tbvc .Lmp_kpos\n"
 "\tlda mos8(__rc2)\n"
 "\teor #$ff\n"
 "\tsta mos8(__rc2)\n"
 "\tlda mos8(__rc3)\n"
 "\teor #$7f\n"
 "\tsta mos8(__rc3)\n"
 ".Lmp_kpos:\n"
 "\tlda mos8(__rc8)\n"
 "\tldx mos8(__rc9)\n"
 "\tjsr afo_mul14\n"
 "\tldy mos8(__rc10)\n"
 "\tbeq .Lmp_sx\n"
 "\teor #$ff\n"
 "\tpha\n"
 "\ttxa\n"
 "\teor #$7f\n"
 "\ttax\n"
 "\tlda mos8(__rc2)\n"
 "\teor #$ff\n"
 "\tsta mos8(__rc2)\n"
 "\tlda mos8(__rc3)\n"
 "\teor #$7f\n"
 "\tsta mos8(__rc3)\n"
 "\tpla\n"
 ".Lmp_sx:\n"
 "\tcpx #$40\n"
 "\tbcc .Lmp_ahi\n"
 "\tpha\n"
 "\ttxa\n"
 "\tora #$80\n"
 "\ttax\n"
 "\tpla\n"
 ".Lmp_ahi:\n"
 "\tldy mos8(__rc3)\n"
 "\tcpy #$40\n"
 "\tbcc .Lmp_lhi\n"
 "\ttay\n"
 "\tlda mos8(__rc3)\n"
 "\tora #$80\n"
 "\tsta mos8(__rc3)\n"
 "\ttya\n"
 ".Lmp_lhi:\n"
 "\trts\n"
);
/* uint32_t afo_mul14(uint16_t x, uint16_t y): x, y < 2^14; returns
 * p >> 14 in the low word and p & 037777 in the high word (p = x * y).
 * Shift-add over the 14 multiplier bits; the product's low bits shift
 * into the multiplier's bytes, so after 14 steps A:rc6:rc3:rc2 = p << 2. */
__asm__(
 "\t.section .text.afo_mul14,\"ax\",@progbits\n"
 "\t.globl afo_mul14\n"
 "afo_mul14:\n"
 "\tsta mos8(__rc4)\n"
 "\tstx mos8(__rc5)\n"
 "\tlda #0\n"
 "\tsta mos8(__rc6)\n"
 "\tldx #14\n"
 "\tlsr mos8(__rc3)\n"
 "\tror mos8(__rc2)\n"
 ".Lmul_step:\n"
 "\tbcc .Lmul_shift\n"
 "\ttay\n"
 "\tclc\n"
 "\tlda mos8(__rc6)\n"
 "\tadc mos8(__rc4)\n"
 "\tsta mos8(__rc6)\n"
 "\ttya\n"
 "\tadc mos8(__rc5)\n"
 ".Lmul_shift:\n"
 "\tror a\n"
 "\tror mos8(__rc6)\n"
 "\tror mos8(__rc3)\n"
 "\tror mos8(__rc2)\n"
 "\tdex\n"
 "\tbne .Lmul_step\n"
 "\ttax\n"
 "\tlsr mos8(__rc3)\n"
 "\tror mos8(__rc2)\n"
 "\tlsr mos8(__rc3)\n"
 "\tror mos8(__rc2)\n"
 "\tlda mos8(__rc6)\n"
 "\trts\n"
);
/* uint32_t afo_dvloop(uint16_t residual, uint16_t payload, uint16_t divisor):
 * the 14 quotient digits of agc_hardware_divide (payload bits 13..0 are the
 * incoming bits, divisor the divisor magnitude). Returns the quotient in the
 * low word and the final residual in the high word. Per digit, with r the
 * residual: folded = r < 0 ? (r << 1) | 0100000 | !incoming
 *                          : (r << 1) & 077776,
 * trial = folded + divisor (end-around carry), digit = sign of trial,
 * r = digit ? trial : folded. */
__asm__(
 "\t.section .text.afo_dvloop,\"ax\",@progbits\n"
 "\t.globl afo_dvloop\n"
 "afo_dvloop:\n"
 "\tsta mos8(__rc6)\n"
 "\tstx mos8(__rc7)\n"
 "\tlda #0\n"
 "\tsta mos8(__rc8)\n"
 "\tsta mos8(__rc9)\n"
 "\tasl mos8(__rc2)\n"
 "\trol mos8(__rc3)\n"
 "\tasl mos8(__rc2)\n"
 "\trol mos8(__rc3)\n"
 "\tldy #14\n"
 ".Ldv_digit:\n"
 "\tasl mos8(__rc2)\n"
 "\trol mos8(__rc3)\n"
 "\tlda mos8(__rc6)\n"
 "\tbit mos8(__rc7)\n"
 "\tbmi .Ldv_neg\n"
 "\tasl a\n"
 "\tsta mos8(__rc10)\n"
 "\tlda mos8(__rc7)\n"
 "\trol a\n"
 "\tand #$7f\n"
 "\tbpl .Ldv_add\n"
 ".Ldv_neg:\n"
 "\trol a\n"
 "\teor #1\n"
 "\tsta mos8(__rc10)\n"
 "\tlda mos8(__rc7)\n"
 "\trol a\n"
 "\tora #$80\n"
 ".Ldv_add:\n"
 "\tsta mos8(__rc11)\n"
 "\tclc\n"
 "\tlda mos8(__rc10)\n"
 "\tadc mos8(__rc4)\n"
 "\ttax\n"
 "\tlda mos8(__rc11)\n"
 "\tadc mos8(__rc5)\n"
 "\tbcc .Ldv_sum\n"
 "\tinx\n"
 "\tbne .Ldv_sum\n"
 "\tclc\n"
 "\tadc #1\n"
 ".Ldv_sum:\n"
 "\tcmp #$80\n"
 "\tbcc .Ldv_reject\n"
 "\tstx mos8(__rc6)\n"
 "\tsta mos8(__rc7)\n"
 "\tbcs .Ldv_next\n"
 ".Ldv_reject:\n"
 "\tlda mos8(__rc10)\n"
 "\tsta mos8(__rc6)\n"
 "\tlda mos8(__rc11)\n"
 "\tsta mos8(__rc7)\n"
 "\tclc\n"
 ".Ldv_next:\n"
 "\trol mos8(__rc8)\n"
 "\trol mos8(__rc9)\n"
 "\tdey\n"
 "\tbne .Ldv_digit\n"
 "\tlda mos8(__rc6)\n"
 "\tsta mos8(__rc2)\n"
 "\tlda mos8(__rc7)\n"
 "\tsta mos8(__rc3)\n"
 "\tlda mos8(__rc8)\n"
 "\tldx mos8(__rc9)\n"
 "\trts\n"
);
#else
uint32_t afo_mpcore(uint16_t a,uint16_t k){
 agc_word op=oc(a),hi,lo;uint32_t r;
 r=afo_mul14((op&040000u)?(uint16_t)(op^077777u):op,(k&040000u)?(uint16_t)(k^077777u):k);
 hi=(agc_word)r;lo=(agc_word)(r>>16);
 if((op^k)&040000u){hi^=077777u;lo^=077777u;}
 return sx(hi)|((uint32_t)sx(lo)<<16);
}
uint32_t afo_mul14(uint16_t x,uint16_t y){
 uint32_t p=(uint32_t)x*y;
 return (p>>14)|((p&037777u)<<16);
}
uint32_t afo_dvloop(uint16_t r,uint16_t payload,uint16_t d){
 uint16_t q=0,w,folded,trial;
 for(w=020000u;w;w>>=1){
  folded=(r&0100000u)?(uint16_t)((r<<1)|0100000u|((payload&w)?0u:1u)):(uint16_t)((r<<1)&077776u);
  trial=add16(folded,d);
  q=(uint16_t)(q<<1);
  if(trial&0100000u){q|=1u;r=trial;}else r=folded;
 }
 return q|((uint32_t)r<<16);
}
#endif
/* agc_multiply on A and k. */
static __attribute__((noinline)) void mp_wide(agc_word k){agc_register_pair p=agc_multiply(A,k);A=p.a;L=p.l;endi();}
AFO_NI void afo_mpf(agc_word k){
 uint32_t r;
 if(k>077777u){mp_wide(k);return;}
 if(k==0u||k==077777u){A=0;L=0;return;}
 r=afo_mpcore(A,k);A=(agc_register)r;L=(agc_register)(r>>16);
}
/* agc_hardware_divide of A,L by d. */
static void dv16(agc_register d){
 agc_register a=A,l=(agc_register)((L&0137777u)|((L>>1)&040000u)),res,low,q;uint16_t nn=(uint16_t)(a&0100000u),dn=(uint16_t)(d&0100000u);uint32_t r;
 res=nn?a:(agc_register)(a^M16);
 if(res==M16)nn=(uint16_t)(l&0100000u);
 low=add16(nn?(agc_register)(l^M16):l,040000u);
 if((low&0140000u)!=040000u)res=add16(res,1u);
 r=afo_dvloop(res,(uint16_t)(low&037777u),dn?(agc_register)(d^M16):d);
 q=(agc_register)r;res=(agc_register)(r>>16);
 if(low&0100000u)q=(agc_register)(q+0140000u);
 if(nn!=dn)q^=M16;
 A=q;L=nn?res:(agc_register)(res^M16);endi();
}
AFO_NI void afo_dvf(agc_word k){dv16(sx(k));}
AFO_NI void afo_mp(const afo_cell *p){afo_mpf(cell(p));}
AFO_NI void afo_dv(const afo_cell *p){afo_dvf(cell(p));}
static uint8_t ccs15(agc_word v){
 v&=077777u;
 if(v==0u){A=0;return 1;}
 if(v==077777u){A=0;return 3;}
 if(v&040000u){agc_word m=(agc_word)(~v&037777u);A=(agc_register)(m>1u?m-1u:0u);return 2;}
 A=(agc_register)(v>1u?v-1u:0u);return 0;
}
AFO_NI uint8_t afo_ccsf(agc_word k){uint8_t c=ccs15(k);endi();return c;}
AFO_NI uint8_t afo_ccs(const afo_cell *p){return afo_ccsf(cell(p));}
AFO_NI void afo_tc(agc_word r){Q=(agc_word)(r&077777u);endi();}
AFO_NI uint8_t afo_bzf(void){return A==0u||A==M16;}
AFO_NI uint8_t afo_bzmf(void){return A==0u||(A&0100000u)!=0u;}
