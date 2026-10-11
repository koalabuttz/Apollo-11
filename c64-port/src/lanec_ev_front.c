/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS front end, Luminary099 and Comanche055: the extended verb
 * fan (GOEXTVB, LST2FAN), the display-system grab and release (TESTXACT,
 * SETXTACT, XACT0, XACTALM) and the operator-error exits (ALM/END, GOPIN,
 * LM CHKPOOH). See docs/LANEC-EV-FRONT.md for the source map. */
#include "lanec_ev_front.h"
#include "lanec_symbols.h"

typedef struct { uint8_t tcf; uint16_t target; } fan_entry;
static const fan_entry fan_lm[60] = LANEC_LM_LST2FAN_TABLE;
static const fan_entry fan_cm[60] = LANEC_CM_LST2FAN_TABLE;

/* Addresses common to both missions' bank 43 layouts, checked at compile time. */
#define GOEXTVB LANEC_LM_GOEXTVB
#define LST2FAN LANEC_LM_LST2FAN
#define TESTXACT LANEC_LM_TESTXACT
#define SETXTACT LANEC_LM_SETXTACT
#define XACTALM LANEC_LM_XACTALM
#define XACT0 LANEC_LM_XACT0
#define ALMEND LANEC_LM_ALM_S_END
#define GOPIN LANEC_LM_GOPIN
#define MPAC LANEC_LM_MPAC
#define EXTVBACT LANEC_LM_EXTVBACT
#define FLAGWRD4 LANEC_LM_FLAGWRD4
typedef char same_layout[(LANEC_CM_GOEXTVB == GOEXTVB && LANEC_CM_LST2FAN == LST2FAN &&
    LANEC_CM_TESTXACT == TESTXACT && LANEC_CM_SETXTACT == SETXTACT && LANEC_CM_XACTALM == XACTALM &&
    LANEC_CM_XACT0 == XACT0 && LANEC_CM_ALM_S_END == ALMEND && LANEC_CM_GOPIN == GOPIN &&
    LANEC_CM_MPAC == MPAC && LANEC_CM_EXTVBACT == EXTVBACT && LANEC_CM_FLAGWRD4 == FLAGWRD4 &&
    LANEC_LM_GOEXTVB_BANK == 043u && LANEC_CM_GOEXTVB_BANK == 043u) ? 1 : -1];

#define OC24100 024100u
#define OCT24 024u
#define TWO 2u

static uint16_t falton(enum lanec_mission m) { return m == LANEC_LM ? LANEC_LM_FALTON : LANEC_CM_FALTON; }
static uint16_t nvsub(enum lanec_mission m) { return m == LANEC_LM ? LANEC_LM_NVSUB : LANEC_CM_NVSUB; }
static uint16_t postjump(enum lanec_mission m) { return m == LANEC_LM ? LANEC_LM_POSTJUMP : LANEC_CM_POSTJUMP; }
static uint16_t endext(enum lanec_mission m) { return m == LANEC_LM ? LANEC_LM_ENDEXT : LANEC_CM_ENDEXT; }

static uint8_t in_bank43(const apollo_overlay_state *s, const lanec_io *io)
{
    return (uint8_t)(lanec_registers_valid(s) && lanec_current_bank(s, io) == 043u);
}

/* ALM/END: TC FALTON (returns to GOPIN). */
static void alm_end(apollo_overlay_state *s, enum lanec_mission m)
{
    lanec_tc(s, falton(m), GOPIN);
}

/* SETXTACT +0 TS EXTVBACT, +1 CA Q, +2 TS MPAC +1, +3 CS TWO, +4 TC NVSUB,
 * entered at +0 or +1 with A as left by the caller. A TS overflow skips the
 * next instruction, exactly as the source does. */
static void setxtact(apollo_overlay_state *s, enum lanec_mission m, uint8_t start)
{
    uint16_t *w = s->words;
    if (start == 1 || !lanec_ts(s, EXTVBACT)) w[LANEC_A] = w[LANEC_Q];  /* CA Q */
    if (!lanec_ts(s, MPAC + 1u)) w[LANEC_A] = (uint16_t)~TWO;           /* CS TWO */
    lanec_tc(s, nvsub(m), SETXTACT + 5u);
}

enum lanec_status lanec_ev_goextvb(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, verb = w[MPAC];
    const fan_entry *e;
    if (!in_bank43(s, io) || w[LANEC_Z] != GOEXTVB || verb > 59u) return LANEC_OUT_OF_SCOPE;
    e = (m == LANEC_LM ? fan_lm : fan_cm) + verb;
    /* INDEX MPAC, TC LST2FAN: Q = GOEXTVB +2, Z = LST2FAN + verb. */
    lanec_tc(s, (uint16_t)(LST2FAN + verb), GOEXTVB + 2u);
    /* VERB69 is the label of its own fan slot: the fan stops at its entry. */
    if (e->target == LST2FAN + verb) return LANEC_OK;
    if (e->tcf) w[LANEC_Z] = e->target;
    else lanec_tc(s, e->target, (uint16_t)(LST2FAN + verb + 1u));
    return LANEC_OK;
}

