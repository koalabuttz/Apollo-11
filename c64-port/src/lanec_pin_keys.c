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

static uint8_t label_of(const places *p, uint16_t z)
{
    uint8_t g;
    for (g = 0; g < LABELS && p->label[g] != z; ++g)
        ;
    return g;
}

/* At label g, a state CHARIN's dispatch leaves (CHAR a key of the label, A
 * the key, Q past its table word, the lock set, MPAC and KEY REL as CHARIN
 * sets them from CADRSTOR): rebuild the state CHARIN started from and run
 * the earlier key module f, which runs CHARIN whole again (the same lock and
 * MPAC; KEY REL is already on). */
static enum lanec_status from_charin(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m, uint8_t g, key_run f)
{
    const places *p = at(m);
    uint16_t *w = s->words, key = w[CHAR], expect, saved[3];
    if (key > 037u || group[key] != g || w[LANEC_A] != key || w[LANEC_Q] != TABLE(p) + key + 1u || w[DSPLOCK] != 1u)
        return LANEC_OUT_OF_SCOPE;
    expect = charin_mpac(w[CADRSTOR], key);
    if (expect == 0177777u ? (io->channel[DSALMOUT] & BIT5) == 0u || w[MPAC] != io->channel[DSALMOUT] || io->channel[DSALMOUT] > 077777u
                           : w[MPAC] != expect)
        return LANEC_OUT_OF_SCOPE;
    saved[0] = w[LANEC_Z]; saved[1] = w[DSPLOCK]; saved[2] = w[MPAC];
    w[LANEC_Z] = p->charin; w[DSPLOCK] = w[REG2122]; w[MPAC] = key;
    if (f(s, io, m) == LANEC_OK) return LANEC_OK;
    w[LANEC_Z] = saved[0]; w[DSPLOCK] = saved[1]; w[MPAC] = saved[2];
    return LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pin_keys_num(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint8_t g;
    LANEC_ONE_MISSION(m);
    if (s->words[LANEC_Z] == at(m)->charin) return lanec_pin_keys_charin(s, io, m);
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 040u) return LANEC_OUT_OF_SCOPE;
    g = label_of(at(m), s->words[LANEC_Z]);
    if (g != NUM && g != T89 && g != NUM0) return LANEC_OUT_OF_SCOPE;
    return from_charin(s, io, m, g, m == LANEC_LM ? lanec_pw_charin_number_run : lanec_pw_cm_charin_number_run);
}

static uint8_t at_label(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    if (!lanec_registers_valid(s) || lanec_current_bank(s, io) != 040u) return LABELS;
    return label_of(at(m), s->words[LANEC_Z]);
}

/* VERB and NOUN, and CHARALRM, which other paths also enter: the label's own
 * module first, on any state it covers. */
enum lanec_status lanec_pin_keys_cmd(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint8_t g;
    LANEC_ONE_MISSION(m);
    switch (g = at_label(s, io, m)) {
    case ALRM:
        if (lanec_pw_source_alarm_run(s, io, m) == LANEC_OK) return LANEC_OK;
        return from_charin(s, io, m, g, lanec_pw_charin_alarm_run);
    case VERB: case NOUN:
        if (lanec_pw_nv_key_run(s, io, m) == LANEC_OK) return LANEC_OK;
        return from_charin(s, io, m, g, m == LANEC_LM ? lanec_pw_charin_nv_run : lanec_pw_cm_charin_nv_run);
    case ENTJ: return from_charin(s, io, m, g, lanec_pw_enter_run);
    default: return LANEC_OUT_OF_SCOPE;
    }
}

/* VBRELDSP, which only CHARIN's dispatch reaches. */
enum lanec_status lanec_pin_keys_release(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    LANEC_ONE_MISSION(m);
    return at_label(s, io, m) == REL ? from_charin(s, io, m, REL, lanec_pw_charin_release_run) : LANEC_OUT_OF_SCOPE;
}

/* POSGN, NEGSGN and ERROR (RSET). */
enum lanec_status lanec_pin_keys_sign(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint8_t g;
    LANEC_ONE_MISSION(m);
    switch (g = at_label(s, io, m)) {
    case POS: case NEG: return from_charin(s, io, m, g, lanec_pw_charin_sign_run);
    case ERROR: return from_charin(s, io, m, g, lanec_pw_charin_reset_run);
    default: return LANEC_OUT_OF_SCOPE;
    }
}

enum lanec_status lanec_pin_keys_clear(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    LANEC_ONE_MISSION(m);
    return at_label(s, io, m) == CLEAR ? from_charin(s, io, m, CLEAR, lanec_pw_charin_clear_run) : LANEC_OUT_OF_SCOPE;
}

enum lanec_status lanec_pin_keys_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    if (lanec_pin_keys_num(s, io, m) == LANEC_OK || lanec_pin_keys_cmd(s, io, m) == LANEC_OK ||
        lanec_pin_keys_release(s, io, m) == LANEC_OK || lanec_pin_keys_sign(s, io, m) == LANEC_OK)
        return LANEC_OK;
    return lanec_pin_keys_clear(s, io, m);
}
