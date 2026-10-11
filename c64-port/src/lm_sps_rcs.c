/* SPDX-License-Identifier: MIT */
#include "lm_sps_rcs.h"
#include "agc_multiword.h"

/* BZMF tests the uncorrected 16-bit A register, including both zeros. */
static uint8_t nonpositive(agc_register a)
{
    return (uint8_t)((a & 0100000u) != 0 || a == 0 || a == 0177777u);
}

static void product(apollo_overlay_state *m, agc_word factor)
{
    agc_register_pair r = agc_multiply(m->words[0], factor);
    m->words[0] = r.a;
    m->words[1] = r.l;
}

static void inhibit(apollo_overlay_state *m, unsigned jet,
                    unsigned counter, int axis)
{
    enum agc_ccs_class sense = agc_classify(m->words[03345]), cl;
    agc_word value;
    if (sense == AGC_PLUS_ZERO) {
        value = m->words[counter];
        m->words[0] = agc_diminished_abs(value);
        cl = agc_classify(value);
        if (cl == AGC_PLUS_ZERO || cl == AGC_MINUS_ZERO) return;
    } else {
        value = m->words[jet];
        if (sense != AGC_POSITIVE) value = agc_negate(value);
        m->words[0] = agc_diminished_abs(value);
        if (agc_classify(value) == AGC_POSITIVE) return;
        m->words[counter] = (agc_word)(axis < 0 ? 4u : 012u);
    }
    /* The literal CTRCHECK path also inhibits negative raw counters. */
    m->words[jet] = 0;
    m->words[0] = 0;
}

enum lm_sps_rcs_status lm_sps_rcs(apollo_overlay_state *m)
{
    agc_word axis_word, saved, old;
    unsigned jet, counter;
    int axis;
    enum agc_ccs_class sense, cl;
    agc_register rate, trial;
    if (m->words[5] != 03727u || m->words[3] != 03000u ||
        m->words[4] != 042000u || m->words[6] != 042006u ||
        m->words[7] != 0 || m->words[2] < 02000u || m->words[2] > 07777u)
        return LM_SPS_RCS_OUT_OF_SCOPE;
    axis_word = m->words[03105];
    if (axis_word != 077776u && axis_word != 0 &&
        axis_word != 1u && axis_word != 077777u)
        return LM_SPS_RCS_OUT_OF_SCOPE;
    axis = agc_to_int(axis_word);
    jet = (unsigned)(03125 + axis);
    counter = (unsigned)(03370 + axis);
    saved = m->words[2];
    m->words[1] = agc_extend(agc_correct(m->words[1]));

    /* Source jet-sense and rate-deadband test. LXCH A leaves signed rate in L. */
    old = m->words[jet];
    sense = agc_classify(old);
    if (sense == AGC_POSITIVE || sense == AGC_NEGATIVE) {
        m->words[03345] = sense == AGC_POSITIVE ? 1u : 077776u;
        rate = agc_extend(m->words[03027]);
        if (sense == AGC_POSITIVE) rate = (agc_register)(rate ^ 0177777u);
        m->words[1] = rate;
        trial = (m->words[0111] & 0200u) ? 0u : 045u;
        trial = agc_add_register(trial, rate);
        m->words[0] = trial;
        if (!nonpositive(trial)) {
            inhibit(m, jet, counter, axis);
            goto done;
        }
    } else {
        m->words[03345] = 0;
    }

    /* Outer rate test uses the source MP high word's CCS classification. */
    m->words[0] = agc_extend(m->words[03027]);
    product(m, 032);
    cl = agc_classify(agc_correct(m->words[0]));
    if (cl == AGC_POSITIVE) { trial = agc_extend(057777); goto store; }
    if (cl == AGC_NEGATIVE) { trial = 020000; goto store; }

    /* Coast zone retains the full AD overflow until MP and BZMF inspect it. */
    m->words[0] = agc_add_register(agc_extend(m->words[03027]),
                                  agc_extend(m->words[03352]));
    product(m, m->words[03011]);
    if (m->words[0] == 0 || m->words[0] == 0177777u) {
        trial = 0;
        goto store;
    }
    if (!nonpositive(m->words[0])) {
        trial = agc_add_register(agc_extend(m->words[03027]), 00632);
        trial = nonpositive(trial) ? 0u : agc_extend(057777);
    } else {
        trial = agc_add_register(agc_extend(077145),
                                 agc_extend(m->words[03027]));
        trial = nonpositive(trial) ? 020000u : 0u;
    }
store:
    m->words[0] = trial;
    m->words[jet] = agc_correct(trial);
    inhibit(m, jet, counter, axis);
done:
    m->words[5] = saved;
    m->words[2] = 3;
    return LM_SPS_RCS_OK;
}

uint8_t lm_sps_rcs_constant_read(uint16_t address, uint16_t *out)
{
    if (out == 0) return 0;
    switch (address) {
    case 18342: *out = 00632; return 1;
    case 18372: *out = 4; return 1;
    case 18373:
    case 18374: *out = 012; return 1;
    default: return 0;
    }
}
