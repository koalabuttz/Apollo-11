/* SPDX-License-Identifier: MIT */
#include "cm_tvc_massprop.h"
/* Labels are the original bank-05 addresses. Every TS honours its overflow
 * skip and every CCS its four-way skip, so the overflow detours the source
 * can take with raw erasable contents follow the original words exactly. */
#define DAPDATR1 01466u
#define IXX 01470u
#define LEMMASS 01473u
#define CSMMASS 01474u
#define WEIGHTG 01475u
#define PHI333 01506u
#define PSI333 01507u
#define TEMP333 01510u
#define VARST0 01511u
#define VARST5 01516u
/* 3462 (TCF PEGGY) through DXITFIX 3520: NOLEMVAL, INTVALUE, SLOPEVAL,
 * NEGBPW and DXITFIX, plus the neighbours an index of -1 reaches. */
static const agc_word bank05[] = {
    013435u,
    000616u, 002526u, 002352u, 001471u, 000634u, 000612u, 003706u, 004425u,
    000644u, 003710u, 004246u, 002011u, 077501u, 000612u, 004656u, 010372u, 077126u, 076261u,
    000767u, 015624u, 003054u, 004532u, 010433u, 000000u, 022070u, 003204u, 077266u, 002476u,
    070364u, 075420u
};
uint8_t cm_tvc_massprop_fixed(uint16_t a, agc_word *out) {
    if (out == 0) return 0;
    if (a == 03435u) { *out = 051506u; return 1; }
    if (a < 03462u || a > 03520u) return 0;
    *out = bank05[a - 03462u];
    return 1;
}
/* INDEX adds the erasable word to the next instruction word (ones'
 * complement). The loop counters stay within +-9, so only the address field
 * moves; anything else is outside the source's reachable domain. */
static unsigned indexed(const agc_image *m, unsigned index_address, unsigned base, uint8_t *bad) {
    agc_word v = ai_word(m, index_address);
    unsigned a = agc_add_word((agc_word)base, v);
    if (agc_to_int(v) > 9 || agc_to_int(v) < -9) *bad = 1;
    return a;
}
static agc_word fixed(unsigned a, uint8_t *bad) {
    agc_word w = 0;
    if (!cm_tvc_massprop_fixed((uint16_t)a, &w)) *bad = 1;
    return w;
}
/* A loop head whose loop state repeats is a loop the source never leaves:
 * nothing either loop reads is written inside it (see docs/CM-TVC-MASSPROP.md).
 * MASSPROP's head state is A at TS PHI333; PEGGY's is the PHI333/PSI333 pair. */
