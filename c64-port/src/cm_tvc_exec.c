/* SPDX-License-Identifier: MIT */
#include "cm_tvc_exec.h"
/* Labels are the original bank-16 addresses. Every TS honours its overflow
 * skip and every CCS its four-way skip; a skip over EXTEND runs the next
 * word as the basic instruction its bits name, as the original does. */
#define CDUX 032u
#define FLAGWRD6 0102u
#define PACTOFF 01423u
#define YACTOFF 01424u
#define V97VCNTR 01444u
#define TEMPDAP 01445u
#define CNTR 01447u
#define OGAD 01450u
#define CSMMASS 01474u
#define DAPDATR1 01466u
#define AK 01476u
#define DELPBAR 01601u
#define DELYBAR 01603u
#define PDELOFF 01605u
#define YDELOFF 01607u
#define TTMP1 01611u
#define TENMDOT 01637u
#define REPFRAC 01642u
#define VCNTR 01643u
#define TVCEXPHS 01651u
#define MASSTMP 01652u
#define VCNTRTMP 01653u
#define OGANOW 01662u
#define OGAPAST 01663u
#define OGAERR 01664u
#define PACTTMP 01672u
#define YACTTMP 01674u
#define CNTRTMP 01676u
#define WAITLIST 05245u
#define IBNKCALL 04740u
#define TASKOVER 05324u
/* EXTEND MSU on an ordinary erasable word: two's-complement difference of
 * the 15-bit operands, sign-extended, minus one when negative. */