enum lanec_status lanec_ev_testxact(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    /* TESTXACT +0 CCS EXTVBACT, +1 TC ALM/END, +2 CA FLAGWRD4, +3 MASK OC24100,
     * +4 CCS A, +5 TC ALM/END, +6 CAF OCT24, +7 SETXTACT. */
    uint16_t *w = s->words;
    uint8_t skip;
    if (!in_bank43(s, io) || w[LANEC_Z] != TESTXACT) return LANEC_OUT_OF_SCOPE;
    skip = lanec_ccs(s, w[EXTVBACT]);
    if (skip == 0) {                                   /* busy */
        lanec_tc(s, ALMEND, TESTXACT + 2u);
        alm_end(s, m);
        return LANEC_OK;
    }
    if (skip == 1) w[LANEC_A] = lanec_a16(w[FLAGWRD4]);
    if (skip <= 2) lanec_mask(s, OC24100);
    switch (lanec_ccs_a(s)) {
    case 0:                                            /* priority display owns DSKY */
        lanec_tc(s, ALMEND, TESTXACT + 6u);
        alm_end(s, m);
        return LANEC_OK;
    case 1:
        w[LANEC_A] = OCT24;                            /* CAF OCT24 */
        setxtact(s, m, 0);
        return LANEC_OK;
    case 2:
        setxtact(s, m, 0);
        return LANEC_OK;
    default:
        setxtact(s, m, 1);
        return LANEC_OK;
    }
}

enum lanec_status lanec_ev_setxtact(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    if (!in_bank43(s, io) || s->words[LANEC_Z] != SETXTACT) return LANEC_OUT_OF_SCOPE;
    setxtact(s, m, 0);
    return LANEC_OK;
}

enum lanec_status lanec_ev_xact0(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    if (!in_bank43(s, io) || s->words[LANEC_Z] != XACT0) return LANEC_OUT_OF_SCOPE;
    s->words[LANEC_A] = 0;                             /* CAF ZERO */
    lanec_tc(s, SETXTACT, XACT0 + 2u);
    setxtact(s, m, 0);
    return LANEC_OK;
}

/* Return from NVSUB to SETXTACT +5 (busy: TC +1) or +6, then TC MPAC +1:
 * the word SETXTACT saved there is its caller's return, a TC to fixed
 * memory, which hands control back to that caller with Q = MPAC +2. */
enum lanec_status lanec_ev_setxtact_return(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words, back = w[MPAC + 1u];
    (void)m;
    if (!in_bank43(s, io) || (w[LANEC_Z] != SETXTACT + 5u && w[LANEC_Z] != SETXTACT + 6u) ||
        back < 02000u || back > 07777u)
        return LANEC_OUT_OF_SCOPE;
    if (w[LANEC_Z] == SETXTACT + 5u) lanec_tc(s, SETXTACT + 6u, SETXTACT + 6u);
    lanec_tc(s, MPAC + 1u, SETXTACT + 7u);
    lanec_tc(s, back, MPAC + 2u);
    return LANEC_OK;
}

enum lanec_status lanec_ev_xactalm(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t z = s->words[LANEC_Z];
    if (!in_bank43(s, io) || (z != XACTALM && z != XACTALM + 1u)) return LANEC_OUT_OF_SCOPE;
    if (z == XACTALM) lanec_tc(s, falton(m), XACTALM + 1u);   /* TC FALTON */
    else lanec_tc(s, endext(m), XACTALM + 2u);                /* FALTON returned: TC ENDEXT */
    return LANEC_OK;
}

enum lanec_status lanec_ev_alm_end(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t z = s->words[LANEC_Z];
    if (!in_bank43(s, io) || (z != ALMEND && z != GOPIN)) return LANEC_OUT_OF_SCOPE;
    if (z == ALMEND) alm_end(s, m);
    else lanec_tc(s, postjump(m), GOPIN + 1u);  /* GOPIN: TC POSTJUMP, CADR PINBRNCH */
    return LANEC_OK;
}

/* LM CHKPOOH: CA MODREG, EXTEND, BZF TCQ, TC ALM/END. */
enum lanec_status lanec_ev_chkpooh(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t *w = s->words;
    if (m != LANEC_LM || !in_bank43(s, io) || w[LANEC_Z] != LANEC_LM_CHKPOOH) return LANEC_OUT_OF_SCOPE;
    w[LANEC_A] = lanec_a16(w[LANEC_LM_MODREG]);
    if (w[LANEC_A] == 0 || w[LANEC_A] == 0177777u) { w[LANEC_Z] = LANEC_LM_TCQ; return LANEC_OK; }
    lanec_tc(s, ALMEND, LANEC_LM_CHKPOOH + 4u);
    alm_end(s, m);
    return LANEC_OK;
}

enum lanec_status lanec_ev_front_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t z = s->words[LANEC_Z];
    if (z == GOEXTVB) return lanec_ev_goextvb(s, io, m);
    if (z == TESTXACT) return lanec_ev_testxact(s, io, m);
    if (z == SETXTACT) return lanec_ev_setxtact(s, io, m);
    if (z == XACT0) return lanec_ev_xact0(s, io, m);
    if (z == SETXTACT + 5u || z == SETXTACT + 6u) return lanec_ev_setxtact_return(s, io, m);
    if (z == XACTALM || z == XACTALM + 1u) return lanec_ev_xactalm(s, io, m);
    if (z == ALMEND || z == GOPIN) return lanec_ev_alm_end(s, io, m);
    if (m == LANEC_LM && z == LANEC_LM_CHKPOOH) return lanec_ev_chkpooh(s, io, m);
    return LANEC_OUT_OF_SCOPE;
}