#define HEADS 32u
static uint8_t seen(uint32_t *list, uint8_t *count, uint32_t key, uint8_t *bad) {
    uint8_t i;
    for (i = 0; i < *count; ++i) if (list[i] == key) return 1;
    if (*count >= HEADS) { *bad = 1; return 0; }
    list[(*count)++] = key;
    return 0;
}
enum cm_tvc_massprop_status cm_tvc_massprop(agc_image *m) {
    uint32_t massprop_heads[HEADS], peggy_heads[HEADS];
    agc_word q;
    uint8_t heads = 0, pegs = 0, bad = 0, ccs;
    unsigned slope;
    if (m->words[4] != 012000u || m->words[3] != 03000u || m->words[6] != 012006u) return CM_TVC_MASSPROP_OUT_OF_SCOPE;
    if (m->words[5] != 03366u && m->words[5] != 03422u) return CM_TVC_MASSPROP_OUT_OF_SCOPE;
    if (m->words[5] == 03422u) goto L3422;
/* MASSPROP: ten passes, PHI333 9..0, LEM-on or LEM-off breakpoints. */
    ai_caf(m, 9u);
L3367:
    if (bad) goto out;
    if (seen(massprop_heads, &heads, m->words[0], &bad)) return CM_TVC_MASSPROP_NO_EXIT;
    if (ai_ts(m, PHI333)) goto L3371;
    ai_ca(m, DAPDATR1);
L3371:
    ai_maskf(m, 010000u);
    if (ai_bzf(m)) goto L3377;                        /* EXTEND; BZF LEMYES */
    ai_caf(m, fixed(indexed(m, PHI333, 03463u, &bad), &bad));    /* NOLEMVAL */
    goto L3407;
L3377:
    ai_ca(m, LEMMASS);
    ai_double(m);
    ai_mpf(m, fixed(indexed(m, PHI333, 03505u, &bad), &bad));    /* SLOPEVAL */
    ai_ddoubl(m);
    ai_adf(m, fixed(indexed(m, PHI333, 03473u, &bad), &bad));    /* INTVALUE */
L3407:
    if (ai_ts(m, indexed(m, PHI333, VARST0, &bad))) goto L3412;
    ccs = ai_ccs(m, PHI333);
    if (ccs == 1u) goto L3413;
    if (ccs >= 2u) goto L3422;
L3412:
    goto L3367;
/* DXTEST: LEM descent stage off (DAPDATR1 negative) moves VARST0+8,9. */
L3413:
    ccs = ai_ccs(m, DAPDATR1);
    if (ccs < 2u) goto L3422;
    if (ccs == 2u) ai_dxch(m, VARST0 + 8u);
    ai_das(m, VARST0 + 1u);
    ai_caf(m, fixed(03520u, &bad));                  /* DXITFIX */
    ai_ads(m, VARST0 + 7u);
/* FIXCW: three outputs IXX, IAVG, IAVG/TLX from the slope selected by the
 * sign of the CSM weight delta. */
L3422:
    ai_caf(m, 2u);
    if (ai_ts(m, PHI333)) goto L3425;
    if (ai_ts(m, PSI333)) goto L3426;
L3425:
    ai_ca(m, CSMMASS);
L3426:
    ai_adf(m, fixed(03517u, &bad));                  /* NEGBPW */
    ai_double(m);
    if (ai_ts(m, TEMP333)) {                         /* skips EXTEND: BZMF word is AD 3435 */
        ai_adf(m, fixed(03435u, &bad));
        goto L3433;
    }
    if (ai_bzmf(m)) goto L3435;
L3433:
    ai_caf(m, 077775u);                              /* NEG2 */
    if (ai_ts(m, PHI333)) { slope = VARST5; goto L3436; }   /* skips INDEX */
L3435:
    if (bad) goto out;
    if (seen(peggy_heads, &pegs, (uint32_t)ai_word(m, PHI333) << 15 | ai_word(m, PSI333), &bad)) return CM_TVC_MASSPROP_NO_EXIT;
    slope = indexed(m, PHI333, VARST5, &bad);
L3436:
    ai_ca(m, slope);
    ai_mp(m, TEMP333);
    ai_double(m);
    ai_ad(m, indexed(m, PSI333, VARST0, &bad));
    if (ai_ts(m, indexed(m, PSI333, IXX, &bad))) goto L3447;
    ccs = ai_ccs(m, PSI333);
    if (ccs == 1u) goto L3450;
    if (ccs == 2u) goto L3451;
    if (ccs == 3u) goto L3452;
L3447:
    goto L3457;
L3450:
    ai_ca(m, DAPDATR1);
L3451:
    ai_maskf(m, 020000u);
L3452:
    ccs = ai_ccs(m, 0u);
    if (ccs == 1u) goto L3454;
    if (ccs == 2u) goto L3455;
    if (ccs == 3u) goto L3456;
    ai_ca(m, LEMMASS);
L3454:
    ai_ad(m, CSMMASS);
L3455:
    if (ai_ts(m, WEIGHTG)) goto L3457;
L3456:                                               /* ENDMASSP: TC Q */
    if (bad) goto out;
    /* TC Q runs the TC held in Q (the caller's return), leaving Q = 3. */
    q = (agc_word)(m->words[2] & 07777u);
    ai_tc(m, 3u);
    m->words[5] = q;
    return CM_TVC_MASSPROP_OK;
L3457:                                               /* BOKKEP2 */
    if (ai_ts(m, PSI333)) { ai_ads(m, PHI333); goto L3435; }   /* skips EXTEND: DIM word is ADS */
    ai_dim(m, PHI333);
    goto L3435;
out:
    return CM_TVC_MASSPROP_FAULT;
}
