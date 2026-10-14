/* SPDX-License-Identifier: MIT */
/* Runtime interpreter: Luminary099/INTERPRETER.agc (Comanche055 through
 * src/cm_interp_runtime.c) run over the mission's rope image, as the AGC ran
 * it. Interpretive programs stay the data they always were: each call fetches
 * the order-pair or store-code word at LOC (under BANKSET) from the rope,
 * decodes its order codes and runs the dispatch path and order body the
 * original takes, using the same image-level bodies the compiled routines
 * call (src/lm_interp_steps.c, src/lm_interp_orders.c). Operand words come
 * from the rope at LOC, and a fixed operand (ADDRWD in 02000-03777 after
 * address formation) is read from the rope at FB/ADDRWD.
 *
 * One call runs one NEWOPS slot: from NEWOPS (or a pending DANZIG, or one of
 * the interpreter's basic entry points) to the next NEWOPS, or to the first
 * point outside the interpreter (EXIT's basic code, an RTB routine, CHANG2,
 * an alarm), with Z there, as a compiled routine does. The trampoline then
 * resolves the continuation, which is the interpreter again at NEWOPS unless
 * the program registers a compiled routine there (the hybrid: compiled
 * routines win, the interpreter takes every other interpretive point). */
#include "interp_runtime.h"
#include "interp_runtime_tables.h"
#include "interp_mission.h"
#include "lm_interp_steps.h"
#include "lm_interp_orders.h"
#include "agc_image_ops.h"
#include "rope_image.h"
#include "agc_image_w.h"
#define MISSION IF_(0u,1u)
enum { rEDOP = 023, rCYR = 020, rNEWJOB = 067, rADDRWD = 0116, rPOLISH = 0117, rFIXLOC = 0120, rLOC = 0164, rBANKSET = 0165 };
enum { DONE = 0, MORE = 1, REFUSE = 2 };
/* Whether STORE,1 or STORE,2 would store outside erasable (#422): a
 * negative indexed address takes STORE's work-area branch, and FIXLOC plus
 * that address lies above 01777, where INDEX ADDRWD turns the DXCH into
 * another instruction. Refused: indexed erasable stays inside erasable. */
static uint8_t store_outside(const apollo_overlay_state *s, agc_word code)
{
    unsigned kind = (code >> 11) & 7u, nx, a;
    if (kind - 1u > 1u) return 0;
    nx = ~(unsigned)W[(W[rFIXLOC] + (kind == 1u ? 046u : 047u)) & 01777u] & 077777u;   /* X1, X2 */
    if (nx < 040000u) return 0;
    a = (code & 03777u) + nx; if (a > 077777u) a -= 077777u;
    if (!(a & 040000u)) return 0;
    a += W[rFIXLOC]; if (a > 077777u) a -= 077777u;
    return a > 01777u;
}

static const agc_word indjump[32] = IRT_INDJUMP, miscjump[16] = IRT_MISCJUMP, unajump[15] = IRT_UNAJUMP;

/* Physical rope address of fixed logical address a under bank register b
 * (FB or BANKSET's bits 14-11) and the channel-7 superbank bits (0, 1, 2
 * and 3 the same banks, as in flight with SUPERBNK 3; 4 banks 040-043);
 * 0xffff for erasable or outside the rope. */
static uint16_t phys(unsigned a, agc_word b, unsigned superbank)
{
    unsigned bank;
    if (a >= 04000u) return (uint16_t)(2048u + (a & 07777u) - 04000u);
    if (a < 02000u) return 0xffffu;
    bank = (b >> 10) & 037u;
    if (bank >= 030u && (superbank & 0100u)) {
        if ((superbank & 0160u) != 0100u) return 0xffffu;
        bank += 010u;
    }
    return (uint16_t)(bank * 1024u + (a & 01777u));
}
static uint8_t rope(uint16_t p, agc_word *w) { return p != 0xffffu && apollo_rope_read(MISSION, p, w); }
/* The word off words after LOC in the interpretive program. */
static uint8_t code_word(const apollo_overlay_state *s, unsigned superbank, unsigned off, agc_word *w)
{
    return rope(phys((W[rLOC] + off) & 077777u, W[rBANKSET], superbank), w);
}
/* Fixed operand words at ADDRWD under FB (n words; past the rope's end
 * reads as zero, which no order uses). Returns k, or 0 for an erasable
 * operand (the bodies then read C(ADDRWD) from the image). */