static void msu(agc_image *m, unsigned k) {
    unsigned a = agc_correct(m->words[0]) & 077777u, b = ~(unsigned)ai_word(m, k) & 077777u, d = a + b + 1u;
    if (d & 040000u) d = (d | 0100000u) - 1u;
    m->words[0] = agc_extend((agc_word)(d & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
/* EXTEND RAND 11 (DSALMOUT): overflow-corrected A AND the channel. */
static void rand11(agc_image *m, agc_word channel) {
    m->words[0] = agc_extend((agc_word)(agc_correct(m->words[0]) & channel & 077777u));
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static enum cm_tvc_exec_status leave(agc_image *m, agc_word z, agc_word q) {
    if (q) ai_tc(m, q);
    m->words[5] = z;
    return CM_TVC_EXEC_OK;
}
enum cm_tvc_exec_status cm_tvc_exec(agc_image *m, agc_word ch11) {
    uint8_t ccs;
    if (m->words[4] != 034000u || m->words[3] != 03000u || m->words[6] != 034006u) return CM_TVC_EXEC_OUT_OF_SCOPE;
    switch (m->words[5]) {
    case 02720u: goto L2720;
    case 02730u: goto L2730;
    case 02746u: goto L2746;
    case 02750u: goto L2750;
    case 02773u: goto L2773;
    case 02775u: goto L2775;
    case 03003u: goto L3003;
    case 03011u: goto L3011;
    case 03040u: goto L3040;
    case 03043u: goto L3043;
    case 03121u: goto L3121;
    case 03132u: goto L3132;
    default: return CM_TVC_EXEC_OUT_OF_SCOPE;
    }
L2720:                                          /* TVCEXEC */
    ai_cs(m, FLAGWRD6);
    ai_maskf(m, 060000u);                       /* OCT60000 */
    if (ai_bzmf(m)) goto L3134;
    ai_caf(m, 062u);                            /* .5SEC */
    return leave(m, WAITLIST, 02726u);          /* 2CADR TVCEXEC */
L2730:                                          /* ROLLPREP */
    ai_ca(m, CDUX);
    ai_xch(m, OGANOW);
    ai_xch(m, OGAPAST);
    ai_ca(m, OGAD);
    msu(m, OGANOW);
    if (ai_ts(m, AK)) { ai_maskf(m, 057777u); goto L2741; }   /* skips EXTEND: MP word is MASK -BIT14 */
    ai_mpf(m, 057777u);                         /* -BIT14 */
L2741:
    if (ai_ts(m, OGAERR)) goto L2743;
    ai_caf(m, 3u);                              /* THREE */
L2743:
    return leave(m, WAITLIST, 02744u);          /* 2CADR ROLLDAP */
L2746:                                          /* NEEDLEUP */
    return leave(m, IBNKCALL, 02747u);          /* CADR NEEDLER */
L2750:                                          /* VARGAINS */
    ai_caf(m, 010000u);                         /* BIT13 */
    rand11(m, ch11);
    ccs = ai_ccs(m, 0u);
    if (ccs == 0u) goto L2760;
    if (ccs == 2u) goto L2756;
    if (ccs == 3u) goto L2757;
L2755:
    ai_caf(m, 2u);                              /* TWO */
L2756:
    if (ai_ts(m, TVCEXPHS)) goto L2760;
L2757:
    goto L3011;
L2760:
    ccs = ai_ccs(m, VCNTR);
    if (ccs == 1u) goto L2771;
    if (ccs == 2u) return CM_TVC_EXEC_NO_EXIT;  /* TCF +0 */
    if (ccs == 3u) goto L2755;
    if (ai_ts(m, VCNTRTMP)) goto L2767;
    ai_ca(m, CSMMASS);
L2767:
    if (ai_ts(m, MASSTMP)) goto L2771;
    goto L3002;
L2771:                                          /* GAINCHNG */
    return leave(m, IBNKCALL, 02772u);          /* CADR FIXCW */
L2773:
    return leave(m, IBNKCALL, 02774u);          /* CADR S40.15 */
L2775:
    ai_cs(m, TENMDOT);
    ai_ad(m, CSMMASS);
    if (ai_ts(m, MASSTMP)) goto L3001;
    ai_caf(m, 023u);                            /* NINETEEN */
L3001:
    if (ai_ts(m, VCNTRTMP)) goto L3003;
L3002:                                          /* EXECCOPY */
    ai_incr(m, TVCEXPHS);
L3003:
    ai_ca(m, MASSTMP);
    if (ai_ts(m, CSMMASS)) goto L3006;
    ai_ca(m, VCNTRTMP);
L3006:
    if (ai_ts(m, VCNTR)) goto L3010;
    if (ai_ts(m, V97VCNTR)) goto L3011;
L3010:
    ai_incr(m, TVCEXPHS);
L3011:                                          /* 1SHOTCHK */
    ccs = ai_ccs(m, CNTR);
    if (ccs == 1u || ccs == 3u) goto L3031;
    if (ccs == 2u) goto L3022;
    if (ai_ts(m, CNTRTMP)) goto L3020;
    ai_caf(m, 6u);                              /* SIX */
L3020:
    if (ai_ts(m, TVCEXPHS)) goto L3022;
    goto L3132;
L3022:                                          /* REPCHEK */
    ai_ca(m, REPFRAC);
    if (ai_bzmf(m)) goto L3134;
    if (ai_ts(m, TEMPDAP + 1u)) goto L3027;
    ai_caf(m, 4u);                              /* FOUR */
L3027:
    if (ai_ts(m, TVCEXPHS)) goto L3031;
    goto L3043;
L3031:                                          /* 1SHOTOK */
    ai_caf(m, 010000u);                         /* BIT13 */
    rand11(m, ch11);
    ccs = ai_ccs(m, 0u);
    if (ccs == 1u) goto L3134;                  /* engine off: TCF TVCEXFIN */
    if (ccs == 3u) goto L3040;
    ai_incr(m, TVCEXPHS);
L3040:                                          /* TEMPSET */
    ai_caf(m, 010000u);                         /* FCORFRAC */
    if (ai_ts(m, TEMPDAP + 1u)) goto L3043;
    ai_incr(m, TVCEXPHS);
L3043:                                          /* CORSETUP */
    ai_ca(m, DAPDATR1);
    ai_maskf(m, 010000u);                       /* BIT13 */
    if (ai_bzf(m)) goto L3050;
    ai_ca(m, TEMPDAP + 1u);
L3050:
    ai_ad(m, TEMPDAP + 1u);
    if (ai_ts(m, TEMPDAP)) goto L3053;
    ai_caf(m, 077776u);                         /* NEGONE */
L3053:
    if (ai_ts(m, CNTRTMP)) { ai_ca(m, PDELOFF + 1u); goto L3056; }   /* skips EXTEND: DCA word is CA */
    ai_dca(m, PDELOFF);                         /* CG.CORR: pitch */
L3056:
    ai_dxch(m, PACTTMP);
    ai_dcs(m, PDELOFF);
    ai_ddoubl(m);
    ai_ddoubl(m);
    ai_dxch(m, TTMP1);
    ai_dca(m, DELPBAR);
    ai_ddoubl(m);
    ai_ddoubl(m);
    ai_das(m, TTMP1);
    ai_dca(m, TTMP1);
    ai_mp(m, TEMPDAP);
    ai_das(m, PACTTMP);
    ai_dca(m, YDELOFF);                         /* yaw */
    ai_dxch(m, YACTTMP);
    ai_dcs(m, YDELOFF);
    ai_ddoubl(m);
    ai_ddoubl(m);
    ai_dxch(m, TTMP1);
    ai_dca(m, DELYBAR);
    ai_ddoubl(m);
    ai_ddoubl(m);
    ai_das(m, TTMP1);
    ai_dca(m, TTMP1);
    ai_mp(m, TEMPDAP);
    ai_das(m, YACTTMP);
    ai_incr(m, TVCEXPHS);                       /* CORCOPY */
L3121:
    ai_dca(m, PACTTMP);
    if (ai_ts(m, PACTOFF)) goto L3125;
    ai_dxch(m, PDELOFF);
L3125:
    ai_dca(m, YACTTMP);
    if (ai_ts(m, YACTOFF)) goto L3131;
    ai_dxch(m, YDELOFF);
L3131:
    ai_incr(m, TVCEXPHS);
L3132:                                          /* CNTRCOPY */
    ai_ca(m, CNTRTMP);
    if (ai_ts(m, CNTR)) goto L3135;
L3134:                                          /* TVCEXFIN */
    ai_caf(m, 0u);                              /* ZERO */
L3135:
    if (ai_ts(m, TVCEXPHS)) return CM_TVC_EXEC_FAULT;
    return leave(m, TASKOVER, 0u);
}
