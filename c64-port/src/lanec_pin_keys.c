/* SPDX-License-Identifier: MIT */
/* Pinball CHARIN and its key dispatch, Luminary099 and Comanche055. See
 * docs/LANEC-PIN-KEYS.md. */
#include "lanec_pin_keys.h"
#include "lanec_symbols.h"
#include "lanec_pw_runs.h"

typedef enum lanec_status (*key_run)(apollo_overlay_state *, lanec_io *, enum lanec_mission);
enum { ALRM, NUM, T89, NUM0, VERB, ERROR, REL, POS, NEG, ENTJ, CLEAR, NOUN, LABELS };
/* CHARIN2 +4 TC +1 jumps to the table word of the key: TC <label>. */
static const uint8_t group[32] = { ALRM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, T89, T89, ALRM, ALRM, ALRM, ALRM, ALRM, ALRM,
                                   NUM0, VERB, ERROR, ALRM, ALRM, ALRM, ALRM, ALRM, ALRM, REL, POS, NEG, ENTJ, ALRM,
                                   CLEAR, NOUN };
typedef struct { uint16_t charin, label[LABELS]; } places;
static const places lm = { LANEC_LM_CHARIN, { LANEC_LM_CHARALRM, LANEC_LM_NUM, LANEC_LM_89TEST, LANEC_LM_NUM - 2u,
    LANEC_LM_VERB, LANEC_LM_ERROR, LANEC_LM_VBRELDSP, LANEC_LM_POSGN, LANEC_LM_NEGSGN, LANEC_LM_ENTERJMP,
    LANEC_LM_CLEAR, LANEC_LM_NOUN } };
static const places cm = { LANEC_CM_CHARIN, { LANEC_CM_CHARALRM, LANEC_CM_NUM, LANEC_CM_89TEST, LANEC_CM_NUM - 2u,
    LANEC_CM_VERB, LANEC_CM_ERROR, LANEC_CM_VBRELDSP, LANEC_CM_POSGN, LANEC_CM_NEGSGN, LANEC_CM_ENTERJMP,
    LANEC_CM_CLEAR, LANEC_CM_NOUN } };
typedef char same_layout[(LANEC_LM_CHARIN2 == LANEC_LM_CHARIN + 11u && LANEC_CM_CHARIN2 == LANEC_CM_CHARIN + 11u &&
    LANEC_LM_CHARIN_BANK == 040u && LANEC_CM_CHARIN_BANK == 040u && LANEC_LM_NUM_BANK == 040u &&
    LANEC_CM_NUM_BANK == 040u && LANEC_LM_ENTERJMP == LANEC_LM_CHARIN2 + 4u + 33u &&
    LANEC_CM_ENTERJMP == LANEC_CM_CHARIN2 + 4u + 33u && LANEC_LM_DSPLOCK == LANEC_CM_DSPLOCK &&
    LANEC_LM_CADRSTOR == LANEC_CM_CADRSTOR && LANEC_LM_MPAC == LANEC_CM_MPAC && LANEC_LM_CHAR == LANEC_CM_CHAR &&
    LANEC_LM_21_S_22REG == LANEC_CM_21_S_22REG && LANEC_LM_ELRCODE1_WORD == 022u &&
    LANEC_CM_ELRCODE1_WORD == 022u) ? 1 : -1];
#define DSPLOCK LANEC_LM_DSPLOCK
#define CADRSTOR LANEC_LM_CADRSTOR
#define MPAC LANEC_LM_MPAC
#define CHAR LANEC_LM_CHAR
#define REG2122 LANEC_LM_21_S_22REG
#define ELRCODE1 022u
#define BIT5 020u
#define DSALMOUT 011u
#define TABLE(p) ((uint16_t)((p)->charin + 15u))   /* CHARIN2 +4 */

static const places *at(enum lanec_mission m) { return m == LANEC_LM ? &lm : &cm; }

uint16_t lanec_pin_keys_label(enum lanec_mission m, uint16_t key)
{
    return at(m)->label[group[key & 037u]];
}

/* CHARIN +0 CAF ONE, +1 XCH DSPLOCK, +2 TS 21/22REG, +3 CCS CADRSTOR, +4 TC +2,
 * +5 TC CHARIN2, +6 CS ELRCODE1, +7 AD MPAC, +8 EXTEND, +9 BZF CHARIN2,
 * +10 TC RELDSPON (CAF BIT5, EXTEND, WOR DSALMOUT, TC Q); CHARIN2 XCH MPAC,
 * TS CHAR, INDEX A, TC +1, then the table: TC <label> for each key. */