static const agc_word *operand(const apollo_overlay_state *s, unsigned superbank, agc_word *k, unsigned n)
{
    unsigned i, a = W[rADDRWD] & 077777u;
    if (a < 02000u) return 0;
    for (i = 0; i < n; ++i) if (!rope(phys(a + i, W[4], superbank), &k[i])) k[i] = 0;
    return k;
}

/* DANZIG's job switch: NEWJOB positive goes to CHANG2 (CCS EDOP +0; CCS
 * NEWJOB; TCF CHANG2), from DANZIG (bank) or NOIBNKSW (no bank). */
static void chang2(apollo_overlay_state *s, uint8_t bank)
{
    if (bank) { ai_ca(s, rBANKSET); (void)ai_ts(s, 6); }
    (void)ai_ccs(s, rEDOP); (void)ai_ccs(s, rNEWJOB); W[5] = IRT_CHANG2;
}

/* CGOTO 04 (ADDRWD names the modifier, the word after the operand the CADR
 * list): INDEX LOC, CA 1; INDEX ADDRWD, AD 0; TS FBANK; MASK LOW10; INDEX A,
 * CA 10000; TS POLISH; TCF GOTO +1. */
static uint8_t cgoto(apollo_overlay_state *s, unsigned superbank)
{
    agc_word list, v, cadr;
    int i;
    if (!code_word(s, superbank, 1u, &list)) return REFUSE;
    v = ai_word(s, W[rADDRWD]);
    i = (v & 040000u) ? -(int)(~v & 037777u) : (int)v;
    if (i < 0 || (list & 01777u) + (unsigned)i > 01777u) return REFUSE;
    if (!rope(phys(02000u + ((list & 01777u) + (unsigned)i), list, superbank), &cadr)) return REFUSE;
    ai_caf(s, list); ai_ad(s, W[rADDRWD]); (void)ai_ts(s, 4); ai_maskf(s, 01777u);
    ai_caf(s, cadr); (void)ai_ts(s, rPOLISH); lmi_goto_any(s);
    return DONE;
}

/* The address order idx (INDJUMP) after its address formation. */
static uint8_t addr_body(apollo_overlay_state *s, unsigned superbank, unsigned idx)
{
    agc_word kw[18], v;
    const agc_word *k = operand(s, superbank, kw, idx == 013u || idx == 016u ? 18u : 6u);
    switch (idx) {
    case 000: lmi_vload(s, k); break;
    case 001: lmi_tad(s, k); break;
    case 002: lmi_sign(s, k); break;
    case 003: lmi_vxsc(s, k); break;
    case 004: return cgoto(s, superbank);
    case 005: if (k) lmi_tload_fixed(s, k); else lmi_tload(s); break;
    case 006: lmi_dload_op(s, k); break;
    case 007: lmi_vsc(s, k); break;
    case 010: if (k) lmi_sload_fixed(s, k); else lmi_sload(s); break;
    case 011: if (!code_word(s, superbank, 1u, &v)) return REFUSE; lmi_ssp(s, v); break;
    case 012: lmi_pddl_op(s, k); break;
    case 013: lmi_mxv(s, k); break;
    case 014: lmi_pdvl(s, k); break;
    case 016: lmi_vxm(s, k); break;
    case 017: lmi_tslc(s); break;
    case 020: lmi_dmpr(s, k); break;
    case 021: lmi_ddv(s, k); break;
    case 022: lmi_bddv(s, k); break;
    case 023: lmi_gshift(s); break;
    case 024: lmi_vad(s, k); break;
    case 025: lmi_vsu(s, k); break;
    case 026: lmi_bvsu(s, k); break;
    case 027: lmi_dot(s, k); break;
    case 030: lmi_vxv(s, k); break;
    case 031: lmi_vproj(s, k); break;
    case 032: lmi_dsu(s, k); break;
    case 033: lmi_bdsu(s, k); break;
    case 034: lmi_dad(s, k); break;
    case 036: lmi_dmp(s, k); break;
    case 037: lmi_setpd(s); break;
    default: return REFUSE;   /* 015 CCALL (no program uses it), 035 unassigned */
    }
    return MORE;
}

/* Erasable branch word (a switch order's or GOTO's CADR in erasable). */
static void swbranch(apollo_overlay_state *s, agc_word w)
{
    if (w & 074000u) { lmi_swbranch(s, w); return; }
    ai_ca(s, rBANKSET); (void)ai_ts(s, 4); ai_caf(s, w); (void)ai_ts(s, rPOLISH); lmi_goto_any(s);
}

