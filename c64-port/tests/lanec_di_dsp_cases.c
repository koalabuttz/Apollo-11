/* SPDX-License-Identifier: MIT */
/* Cases for the DISPLAY_INTERFACE display-start entries: every entry, A with
 * and without overflow (the TS skips), flashing and non-flashing R displays,
 * every MAKECADR return SAVELOCR saves, BOTHJOBS copies into any job
 * register set (overlapping MPAC included), COPIES indices, CHKPRIO and
 * MARKPERF inputs. */
#include "lanec_suites.h"
#include "lanec_di_dsp.h"
#include "lanec_symbols.h"

#define ENTRIES 34u

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

#define E(M, X) LANEC_##M##_##X
#define ENTRY_LIST(M) { E(M,CHKPRIO), E(M,MARKPERF), E(M,GODSP), E(M,GODSP) + 1u, E(M,GODSPRET), E(M,GODSPR), \
    E(M,GODSPR) + 1u, E(M,GODSPR) + 2u, E(M,CLEANDSP), E(M,CLEANDSP) + 1u, REFLASHR_##M, E(M,REGODSP), \
    E(M,REGODSPR), E(M,CLOCPLAY), E(M,GOFLASH), E(M,GOFLASH2), E(M,PRIODSPR), E(M,PRIODSP), E(M,PRIODSP) + 1u, \
    E(M,EXDSPRET), E(M,GOPERF1), E(M,GOPERF1) + 2u, E(M,GOPERF2), E(M,GOPERF4), E(M,GOFLASHR), E(M,GODSPRS), \
    E(M,GODSPRS1), E(M,GODSPRS1) + 13u, E(M,BOTHJOBS), E(M,GOPERF1R), E(M,GOPERF1R) + 2u, E(M,GOPERF2R), \
    E(M,GOPERF4R), 0u }
#define REFLASHR_LM LANEC_LM_REFLASHR
#define REFLASHR_CM LANEC_CM_GODSPR      /* Comanche055 has no REFLASHR */
static const uint16_t entries_lm[ENTRIES] = ENTRY_LIST(LM);
static const uint16_t entries_cm[ENTRIES] = ENTRY_LIST(CM);
/* The last slot cycles through the MAKECADR return and the COPIES entries. */

static uint16_t any_a(uint32_t *r, uint32_t v)
{
    uint16_t x = (uint16_t)lanec_rand(r);
    switch (v % 4u) {
    case 0: return (uint16_t)(0040000u | (x & 037777u));          /* positive overflow */
    case 1: return (uint16_t)(0100000u | (x & 037777u));          /* negative overflow */
    default: return lanec_a16((uint16_t)(x & 077777u));
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, e = (uint16_t)(n % ENTRIES), z, mpac = LANEC_LM_MPAC;
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES / 2u;
    uint16_t chk, pd, ex, sk, rq, sl, bj, g2;
    lanec_fill(s, io, n);
    m->mission = (n / ENTRIES) & 1u ? LANEC_CM : LANEC_LM;
    if (m->mission == LANEC_LM) {
        z = entries_lm[e]; chk = E(LM,CHKPRIO); pd = E(LM,PRIODSP); ex = E(LM,EXDSPRET); sk = E(LM,SKIPADD);
        rq = E(LM,RELINTQ); sl = E(LM,SAVELOCS); bj = E(LM,BOTHJOBS); g2 = E(LM,GOFLASH2);
    } else {
        z = entries_cm[e]; chk = E(CM,CHKPRIO); pd = E(CM,PRIODSP); ex = E(CM,EXDSPRET); sk = E(CM,SKIPADD);
        rq = E(CM,RELINTQ); sl = E(CM,SAVELOCS); bj = E(CM,BOTHJOBS); g2 = E(CM,GOFLASH2);
    }
    w[LANEC_A] = any_a(&r, v);
    if ((v / 4u) & 1u) w[mpac + 4u] &= (uint16_t)~010u;                 /* PLAYTEM4 BIT4 clear */
    else w[mpac + 4u] |= 010u;
    if (e == ENTRIES - 1u) {
        uint16_t pick = (uint16_t)(v % 6u), copynorm = m->mission == LANEC_LM ? E(LM,COPYNORM) : E(CM,COPYNORM);
        if (pick < 3u) {                                                /* MAKECADR returns to SAVELOCS +7 */
            z = (uint16_t)(sl + 7u);
            w[LANEC_L] = pick == 0 ? (uint16_t)(g2 + 2u) : pick == 1 ? (uint16_t)(bj + 1u) : (uint16_t)(bj + 12u);
            w[LANEC_LM_LOCCTR] = (v / 6u) % 3u == 0 ? (uint16_t)(lanec_rand(&r) % 5u)
                               : (uint16_t)(lanec_rand(&r) % (01376u - (mpac + 4u) + 1u));
        } else {                                                        /* COPYNORM, COPIES, COPIES2 */
            z = (uint16_t)(copynorm + pick - 3u);
            w[LANEC_Q] = (uint16_t)(04000u + lanec_rand(&r) % 04000u);
            w[LANEC_LM_LOC] = (uint16_t)(lanec_rand(&r) % 0301u);
            if (pick == 4u) w[LANEC_A] = (uint16_t)((v & 1u ? 0040000u : 0u) | (lanec_rand(&r) % 0301u)); /* +overflow too */
            if ((v / 6u) & 1u) w[mpac + 4u] &= (uint16_t)~050u;
        }
    }
    if (e == 0u && (v / 8u) & 1u) w[LANEC_LM_FLAGWRD4] &= (uint16_t)~024100u;   /* CHKPRIO: no PRIO */
    lanec_enter(s, io, 010u, z);
    range(m, 010u, chk, (uint16_t)(pd + 2u));
    range(m, 010u, ex, (uint16_t)(sk + 3u));
    range(m, 010u, rq, (uint16_t)(rq + 1u));
    range(m, LANEC_ANY_BANK, 1u, 2u);
}

const lanec_suite lanec_suite_di_dsp = { "di-dsp", ENTRIES * 2u * 48u, make, lanec_di_dsp_dispatch };