enum lanec_status lanec_pin_keys_charin(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    const places *p;
    uint16_t *w = s->words, key, old;
    uint8_t skip, light = 0;
    LANEC_ONE_MISSION(m);
    p = at(m);
    key = w[MPAC];
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 040u || w[LANEC_Z] != p->charin || key > 037u)
        return LANEC_OUT_OF_SCOPE;
    old = w[DSPLOCK]; w[DSPLOCK] = 1u; w[LANEC_A] = lanec_a16(old);   /* CAF ONE, XCH DSPLOCK */
    (void)lanec_ts(s, REG2122);                                         /* A is a 15-bit word: no overflow */
    skip = lanec_ccs(s, w[CADRSTOR]);
    if (skip != 1u) {                                                   /* +0: TC CHARIN2 */
        if (skip != 3u) w[LANEC_A] = (uint16_t)~lanec_a16(ELRCODE1);   /* -0 skips to AD MPAC */
        lanec_ad(s, w[MPAC]);
        light = w[LANEC_A] != 0u && w[LANEC_A] != 0177777u;            /* BZF CHARIN2 */
    }
    if (light) {                                                        /* RELDSPON */
        w[LANEC_A] = BIT5;
        w[LANEC_A] = lanec_a16((uint16_t)(BIT5 | io->channel[DSALMOUT]));
        lanec_write(s, io, DSALMOUT);
    }
    old = w[MPAC]; (void)lanec_ts(s, MPAC); w[LANEC_A] = lanec_a16(old);   /* XCH MPAC */
    (void)lanec_ts(s, CHAR);
    lanec_tc(s, p->label[group[key]], (uint16_t)(TABLE(p) + key + 1u));
    return LANEC_OK;
}

/* MPAC as CHARIN leaves it for this key and CADRSTOR, or 0177777 when the key
 * lights KEY REL and MPAC must equal channel 11 with bit 5 set. */
static uint16_t charin_mpac(uint16_t cadrstor, uint16_t key)
{
    if (cadrstor == 0u || (cadrstor == 077777u && key == 0u)) return 0u;
    if (cadrstor != 077777u && key == ELRCODE1) return 077777u;
    return 0177777u;
}

static enum lanec_status run(const key_run *f, apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    for (; *f; ++f)
        if ((*f)(s, io, m) == LANEC_OK) return LANEC_OK;
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pin_keys_handler(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    static const key_run own_alrm[] = { lanec_pw_source_alarm_run, 0 }, own_rel[] = { lanec_pw_operator_release_run, 0 },
                           own_nv[] = { lanec_pw_nv_key_run, 0 }, none[] = { 0 };
    static const key_run lm_num[] = { lanec_pw_charin_number_run, 0 }, cm_num[] = { lanec_pw_cm_charin_number_run, 0 },
                           lm_nv[] = { lanec_pw_charin_nv_run, 0 }, cm_nv[] = { lanec_pw_cm_charin_nv_run, 0 },
                           alrm[] = { lanec_pw_charin_alarm_run, 0 }, reset[] = { lanec_pw_charin_reset_run, 0 },
                           rel[] = { lanec_pw_charin_release_run, 0 }, sign[] = { lanec_pw_charin_sign_run, 0 },
                           enter[] = { lanec_pw_enter_run, 0 }, clear[] = { lanec_pw_charin_clear_run, 0 };
    const places *p;
    const key_run *own, *from;
    uint16_t *w = s->words, key = w[CHAR], z = w[LANEC_Z], expect, saved[3];
    uint8_t g;
    LANEC_ONE_MISSION(m);
    p = at(m);
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 040u) return LANEC_OUT_OF_SCOPE;
    for (g = 0; g < LABELS && p->label[g] != z; ++g)
        ;
    switch (g) {
    case ALRM: own = own_alrm; from = alrm; break;
    case NUM: case T89: case NUM0: own = none; from = m == LANEC_LM ? lm_num : cm_num; break;
    case VERB: case NOUN: own = own_nv; from = m == LANEC_LM ? lm_nv : cm_nv; break;
    case ERROR: own = none; from = reset; break;
    case REL: own = own_rel; from = rel; break;
    case POS: case NEG: own = none; from = sign; break;
    case ENTJ: own = none; from = enter; break;
    case CLEAR: own = none; from = clear; break;
    default: return LANEC_OUT_OF_SCOPE;
    }
    /* A label's own module takes any state it covers. */
    if (run(own, s, io, m) == LANEC_OK) return LANEC_OK;
    /* Otherwise CHARIN's dispatch: CHAR a key of this label, A the key, Q
     * past its table word, the lock set, MPAC and KEY REL as CHARIN leaves them. */
    if (key > 037u || group[key] != g || w[LANEC_A] != key || w[LANEC_Q] != TABLE(p) + key + 1u || w[DSPLOCK] != 1u)
        return LANEC_OUT_OF_SCOPE;
    expect = charin_mpac(w[CADRSTOR], key);
    if (expect == 0177777u ? (io->channel[DSALMOUT] & BIT5) == 0u || w[MPAC] != io->channel[DSALMOUT] || io->channel[DSALMOUT] > 077777u
                           : w[MPAC] != expect)
        return LANEC_OUT_OF_SCOPE;
    /* The state CHARIN started from: the earlier modules run CHARIN whole,
     * relighting KEY REL (already on) and rewriting the lock and MPAC. */
    saved[0] = z; saved[1] = w[DSPLOCK]; saved[2] = w[MPAC];
    w[LANEC_Z] = p->charin; w[DSPLOCK] = w[REG2122]; w[MPAC] = key;
    if (run(from, s, io, m) == LANEC_OK) return LANEC_OK;
    w[LANEC_Z] = saved[0]; w[DSPLOCK] = saved[1]; w[MPAC] = saved[2];
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pin_keys_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    LANEC_ONE_MISSION(m);
    return s->words[LANEC_Z] == at(m)->charin ? lanec_pin_keys_charin(s, io, m) : lanec_pin_keys_handler(s, io, m);
}
