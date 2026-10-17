/* SPDX-License-Identifier: MIT */
#include "lm_interbank_dap.h"
#include "p66_bias.h"
#include "agc_image_w.h"
/* High byte of word i (both targets are little-endian). */
#define HI(i) ((uint8_t *)&W[i])[1]
/* A or L through BUF2: overflow-corrected, then sign-extended again (bit
 * 15 copied to bit 14). */
#define NORM(i) (HI(i)=(uint8_t)((HI(i)&0277u)|((HI(i)>>1)&0100u)))
/* IBNKCALL (Z 04674): the CADR at Q (erasable, or the caller's compiled
 * operand) goes to FB, A and L pass through BUF2 (072, 073), and the return
 * is ISWRETRN with Q+1 and the old FB in BUF2. ISWRETRN (Z 04707): back to
 * BUF2's return address under its bank. Same results as lm_interbank's two
 * routines in the CPU state they accept (its context(): EB, FB and BB
 * agree, superbank at most 7, interrupts 0 or 1, Z18 word 7 clear); any
 * other state is out of scope. */
static enum exec_status run(const agc_native_program *p,apollo_overlay_state *s,void *v){const p66_bias_io *io=v;const agc_site_operand *o;uint16_t n,at;agc_word q,t;
 /* Reached only through the transfer engine (rk_direct), which passes
  * the program, the image and rk_device: no NULL checks (124 bytes). */
 if(W[7]||(W[3]&~03400u)||(W[4]&~076000u)||W[6]!=(W[4]|(W[3]>>8))||(io->superbank&~0160u)||io->interrupt_enabled>1u)return EXEC_OUT_OF_SCOPE;
 q=W[2];
 if(W[5]==04674u){
  if(q<0100u||q>07775u)return EXEC_OUT_OF_SCOPE;
  if(q<01400u)t=W[q];
  else if(q<02000u){if(W[3]+(q&0377u)<0100u)return EXEC_OUT_OF_SCOPE;t=W[W[3]+(q&0377u)];}
  else{ /* The caller's CADR: a short scan of the module's few sites. */
   at=agc_fixed_physical(q,W[4],io->superbank);
   for (o = p->operands, n = AGC_SITES_PAIRS(p->operand_count); n && o->physical != at; --n, ++o) {
   }
   if(!n||(t=o->word)>AGC_MASK)return EXEC_OUT_OF_SCOPE;}
  W[072]=(agc_word)(q+1u);W[2]=04707u;W[5]=(agc_word)(02000u+(t&01777u));
 }else{
  if(W[5]!=04707u||W[072]<0100u||W[072]>07777u)return EXEC_OUT_OF_SCOPE;
  t=W[073];W[2]=073u;W[5]=W[072];
 }
 W[073]=W[4];NORM(0);NORM(1);
 W[4]=(agc_word)(t&076000u);W[6]=(agc_word)(W[4]|(W[3]>>8));return EXEC_OK;
}
/* POSTJUMP (Z 04635), for the self-check idle loop's CHECKNJ: the CADR at Q
 * goes to FB and Z, its low ten bits to Q; A passes through, L through
 * BUF2's normalisation. Same results as lm_interbank_routines' POSTJUMP in
 * the state it accepts (the guard above, and Q a TC caller's). */
static enum exec_status postjump(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    const p66_bias_io *io = v;
    const agc_site_operand *o;
    uint16_t n, at;
    agc_word q, t;
    if (W[5] != 04635u || W[7] || (W[3] & ~03400u) || (W[4] & ~076000u) ||
        W[6] != (W[4] | (W[3] >> 8)) || (io->superbank & ~0160u) || io->interrupt_enabled > 1u)
        return EXEC_OUT_OF_SCOPE;
    q = W[2];
    if (q < 0100u || q > 07775u) return EXEC_OUT_OF_SCOPE;
    if (q < 01400u)
        t = W[q];
    else if (q < 02000u) {
        if (W[3] + (q & 0377u) < 0100u) return EXEC_OUT_OF_SCOPE;
        t = W[W[3] + (q & 0377u)];
    } else {
        at = agc_fixed_physical(q, W[4], io->superbank);
        for (o = p->operands, n = AGC_SITES_PAIRS(p->operand_count); n && o->physical != at;
             --n, ++o) {
        }
        if (!n || (t = o->word) > AGC_MASK) return EXEC_OUT_OF_SCOPE;
    }
    NORM(1);
    W[4] = (agc_word)(t & 076000u);
    W[6] = (agc_word)(W[4] | (W[3] >> 8));
    W[2] = (agc_word)(t & 01777u);
    W[5] = (agc_word)(02000u + W[2]);
    return EXEC_OK;
}
#define AT(z) (uint16_t)(2048u+((z)-04000u))
static const agc_routine_entry entries[2] = {{AT(04674), run}, {AT(04707), run}},
                               pj[1] = {{AT(04635), postjump}};
const agc_routine_table lm_interbank_dap_hot_routines = {entries, 2},
                        lm_interbank_postjump_hot_routines = {pj, 1};