/* One order with code c (OPJUMP done: CYR holds it, A its CCS). */
static uint8_t order(apollo_overlay_state *s, p66_bias_io *io, unsigned c)
{
    unsigned sb = io->superbank, idx;
    agc_word w;
    if (c == 0u) { lmi_exit(s); return DONE; }
    if (c & 1u) {
        idx = c >> 2;
        if (!code_word(s, sb, 1u, &w)) return REFUSE;
        if (c & 2u) lmi_indexed(s, w, indjump[idx]);
        else if (w & 040000u) {
            if (idx == 037u || idx == 004u || idx == 011u) return REFUSE;   /* SETPD, CGOTO, SSP push up: never assembled */
            lmi_address_pushup(s, w, indjump[idx]);
        } else lmi_direct(s, w, indjump[idx]);
        return addr_body(s, sb, idx);
    }
    if (c & 2u) {
        idx = c >> 3;
        if (!code_word(s, sb, 1u, &w)) return REFUSE;
        lmi_misc(s, w, miscjump[idx]);
        switch (idx) {
        case 000: lmi_axt(s); break;
        case 001: lmi_axc(s); break;
        case 002: lmi_lxa(s); break;
        case 003: lmi_lxc(s); break;
        case 004: lmi_sxa(s); break;
        case 005: lmi_xchx(s); break;
        case 006: lmi_incr(s); break;
        case 007: if (lmi_tix(s)) { lmi_goto_any(s); return DONE; } break;
        case 010: lmi_xad(s); break;
        case 011: lmi_xsu(s); break;
        case 012: if (lmi_bzegoto(s)) { lmi_goto_any(s); return DONE; } break;
        case 013: if (lmi_bplbmn(s)) { lmi_goto_any(s); return DONE; } break;
        case 014:
            (void)ai_ccs(s, rCYR);
            if (!(c & 4u)) { lmi_rtb(s); return DONE; }
            if (lmi_bhiz(s)) { lmi_goto_any(s); return DONE; }
            break;
        case 015:
            if (c & 4u) { lmi_ita(s); break; }
            lmi_call(s); lmi_goto_any(s); return DONE;
        case 016:
            if (lmi_switch(s, io)) {
                if (!code_word(s, sb, 1u, &w)) return REFUSE;
                swbranch(s, w); return DONE;
            }
            break;
        default:
            switch (lmi_bov(s)) {
            case LMI_BOV_NONE: break;
            case LMI_BOV_GOTO: lmi_goto_any(s); return DONE;
            case LMI_BOV_RTB: lmi_rtb(s); return DONE;
            default: return DONE;
            }
        }
        return MORE;
    }
    if (c & 4u) {
        lmi_short_dispatch(s);
        if (W[5] == IRT_SHORTT) lmi_shortt(s); else lmi_shortv(s);
        return MORE;
    }
    idx = (c >> 3) - 1u;
    lmi_unary(s, unajump[idx]);
    switch (idx + 1u) {
    case 001: if (lmi_sqrt(s)) return DONE; break;
    case 002: lmi_sin(s); break;
    case 003: lmi_cos(s); break;
    case 004: if (lmi_asin(s)) return DONE; break;
    case 005: if (lmi_acos(s)) return DONE; break;
    case 006: lmi_dsq(s); break;
    case 007: lmi_round(s); break;
    case 010: lmi_comp(s); break;
    case 011: lmi_vdef(s); break;
    case 012: if (lmi_unit(s)) return DONE; break;
    case 013: if (lmi_abvalabs(s)) return DONE; break;
    case 014: lmi_vsq(s); break;
    case 015:
        if (!code_word(s, sb, 1u, &w)) return REFUSE;
        /* (STADR's store code: CS 0; AD NEGONE.) */
        if (store_outside(s, (agc_word)(w & 077777u ? (~(unsigned)w & 077777u) - 1u : 077776u))) return REFUSE;
        lmi_stadr(s, w); break;
    case 016: lmi_rvq(s); return DONE;
    default: lmi_push(s); break;
    }
    return MORE;
}

/* From the image's Z through the dispatch paths and bodies to the end of
 * the slot. */
