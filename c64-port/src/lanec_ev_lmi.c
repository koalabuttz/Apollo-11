/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS interpretive words of Luminary099 RRLOSDSP (bank 40),
 * TRMTRAK1 (bank 42), ATTACHIT and TCPIN (bank 43), compiled onto the native interpreter paths
 * of lm_interp_steps and lm_interp_orders. As in the other compiled
 * interpretive routines, each order and operand word appears below only as
 * the literal argument those paths take for the interpreter's bookkeeping.
 * Entries are the interpreter's own states: NEWOPS with LOC at a word the
 * interpreter resumes on: the first word after a basic TC INTPRET (INTPRET,
 * the interpreter's code, leaves LOC there), a CALL's return, a branch
 * target, or the word after each DANZIG (where a job that CHANG2 suspended
 * for a higher-priority NEWJOB resumes). Each is an interpretive point of
 * the transfer ABI, keyed by LOC under BANKSET.
 * Each run ends where the interpreter leaves these words: at NEWOPS of a
 * called routine, at the basic word after EXIT, at the RTB routine, at
 * UNIT's alarm, or at CHANG2 when a DANZIG finds NEWJOB set. See
 * docs/LANEC-EV-LMI.md. */
#include "lanec_ev_lmi.h"
#include "lanec_symbols.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"

#define L(X) LANEC_LM_##X
#define W s->words
#define NEWOPS 06070u
#define CHANG2 05126u
#define NOVAC_FIXLOC 0110u /* NOVAC's FAKEPRET: LOC(MPAC +6) - LOC(QPRET) */

/* MISCJUMP body entries (Luminary099 INTERPRETER). */
enum { CALLITA = 02507, RTBBHIZ = 02447, SWITCH = 02516, LXA = 02354, BPLBMN = 02474 };
/* INDJUMP/UNAJUMP body entries. */
enum { DLOAD = 06052, VLOAD = 06505, DAD = 06775, DOT = 07331, UNIT = 03023 };
/* INTERPRETIVE CONSTANTS (23,2513-2530): UNITZ, UNITY +2, UNITX +4,
 * ZEROVECS (HI6ZEROS) +6, DPPOSMAX +14. */
static const agc_word K[16] = { 0, 0, 0, 0, 020000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 037777, 037777 };
#define RR(n) (L(RRLOSDSP) + (n))

typedef struct { uint16_t bank, z, at; } point;   /* at: LOC at NEWOPS */
static const point entries[] = {
    { 040u, NEWOPS, RR(4u) }, { 040u, NEWOPS, RR(6u) }, { 040u, NEWOPS, RR(025u) }, { 040u, NEWOPS, RR(030u) },
    { 040u, NEWOPS, RR(042u) }, { 040u, NEWOPS, RR(045u) },
    /* the words after each DANZIG */
    { 040u, NEWOPS, RR(7u) }, { 040u, NEWOPS, RR(011u) }, { 040u, NEWOPS, RR(013u) }, { 040u, NEWOPS, RR(014u) },
    { 040u, NEWOPS, RR(015u) }, { 040u, NEWOPS, RR(017u) }, { 040u, NEWOPS, RR(021u) }, { 040u, NEWOPS, RR(023u) },
    { 040u, NEWOPS, RR(032u) }, { 040u, NEWOPS, RR(034u) }, { 040u, NEWOPS, RR(036u) }, { 040u, NEWOPS, RR(040u) },
    { 040u, NEWOPS, RR(046u) }, { 043u, NEWOPS, L(ATTACHIT) + 7u },
    { 042u, NEWOPS, L(TRMTRAK1) + 7u }, { 042u, NEWOPS, L(TRMTRAK1) + 9u },
    { 043u, NEWOPS, L(ATTACHIT) + 1u }, { 043u, NEWOPS, L(ATTACHIT) + 3u }, { 043u, NEWOPS, L(ATTACHIT) + 9u },
    { 043u, NEWOPS, L(ATTACHIT) + 16u }, { 043u, NEWOPS, L(ATTACHIT) + 18u }, { 043u, NEWOPS, L(ATTACHIT) + 21u },
    { 043u, NEWOPS, L(TCPIN) }
};
#define COUNT ((uint16_t)(sizeof(entries) / sizeof(entries[0])))

/* CALL label (fixed CADR) to the callee's NEWOPS. */
static void call(apollo_overlay_state *s, agc_word word) { lmi_misc(s, word, CALLITA); lmi_call(s); lmi_goto(s); }
static uint8_t sw(apollo_overlay_state *s, p66_bias_io *io, agc_word word)
{
    uint8_t r;
    lmi_misc(s, word, SWITCH);
    r = lmi_switch(s, io);
    /* TS Q stored the flag word sign-extended (16 bits); the shared step
     * keeps Q's 15 */
    W[LANEC_Q] = (uint16_t)(W[LANEC_Q] | ((W[LANEC_Q] & 040000u) << 1));
    return r;
}
/* EXIT (a single order) to the basic word after it. */
static void exit_order(apollo_overlay_state *s) { lmi_newops(s, 077776); lmi_exit(s); }

/* DANZIG to the next word's NEWOPS; with NEWJOB set (a job a T3RUPT or
 * another job readied) CCS NEWJOB takes TCF CHANG2 instead, and the run
 * ends there: every order after it is skipped. */
static uint8_t parked;
static void next(apollo_overlay_state *s)
{
    if (parked) return;
    if (W[067] == 0) { lmi_danzig_next(s); return; }
    /* DANZIG CA BANKSET, TS BBANK; NOIBNKSW CCS EDOP (+0); CCS NEWJOB; TCF CHANG2 */
    ai_ca(s, 0165u); (void)ai_ts(s, 6u); (void)ai_ccs(s, 023u); (void)ai_ccs(s, 067u);
    W[LANEC_Z] = CHANG2;
    parked = 1;
}
/* STORE: the store from NEWOPS, then DANZIG to the next word's NEWOPS. */
static void store(apollo_overlay_state *s, agc_word code) { lmi_store_code(s, code); next(s); }
/* STODL/STOVL: the store, then the load of `word` (`k` its fixed data, or
 * NULL for erasable). */
static void stodl(apollo_overlay_state *s, agc_word code, agc_word word, const agc_word *k)
{
    lmi_store_code(s, code); lmi_dodload(s); lmi_diradres(s, word, DLOAD); lmi_dload_op(s, k); next(s);
}
static void stovl(apollo_overlay_state *s, agc_word code, agc_word word, const agc_word *k)
{
    lmi_store_code(s, code); lmi_dovload(s); lmi_diradres(s, word, VLOAD); lmi_vload(s, k); next(s);
}
static void dot(apollo_overlay_state *s, agc_word word, const agc_word *k)
{
    lmi_newops(s, 077641); lmi_direct(s, word, DOT); lmi_dot(s, k); next(s);
}
/* STCALL ARCTRIG to its NEWOPS. */
static void stcall_arctrig(apollo_overlay_state *s, agc_word code)
{
    lmi_store_code(s, code); lmi_15bitadr(s, 047320, CALLITA); lmi_call(s); lmi_goto(s);
}
/* BPL DAD +2 DPPOSMAX: to NEWOPS of the word after either way. */
static void bpl_dad(apollo_overlay_state *s, agc_word word)
{
    lmi_newops(s, 043244);
    lmi_misc(s, word, BPLBMN);
    if (lmi_bplbmn(s)) { lmi_goto(s); return; }
    lmi_danzig_right(s);
    lmi_direct(s, 06530, DAD); lmi_dad(s, K + 14); next(s);
}

/* RRLOSDSP: elevation and azimuth of the RR line of sight. */
static void rrlosdsp(apollo_overlay_state *s, uint16_t at)
{
#define PARKED() do { if (parked) return; } while (0)
    switch (at) {
    case RR(6u):
        /* STORE 0D; STODL 6D HI6ZEROS; STOVL 8D 6D; UNIT; STORE 6D */
        store(s, 000001); PARKED();
        /* fall through */
    case RR(7u): stodl(s, 014007, 06522, K + 6); PARKED();
        /* fall through */
    case RR(011u): stovl(s, 024011, 000007, 0); PARKED();
        /* fall through */
    case RR(013u):
        lmi_newops(s, 077656); lmi_unary(s, UNIT);
        if (lmi_unit(s)) return;
        next(s); PARKED();
        /* fall through */
    case RR(014u): store(s, 000007); PARKED();
        /* fall through */
    /* DOT UNITZ; STOVL COSTH UNITX; DOT 6D; STCALL SINTH ARCTRIG */
    case RR(015u): dot(s, 06514, K); PARKED();
        /* fall through */
    case RR(017u): stovl(s, 024021, 06520, K + 4); PARKED();
        /* fall through */
    case RR(021u): dot(s, 000007, 0); PARKED();
        /* fall through */
    case RR(023u): stcall_arctrig(s, 034023);
        return;
    case RR(025u):
        bpl_dad(s, 060047); PARKED();
        /* fall through */
    case RR(030u):
        /* STOVL RR-ELEV 0D; DOT UNITY; STOVL SINTH 0D; DOT 6D; STCALL COSTH ARCTRIG */
        stovl(s, 026203, 000001, 0); PARKED();
        /* fall through */
    case RR(032u): dot(s, 06516, K + 2); PARKED();
        /* fall through */
    case RR(034u): stovl(s, 024023, 000001, 0); PARKED();
        /* fall through */
    case RR(036u): dot(s, 000007, 0); PARKED();
        /* fall through */
    case RR(040u): stcall_arctrig(s, 034021);
        return;
    case RR(042u):
        bpl_dad(s, 060064); PARKED();
        /* fall through */
    case RR(045u):
        /* STORE RR-AZ; EXIT */
        store(s, 002201); PARKED();
        /* fall through */
    default:
        exit_order(s);
        return;
    }
#undef PARKED
}

static void run(apollo_overlay_state *s, p66_bias_io *io, const point *p)
{
    if (p->at == RR(4u) || p->at == L(TRMTRAK1) + 7u || p->at == L(ATTACHIT) + 1u || p->at == L(ATTACHIT) + 16u) {
        /* the first word after TC INTPRET: CALL RRNBMPAC (RRLOSDSP),
         * INTSTALL (TRMTRAK1, ATTACHIT) or PTOALEM */
        lmi_newops(s, 077624);
        call(s, p->bank == 040u ? 046065 : p->at == L(ATTACHIT) + 16u ? 026760 : 027414);
        return;
    }
    parked = 0;
    if (p->bank == 040u) {
        rrlosdsp(s, p->at);
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
            exit_order(s);
            return;
        }
        next(s);
        if (parked) return;
        /* fall through */
    case L(ATTACHIT) + 7u:
        lmi_newops(s, 077614);
        (void)sw(s, io, 004263);
        next(s);
        if (parked) return;
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
    uint16_t i, bank;
    p66_bias_io pio;
    if (m != LANEC_LM || !lanec_registers_valid(s)) return LANEC_OUT_OF_SCOPE;
    bank = lanec_current_bank(s, io);
    for (i = 0; i < COUNT; ++i) {
        if (entries[i].bank == bank && entries[i].z == W[LANEC_Z] && entries[i].at == W[0164]) break;
    }
    if (i == COUNT) return LANEC_OUT_OF_SCOPE;
    /* the interpreter's contract: NEWJOB +0 or a job's core set (positive;
     * the next DANZIG then changes jobs), the job's
     * VAC area (FIXLOC) in unswitched erasable above the interpreter's work
     * area, and at NEWOPS EDOP +0 with
     * BANKSET naming the current banks and INTBIT15 its bit 15. TRMTRAK1
     * (V56) runs in Pinball's NOVAC job: FIXLOC is NOVAC's, MPAC +6 less
     * QPRET (0110), so CALL INTSTALL's QPRET lands in MPAC +6, as on the
     * AGC; its two words use no other work-area word. */
    if (W[067] & 040000u) return LANEC_OUT_OF_SCOPE;
    if (!(W[0120] == NOVAC_FIXLOC && bank == 042u) && (W[0120] < 0200u || W[0120] >= 01400u - 060u))
        return LANEC_OUT_OF_SCOPE;
    if (W[023] != 0 || W[0165] != W[LANEC_BB] || W[0115] != (W[0165] & 040000u))
        return LANEC_OUT_OF_SCOPE;
    pio.superbank = io->superbank; pio.interrupt_enabled = io->interrupt_enabled;
    run(s, &pio, &entries[i]);
    io->superbank = pio.superbank; io->interrupt_enabled = pio.interrupt_enabled;
    return LANEC_OK;
}
