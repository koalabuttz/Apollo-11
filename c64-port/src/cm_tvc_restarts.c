/* SPDX-License-Identifier: MIT */
#include "cm_tvc_restarts.h"
/* Labels are the original bank-16 addresses. Every CCS takes its four-way
 * skip; the TS words here store a CAF constant or -0 plus a word, which
 * cannot overflow, so they never skip. */
#define QRUPT 012u
#define BANKRUPT 016u
#define TIME5 030u
#define TVCYAW 053u
#define TVCPITCH 054u
#define T5LOC 01312u
#define PACTOFF 01423u
#define STROKER 01574u
#define PCMD 01623u
#define YCMD 01624u
#define TVCPHASE 01644u
#define TVCEXPHS 01651u
#define POSTJUMP 04701u
#define IBNKCALL 04740u
#define ISWCALL 04744u
#define WAITLIST 05245u
#define RESUME 05333u
#define NOQRSM 05335u
/* TVCCADR (3257) through TVCEXADR +6 (3271): the copycycle CADRs, the T5
 * GENADRs and the TVCEXECUTIVE restart points. */
static const agc_word table[] = {
    003174u, 040675u, 003205u, 041051u, 037772u,
    003003u, 003011u, 003040u, 003043u, 003121u, 003132u
};
/* EXTEND WOR on an output channel: overflow-corrected A OR the channel
 * goes to both A (sign-extended) and the channel. */
static void wor(agc_image *m, agc_word *channel) {
    agc_word v = (agc_word)((agc_correct(m->words[0]) | *channel) & 077777u);
    *channel = v;
    m->words[0] = agc_extend(v);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static enum cm_tvc_restarts_status leave(agc_image *m, agc_word z, agc_word q) {
    if (q) ai_tc(m, q);
    m->words[2] &= 077777u;                          /* canonical frontier Q */
    m->words[5] = z;
    return CM_TVC_RESTARTS_OK;
}
static uint8_t positive(agc_word v) { return v != 0u && !(v & 040000u); }
enum cm_tvc_restarts_status cm_tvc_restarts(agc_image *m, cm_tvc_restarts_io *io) {
    agc_word phase, exphs;
    uint8_t ccs;
    if (m->words[4] != 034000u || m->words[3] != 03000u || m->words[6] != 034006u) return CM_TVC_RESTARTS_OUT_OF_SCOPE;
    phase = ai_word(m, TVCPHASE);
    exphs = ai_word(m, TVCEXPHS);
    switch (m->words[5]) {
    case 03140u:
        if (exphs == 0u && positive((agc_word)(phase & 040001u)) && phase > 9u) return CM_TVC_RESTARTS_OUT_OF_SCOPE;
        goto L3140;
    case 03174u: goto L3174;
    case 03205u: goto L3205;
    case 03232u: goto L3232;
    case 03243u: goto L3243;
    case 03253u:
        if (exphs < 1u || exphs > 6u) return CM_TVC_RESTARTS_OUT_OF_SCOPE;
        goto L3253;
    default: return CM_TVC_RESTARTS_OUT_OF_SCOPE;
    }
L3140:                                               /* REDOTVC */
    ai_lxch(m, BANKRUPT);
    ai_qxch(m, QRUPT);
    ccs = ai_ccs(m, TVCEXPHS);                       /* EXECPHS */
    if (ccs == 1u) goto L3152;
    if (ccs == 3u) goto L3147;
    ai_caf(m, 011u);                                 /* NINE */
L3147:
    return leave(m, WAITLIST, 03150u);               /* 2CADR EXRSTRT */
L3152:                                               /* TVCDAPHS */
    ai_csf(m, 037776u);                              /* OCT37776 */
    ai_mask(m, TVCPHASE);
    ccs = ai_ccs(m, 0u);
    if (ccs == 0u) goto L3240;
    if (ccs == 1u) goto L3163;
    if (ccs == 3u) goto L3160;
    ai_cs(m, TVCPHASE);
L3160:
    ai_maskf(m, 2u);                                 /* BIT2 */
    if (ai_bzf(m)) goto L3244;                       /* EXTEND; BZF TRIM/CMD */
L3163:                                               /* ENABL1 */
    ai_caf(m, 0200u);                                /* BIT8 */
    ai_adf(m, 02000u);                               /* BIT11 */
    wor(m, &io->channel12);
    ai_caf(m, 03174u);                               /* TVCADDR */
    ai_ts(m, T5LOC);
    ai_caf(m, 037772u);                              /* TVCADDR +4 */
    ai_ts(m, TIME5);
    return leave(m, RESUME, 0u);
L3174:                                               /* ENABL2 */
    ai_lxch(m, BANKRUPT);
    ai_caf(m, 2u);                                   /* BIT2 */
    wor(m, &io->channel12);
    ai_caf(m, 03205u);                               /* TVCADDR +2 */
    ai_ts(m, T5LOC);
    ai_caf(m, 037776u);                              /* OCT37776 */
    ai_ts(m, TIME5);
    return leave(m, NOQRSM, 0u);
L3205:                                               /* CMDSOUT */
    ai_lxch(m, BANKRUPT);
    ai_qxch(m, QRUPT);
    ai_csf(m, 0u);                                   /* ZERO */
    ai_ad(m, PCMD);
    ai_ts(m, TVCPITCH);
    ai_csf(m, 0u);                                   /* ZERO */
    ai_ad(m, YCMD);
    ai_ts(m, TVCYAW);
    ai_caf(m, 06000u);                               /* PRIO6 */
    wor(m, &io->channel14);
    ccs = ai_ccs(m, TVCPHASE);                       /* PHSCHK2 */
    if (ccs < 2u) goto L3232;
    if (ccs == 3u) goto L3225;
    /* A holds the diminished absolute of a negative TVCPHASE: never negative. */
    if (ai_ccs(m, 0u) == 1u) return leave(m, POSTJUMP, 03227u);   /* CADR MRCLEAN */
L3225:
    ai_tc(m, 03226u);                                /* TC +3 */
    return leave(m, IBNKCALL, 03231u);               /* CADR SWICHOVR +5 */
L3232:                                               /* CHKSTRK */
    ccs = ai_ccs(m, STROKER);
    if (ccs == 1u || ccs == 3u) goto L3236;
    ai_caf(m, 0u);                                   /* TSTINITJ: ZERO */
    ai_ts(m, STROKER);
L3236:
    return leave(m, POSTJUMP, 03237u);               /* CADR TVCINIT4 */
L3240:                                               /* FINCOPY: INDEX TVCPHASE */
    ai_caf(m, table[ai_word(m, TVCPHASE)]);          /* TVCCADR */
    return leave(m, ISWCALL, 03243u);
L3243:
    goto L3163;
L3244:                                               /* TRIM/CMD */
    ai_dca(m, PACTOFF);
    ai_dxch(m, PCMD);
    goto L3163;
L3253:                                               /* EXRSTRT: INDEX TVCEXPHS */
    ai_caf(m, table[4u + ai_word(m, TVCEXPHS)]);     /* TVCEXADR */
    return leave(m, (agc_word)(m->words[0] & 07777u), 0u);   /* INDEX A; TCF 0 */
}