static enum exec_status run(apollo_overlay_state *s, p66_bias_io *io)
{
    agc_word w;
    uint8_t r = MORE, load = 0;
    unsigned guard;
    for (guard = 0; guard < 16u && r == MORE; ++guard) {
        switch (W[5]) {
        case IRT_NEWOPS:
            if (!code_word(s, io->superbank, 0u, &w)) return EXEC_OUT_OF_SCOPE;
            if (!(w & 040000u)) { if (store_outside(s, w)) return EXEC_OUT_OF_SCOPE; lmi_store_code(s, w); break; }
            lmi_newops(s, w);
            r = order(s, io, ((~(unsigned)w & 077777u) - 1u) & 0177u); break;
        case IRT_DANZIG:
            if (W[rEDOP]) { unsigned c = W[rEDOP] - 1u; lmi_danzig_right(s); r = order(s, io, c); break; }
            if (W[rNEWJOB]) { chang2(s, 1); return EXEC_OK; }
            lmi_danzig_next(s); return EXEC_OK;
        case IRT_NOIBNKSW:
            if (W[rEDOP]) { unsigned c = W[rEDOP] - 1u; lmi_noibnksw_right(s); r = order(s, io, c); break; }
            if (W[rNEWJOB]) { chang2(s, 0); return EXEC_OK; }
            (void)ai_ccs(s, rEDOP); (void)ai_ccs(s, rNEWJOB); ai_incr(s, rLOC); W[5] = IRT_NEWOPS; return EXEC_OK;
        case IRT_DODLOAD: lmi_dodload(s); load = 0; break;
        case IRT_DOVLOAD: lmi_dovload(s); load = 1; break;
        case IRT_DIRADRES:
            if (!code_word(s, io->superbank, 1u, &w)) return EXEC_OUT_OF_SCOPE;
            if (w & 040000u) lmi_diradres_pushup(s, w, indjump[load ? 000 : 006]);
            else lmi_diradres(s, w, indjump[load ? 000 : 006]);
            r = addr_body(s, io->superbank, load ? 000 : 006); break;
        case IRT_DODLOADX: case IRT_DOVLOADX:
            load = W[5] == IRT_DOVLOADX;
            if (!code_word(s, io->superbank, 1u, &w)) return EXEC_OUT_OF_SCOPE;
            ai_caf(s, load ? 040001u : IRT_DLOADX_WORD); (void)ai_ts(s, rCYR);
            lmi_indexed(s, w, indjump[load ? 000 : 006]);
            r = addr_body(s, io->superbank, load ? 000 : 006); break;
        case IRT_B15BITADR:
            if (!code_word(s, io->superbank, 1u, &w)) return EXEC_OUT_OF_SCOPE;
            lmi_15bitadr(s, w, miscjump[015]); lmi_call(s); lmi_goto_any(s); return EXEC_OK;
        case IRT_GOTO: lmi_goto_any(s); return EXEC_OK;
        case IRT_SWRETURN: lmi_swreturn(s); break;
        case IRT_INTPRET: lmi_intpret(s, io); return EXEC_OK;
        default: return EXEC_OK;   /* outside the interpreter */
        }
    }
    return r == REFUSE ? EXEC_OUT_OF_SCOPE : EXEC_OK;
}

/* The image as the bodies need it (the compiled routines' sane()). */
static uint8_t sane(const apollo_overlay_state *s, const p66_bias_io *io)
{
    unsigned vac = W[rFIXLOC];
    if (W[7] || W[rNEWJOB] > 037777u || W[3] > 03400u || (W[3] & 0377u) || W[6] != (agc_word)((W[4] & 076000u) | (W[3] >> 8))) return 0;
    if (io->interrupt_enabled > 1u || (io->superbank & ~0160u)) return 0;
    if (vac < 0401u || vac > 0661u || (vac - 0401u) % 054u) return 0;
    return 1;
}

#ifdef INTERP_COMANCHE
#include "cm_native_image.h"
static enum exec_status entry(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    cm_native_io *io = v;
    p66_bias_io l;
    enum exec_status st;
    (void)p;
    if (!s || !io) return EXEC_OUT_OF_SCOPE;
    l.superbank = io->superbank; l.interrupt_enabled = io->interrupt_enabled;
    if (!sane(s, &l)) return EXEC_OUT_OF_SCOPE;
    st = run(s, &l);
    io->superbank = l.superbank; io->interrupt_enabled = l.interrupt_enabled;
    return st;
}
#else
static enum exec_status entry(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    p66_bias_io *io = v;
    (void)p;
    if (!s || !io || !sane(s, io)) return EXEC_OUT_OF_SCOPE;
    return run(s, io);
}
#endif
const agc_routine lm_interp_runtime = entry;
#define PH(z) (uint16_t)((z) >= 04000u ? 2048u + (z) - 04000u : (z) - 02000u)
/* The interpreter's basic entry points, by physical Z (SWRETURN and
 * INTPRET are in fixed-fixed; DANZIG is reached by TCF DANZIG from basic
 * code). */
static const agc_routine_entry basic[3] = {
    { PH(IRT_SWRETURN), entry }, { PH(IRT_INTPRET), entry }, { PH(IRT_DANZIG), entry }
};
const agc_routine_table lm_interp_runtime_basic = { basic, 3u };
#undef W
