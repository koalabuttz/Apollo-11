/* SPDX-License-Identifier: MIT */
#include "cm_tvc_stroke.h"
/* Labels are the original bank-20 addresses. Every CCS takes its four-way
 * skip and the one TS that can overflow (CADDY after DOUBLE) its skip; the
 * other TS words store constants, CCS results or a negated word. */
#define TVCPITCH 054u
#define ESTROKER 01413u
#define DAPDATR1 01466u
#define STROKER 01574u
#define CADDY 01655u
#define N 01656u
#define BUNKER 01657u
#define REVS 01660u
#define CARD 01661u
#define ENDOFJOB 05217u
#define WAITLIST 05245u
/* FCARD (3524) through FCARD6 (3532): the waveform's REVS and N steps. */
static const agc_word fcard[] = { 04u, 05u, 011u, 015u, 06u, 05u, 04u };
/* EXTEND WOR on an output channel: overflow-corrected A OR the channel
 * goes to both A (sign-extended) and the channel. */
static void wor(agc_image *m, agc_word *channel) {
    agc_word v = (agc_word)((agc_correct(m->words[0]) | *channel) & 077777u);
    *channel = v;
    m->words[0] = agc_extend(v);
    m->words[1] = agc_extend(agc_correct(m->words[1]));
}
static enum cm_tvc_stroke_status leave(agc_image *m, agc_word z, agc_word q) {
    if (q) ai_tc(m, q);
    m->words[2] &= 077777u;                          /* canonical frontier Q */
    m->words[5] = z;
    return CM_TVC_STROKE_OK;
}
/* INDEX CARD on FCARD +4 / FCARD +7 with CARD -4..-1. */
static agc_word card(const agc_image *m, unsigned base) {
    return fcard[base - 4u + (unsigned)(agc_to_int(ai_word(m, CARD)) + 4)];
}
enum cm_tvc_stroke_status cm_tvc_stroke(agc_image *m, cm_tvc_stroke_io *io) {
    agc_word c;
    uint8_t ccs;
    if (m->words[4] != 040000u || m->words[3] != 03000u || m->words[6] != 040006u) return CM_TVC_STROKE_OUT_OF_SCOPE;
    c = ai_word(m, CARD);
    switch (m->words[5]) {
    case 03501u: goto L3501;
    case 03533u: case 03541u: case 03542u:
        if (c < 077773u || c > 077776u) return CM_TVC_STROKE_OUT_OF_SCOPE;
        if (m->words[5] == 03533u) goto L3533;
        if (m->words[5] == 03541u) goto L3544;       /* TCF +3 */
        goto L3542;
    default: return CM_TVC_STROKE_OUT_OF_SCOPE;
    }
L3501:                                               /* STRKTSTI: TCR TSTINIT */
    ai_tc(m, 03502u);
    ai_csf(m, 012u);                                 /* FCADDY */
    ai_ts(m, CADDY);
    ai_ts(m, N);
    ai_caf(m, 3u);                                   /* FREVS */
    ai_ts(m, REVS);
    ai_csf(m, 4u);                                   /* FCARD */
    ai_ts(m, CARD);
    ai_tc(m, 3u);                                    /* TC Q runs TC STRKCHK */
    io->interrupt_enabled = 0u;                      /* STRKCHK: INHINT */
    ai_ca(m, DAPDATR1);
    ai_maskf(m, 020000u);                            /* BIT14 */
    if (ai_bzf(m)) return leave(m, ENDOFJOB, 0u);    /* EXTEND; BZF +3 */
    ai_ca(m, ESTROKER);
    ai_ts(m, STROKER);                               /* a loaded word: never skips */
    return leave(m, ENDOFJOB, 0u);
L3533:                                               /* HACK */
    ai_qxch(m, BUNKER);
    ai_caf(m, 2u);                                   /* 20MS */
    return leave(m, WAITLIST, 03537u);               /* 2CADR HACKWLST */
L3542:                                               /* HACKWLST */
    ai_caf(m, 05324u);                               /* TCTSKOVR */
    ai_ts(m, BUNKER);
L3544:
    ai_ca(m, STROKER);
    ai_ads(m, TVCPITCH);
    ai_caf(m, 02000u);                               /* BIT11 */
    wor(m, &io->channel14);
    ai_incr(m, CADDY);
    ai_cs(m, CADDY);
    if (!ai_bzmf(m)) return leave(m, BUNKER, 03556u);   /* EXTEND; BZMF +2; TC BUNKER */
    ccs = ai_ccs(m, REVS);
    if (ccs == 0u) goto L3577;
    if (ccs == 1u) goto L3603;
    if (ccs == 2u) ai_incr(m, CARD);
    ai_cs(m, CARD);
    if (ai_bzf(m)) goto L3575;                       /* EXTEND; BZF STROKILL */
    ai_caf(m, card(m, 4u));                          /* INDEX CARD; FCARD +4 */
    ai_ts(m, REVS);
    ai_csf(m, card(m, 7u));                          /* INDEX CARD; FCARD +7 */
    ai_ts(m, N);
    ai_ts(m, CADDY);
    return leave(m, BUNKER, 03575u);
L3575:                                               /* STROKILL */
    ai_ts(m, STROKER);
    return leave(m, BUNKER, 03577u);
L3577:                                               /* REVUP */
    ai_ts(m, REVS);
    ai_ca(m, N);
    ai_double(m);
    goto L3606;                                      /* TCF +4 */
L3603:
    ai_csf(m, 1u);                                   /* ONE */
    ai_ts(m, REVS);
    ai_ca(m, N);
L3606:
    if (ai_ts(m, CADDY)) goto L3610;
    ai_cs(m, STROKER);
L3610:
    ai_ts(m, STROKER);
    return leave(m, BUNKER, 03612u);
}
