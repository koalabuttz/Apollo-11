/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS interpretive words of Luminary099 TRMTRAK1 (bank 42),
 * ATTACHIT and TCPIN (bank 43), compiled onto the native interpreter paths
 * of lm_interp_steps and lm_interp_orders. As in the other compiled
 * interpretive routines, each order and operand word appears below only as
 * the literal argument those paths take for the interpreter's bookkeeping.
 * Entries are the interpreter's own states: INTPRET with Q at the first
 * interpretive word (from the basic TC INTPRET), and NEWOPS with LOC at a
 * word the interpreter resumes on (a CALL's return or a branch target).
 * Each run ends where the interpreter leaves these words: at NEWOPS of a
 * called routine, at the basic word after EXIT, or at the RTB routine. See
 * docs/LANEC-EV-LMI.md. */
#include "lanec_ev_lmi.h"
#include "lanec_symbols.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"

#define L(X) LANEC_LM_##X
#define W s->words
#define INTPRET 06037u
#define NEWOPS 06070u

/* MISCJUMP body entries (Luminary099 INTERPRETER). */
enum { CALLITA = 02507, RTBBHIZ = 02447, SWITCH = 02516, LXA = 02354 };

typedef struct { uint16_t bank, z, at; } point;   /* at: Q at INTPRET, LOC at NEWOPS */
static const point entries[] = {
    { 042u, INTPRET, L(TRMTRAK1) + 7u }, { 042u, NEWOPS, L(TRMTRAK1) + 9u },
    { 043u, INTPRET, L(ATTACHIT) + 1u }, { 043u, NEWOPS, L(ATTACHIT) + 3u }, { 043u, NEWOPS, L(ATTACHIT) + 9u },
    { 043u, INTPRET, L(ATTACHIT) + 16u }, { 043u, NEWOPS, L(ATTACHIT) + 18u }, { 043u, NEWOPS, L(ATTACHIT) + 21u },
    { 043u, NEWOPS, L(TCPIN) }
};
#define COUNT ((uint16_t)(sizeof(entries) / sizeof(entries[0])))

/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s, agc_word word) { lmi_misc(s, word, CALLITA); lmi_call(s); lmi_goto(s); }
static uint8_t sw(apollo_overlay_state *s, p66_bias_io *io, agc_word word)
{
    lmi_misc(s, word, SWITCH);
    return lmi_switch(s, io);
}
/* EXIT (a single order) to the basic word after it. */
static void exit_order(apollo_overlay_state *s) { lmi_newops(s, 077776); lmi_exit(s); }

static void run(apollo_overlay_state *s, p66_bias_io *io, const point *p)
{
    if (p->z == INTPRET) {
        /* TC INTPRET; CALL INTSTALL (TRMTRAK1, ATTACHIT) or CALL PTOALEM */
        lmi_intpret(s, io);
        lmi_newops(s, 077624);
        call(s, p->at == L(ATTACHIT) + 16u ? 026760 : 027414);
        return;
    }
    switch (p->at) {
    case L(TRMTRAK1) + 9u: case L(ATTACHIT) + 9u: case L(ATTACHIT) + 21u:
        exit_order(s);
        return;
    case L(ATTACHIT) + 3u:
        /* SET MOONOTH, BON MOONTHIS +3; CLEAR MOONOTH; +3 EXIT */
        lmi_newops(s, 043014);
        (void)sw(s, io, 004063);
        lmi_danzig_right(s);
        if (sw(s, io, 004304)) {
            lmi_swbranch(s, 067160);
        } else {
            lmi_danzig_next(s);
            lmi_newops(s, 077614);
            (void)sw(s, io, 004263);
            lmi_danzig_next(s);
        }
        exit_order(s);
        return;
    case L(ATTACHIT) + 18u:
        /* LXA,2 PBODY, CALL SVDWN1 */
        lmi_newops(s, 045154);
        lmi_misc(s, 002030, LXA); lmi_lxa(s);
        lmi_danzig_right(s);
        call(s, 026114);
        return;
    default:
        /* TCPIN: RTB PINBRNCH */
        lmi_newops(s, 077634);
        lmi_misc(s, 021050, RTBBHIZ); (void)ai_ccs(s, 020); lmi_rtb(s);
        return;
    }
}

const uint16_t *lanec_ev_lmi_entry(uint16_t i, uint16_t *bank, uint16_t *z, uint16_t *count)
{
    *count = COUNT;
    if (i >= COUNT) return 0;
    *bank = entries[i].bank; *z = entries[i].z;
    return &entries[i].at;
}

enum lanec_status lanec_ev_lmi_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    uint16_t i, bank, at;
    p66_bias_io pio;
    if (m != LANEC_LM || !lanec_registers_valid(s)) return LANEC_OUT_OF_SCOPE;
    bank = lanec_current_bank(s, io);
    for (i = 0; i < COUNT; ++i) {
        at = entries[i].z == INTPRET ? W[LANEC_Q] : W[0164];
        if (entries[i].bank == bank && entries[i].z == W[LANEC_Z] && entries[i].at == at) break;
    }
    if (i == COUNT) return LANEC_OUT_OF_SCOPE;
    /* the interpreter's contract: no job change pending (NEWJOB +0), the job's
     * VAC area (FIXLOC) in unswitched erasable, and at NEWOPS EDOP +0 with
     * BANKSET naming the current banks */
    if (W[067] != 0 || W[0120] >= 01400u - 060u) return LANEC_OUT_OF_SCOPE;
    if (entries[i].z == NEWOPS && (W[023] != 0 || W[0165] != W[LANEC_BB])) return LANEC_OUT_OF_SCOPE;
    pio.superbank = io->superbank; pio.interrupt_enabled = io->interrupt_enabled;
    run(s, &pio, &entries[i]);
    io->superbank = pio.superbank; io->interrupt_enabled = pio.interrupt_enabled;
    return LANEC_OK;
}
