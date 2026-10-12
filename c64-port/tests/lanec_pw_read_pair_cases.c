/* SPDX-License-Identifier: MIT */
/* pw-read-pair: READLO's fresh double-word read (src/dsky_read_pair.c).
 * Generator numbers 0-8191 are LM, 8192-16383 CM. */
#include "lanec_pw_e.h"
#include "dsky_read_pair.h"
#include "dsky_read_pair_cases.h"

/* The oracle stops where Z first reaches the caller (Q at entry). A caller
 * the original path itself passes first cannot be told apart there, so the
 * run refuses it: TEM4 (0144, where READLO's TC TEM4 lands), READLO's own
 * body (LM 02747-02770, CM 02655-02676, bank 40) and SETEBANK (LM
 * 04313-04316, CM 04327-04332): the addresses the oracle traced for this
 * suite (LANEC_TRACE). */
static enum lanec_status read_pair_run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    uint16_t q = s->words[LANEC_Q], entry = mission == LANEC_CM ? 02655u : 02747u;
    uint16_t sete = mission == LANEC_CM ? 04327u : 04313u, sete_end = mission == LANEC_CM ? 04332u : 04316u;
    p66_bias_io p; p.superbank = io->superbank; p.interrupt_enabled = io->interrupt_enabled;
    if (q == 0144u || (q >= entry && q <= entry + 021u) || (q >= sete && q <= sete_end)) return LANEC_OUT_OF_SCOPE;
    if (dsky_read_pair(s, &p, mission == LANEC_CM ? DSKY_NV_CM : DSKY_NV_LM) != P66_BIAS_OK) return LANEC_OUT_OF_SCOPE;
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    return LANEC_OK;
}
#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint16_t k = (uint16_t)lanec_pw_spread(n, CASES, DSKY_READ_PAIR_CASES);
    lanec_fill(s, io, n);
    dsky_read_pair_input(s, &p, k, &mission);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    LANEC_PW_E_STOP(m, s, io, read_pair_run);
}
const lanec_suite lanec_suite_pw_read_pair = { "pw-read-pair", CASES, make, read_pair_run };
