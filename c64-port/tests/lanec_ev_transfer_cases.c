/* SPDX-License-Identifier: MIT */
/* ev-transfer: EXTENDED_VERBS through the control-transfer ABI. Each case
 * starts at a genuine caller's TC BANKCALL or POSTJUMP whose CADR names an
 * extended-verb entry (Pinball's POSTJUMP GOEXTVB with every verb of the
 * front suite's cases, and the file's own BANKCALL/POSTJUMP sites): Z at the
 * INTER-BANK entry, Q at the caller's CADR, the caller's bank, the rest of the
 * state from an accepted case of the destination's own suite.
 *
 * The native side runs agc_transfer_run over a program of the mission's
 * INTER-BANK routines, the display-interface and extended-verb routines and,
 * for Luminary099, the Executive's job services, INTPRET for this file's
 * callers (lanec_ev_lm_intpret) and the extended verbs' interpretive words
 * (lanec_ev_lm_interpretive, keyed by LOC at NEWOPS), and ends where a
 * continuation has no registered routine (EXEC_UNMAPPED). The oracle runs the
 * original program until Z leaves the words of those files and of the
 * interpreter, or reaches NEWOPS with LOC outside the extended verbs'
 * interpretive words, or the Executive's idle loop ADVAN. */
#include <string.h>
#include "lanec_suites.h"
#include "lanec_symbols.h"
#include "lanec_di_routines.h"
#include "lanec_ev_routines.h"
#include "lm_interbank_routines.h"
#include "lm_exec_routines.h"
#include "cm_interbank_routines.h"
#include "standard_lead_routines.h"   /* LM_INTERPRETER_FETCH */
#include "lm_downtelemetry.h"
#include "cm_downtelemetry.h"

typedef struct { uint16_t site, callee; uint16_t n; uint8_t module; uint8_t mission; } transfer_case;
#include "lanec_ev_transfer_cases.inc"
#ifndef LANEC_EV_TRANSFER_C64
static const lanec_suite *const modules[] = { &lanec_suite_ev_front, &lanec_suite_ev_short, &lanec_suite_ev_align,
                                              &lanec_suite_ev_radar, &lanec_suite_ev_dap, &lanec_suite_ev_time,
                                              &lanec_suite_ev_track };
#else   /* the C64 program's cases all start from front-suite cases */
static const lanec_suite *const modules[] = { &lanec_suite_ev_front };
#endif

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static void loc(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->loc[m->locs].bank = bank; m->loc[m->locs].lo = lo; m->loc[m->locs].hi = hi; ++m->locs;
}

static void make_case(const transfer_case *c, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t bank = (uint16_t)(c->site / 1024u), offset = (uint16_t)(c->site % 1024u), i;
    modules[c->module]->make(c->n, s, io, m);
    memset(m, 0, sizeof *m);
    m->mission = (enum lanec_mission)c->mission;
    lanec_enter(s, io, bank, c->callee);
    s->words[LANEC_Q] = (uint16_t)(02000u + offset);
    range(m, LANEC_ANY_BANK, 0u, 0177u);             /* TC Q, TC A: code in the central registers */
    if (m->mission == LANEC_LM) {
        /* the Executive as in di-transfer: the caller alone in core set 0 */
        s->words[0167] = 020000u;
        s->words[LANEC_LM_DSPLOCK] &= 1u;
        for (i = 1u; i < 8u; ++i) s->words[0167u + i * 12u] = 077777u;
        for (i = 0; i < 5u; ++i) s->words[0400u + i * 44u] = (uint16_t)(0400u + i * 44u);
        s->words[067] = 0;
        range(m, LANEC_ANY_BANK, 04616u, 04731u);   /* INTER-BANK_COMMUNICATION */
        range(m, LANEC_ANY_BANK, 05464u, 05503u);   /* DISPLAY_INTERFACE_ROUTINES */
        range(m, 010u, 02326u, 03543u);
        range(m, LANEC_ANY_BANK, 05072u, 05172u);   /* EXECUTIVE */
        range(m, LANEC_ANY_BANK, 06000u, 07777u);   /* INTERPRETER (and the Executive's Q+2, TCQ) */
        range(m, 0u, 02000u, 03777u);
        range(m, 01u, 02000u, 03777u);              /* INTERPRETER, EXECUTIVE */
        m->stop[0].bank = 01u; m->stop[0].address = LM_EXEC_ADVAN; m->stops = 1u;
        m->range_list = lm_file_ranges; m->range_count = (uint16_t)(sizeof lm_file_ranges / sizeof lm_file_ranges[0]);
        /* the interpretive words with native routines (lanec_ev_lmi) */
        m->newops = 06070u;
        loc(m, 040u, LANEC_LM_RRLOSDSP + 4u, LANEC_LM_RRLOSDSP + 046u);
        loc(m, 042u, LANEC_LM_TRMTRAK1 + 7u, LANEC_LM_TRMTRAK1 + 9u);
        loc(m, 043u, LANEC_LM_ATTACHIT + 1u, LANEC_LM_ATTACHIT + 9u);
        loc(m, 043u, LANEC_LM_ATTACHIT + 16u, LANEC_LM_ATTACHIT + 21u);
        loc(m, 043u, LANEC_LM_TCPIN, LANEC_LM_TCPIN + 1u);
    } else {
        range(m, LANEC_ANY_BANK, 04662u, 04775u);
        range(m, LANEC_ANY_BANK, 05526u, 05545u);
        range(m, 010u, 02553u, 03765u);
        m->range_list = cm_file_ranges; m->range_count = (uint16_t)(sizeof cm_file_ranges / sizeof cm_file_ranges[0]);
    }
}

#ifndef LANEC_EV_TRANSFER_C64
/* The program's operand table: the INTER-BANK and Executive sites, merged in
 * physical order. */
static agc_site_operand lm_sites[1024];
static uint16_t lm_operand_count;
static const agc_site_operand *lm_operands(void)
{
    uint16_t na, nb, i = 0, j = 0;
    const agc_site_operand *a = lm_interbank_site_table(&na), *b = lm_exec_site_table(&nb);
    if (lm_operand_count) return lm_sites;
    while (i < na || j < nb)
        lm_sites[lm_operand_count++] = j == nb || (i < na && a[i].physical < b[j].physical) ? a[i++] : b[j++];
    return lm_sites;
}

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static agc_routine_table tables[6], interpretive[1];
    static agc_native_program program;
    static agc_hardware hardware;
    static lm_dntm_io lm;
    static cm_dntm_io cm;
    enum exec_status status;
    uint16_t steps = 0, count;
    memset(&program, 0, sizeof program);
    memset(&hardware, 0, sizeof hardware);
    program.basic = tables;
    program.interpreter_fetch = AGC_TRANSFER_NONE; program.loc = 0164u; program.bankset = 0165u;
    program.hardware = &hardware;
    hardware.channel11 = io->channel[011];
    if (mission == LANEC_LM) {
        tables[0] = lm_interbank_routines; tables[1] = lanec_di_lm_routines; tables[2] = lanec_ev_lm_routines;
        tables[3] = lm_exec_routines; tables[4] = lm_exec_job_routines; tables[5] = lanec_ev_lm_intpret;
        program.basic_count = 6u;
        interpretive[0] = lanec_ev_lm_interpretive;
        program.interpretive = interpretive; program.interpretive_count = 1u;
        program.interpreter_fetch = LM_INTERPRETER_FETCH;
        program.operands = lm_operands(); program.operand_count = lm_operand_count;
        memset(&lm, 0, sizeof lm);
        lm.control.superbank = io->superbank; lm.control.interrupt_enabled = io->interrupt_enabled;
        lm.channel12 = io->channel[012]; lm.channel13 = io->channel[013]; lm.channel14 = io->channel[014];
        lm.channel30 = io->channel[030]; lm.channel31 = io->channel[031];
        lm.channel32 = io->channel[032]; lm.channel33 = io->channel[033];
        status = agc_transfer_run(&program, s, &lm, &lm.control.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = lm.control.superbank; io->interrupt_enabled = lm.control.interrupt_enabled;
        io->channel[011] = hardware.channel11;
        io->channel[012] = lm.channel12; io->channel[013] = lm.channel13; io->channel[014] = lm.channel14;
        io->channel[030] = lm.channel30; io->channel[031] = lm.channel31;
        io->channel[032] = lm.channel32; io->channel[033] = lm.channel33;
    } else {
        tables[0] = cm_interbank_routines; tables[1] = lanec_di_cm_routines; tables[2] = lanec_ev_cm_routines;
        program.basic_count = 3u;
        program.operands = cm_interbank_site_table(&count); program.operand_count = count;
        memset(&cm, 0, sizeof cm);
        cm.io.superbank = io->superbank; cm.io.interrupt_enabled = io->interrupt_enabled;
        cm.io.channel11 = io->channel[011]; cm.io.channel12 = io->channel[012];
        cm.io.channel13 = io->channel[013]; cm.io.channel14 = io->channel[014];
        cm.channel30 = io->channel[030]; cm.channel31 = io->channel[031];
        cm.channel32 = io->channel[032]; cm.channel33 = io->channel[033];
        status = agc_transfer_run(&program, s, &cm, &cm.io.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = cm.io.superbank; io->interrupt_enabled = cm.io.interrupt_enabled;
        io->channel[011] = cm.io.channel11; io->channel[012] = cm.io.channel12;
        io->channel[013] = cm.io.channel13; io->channel[014] = cm.io.channel14;
        io->channel[030] = cm.channel30; io->channel[031] = cm.channel31;
        io->channel[032] = cm.channel32; io->channel[033] = cm.channel33;
    }
    /* at least the INTER-BANK entry and the extended-verb routine ran */
    return status == EXEC_UNMAPPED && steps >= 2u ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m) { make_case(&cases[n], s, io, m); }
const lanec_suite lanec_suite_ev_transfer = { "ev-transfer", LANEC_EV_TRANSFER_CASES, make, run };
#endif

/* ev-transfer-c64: the part of the round trip that fits one C64 program.
 * Pinball's POSTJUMP GOEXTVB for every verb of the Luminary099 front-suite
 * cases, through POSTJUMP and the verb fan to the verb's entry, with only the
 * INTER-BANK and front (lanec_ev_lm_front) routines registered. The oracle
 * also stops at every point of the other extended-verb modules, where the
 * native program has no routine. */
#define LANEC_EV_TRANSFER_C64_STOPS 512u
#ifndef LANEC_EV_TRANSFER_C64
static lanec_point c64_stops[LANEC_EV_TRANSFER_C64_STOPS];
#endif
static void make_c64(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    make_case(&c64_cases[n], s, io, m);
    m->ranges = 2u; m->stops = 0; m->newops = 0; m->locs = 0;   /* INTER-BANK and the file only */
#ifndef LANEC_EV_TRANSFER_C64           /* the oracle's stops (the C64 program has no oracle) */
    {
        uint16_t j, k = 0, p, f = 0;
        for (j = 0; j < lanec_ev_lm_routines.count && k < LANEC_EV_TRANSFER_C64_STOPS; ++j) {
            p = lanec_ev_lm_routines.entries[j].physical;
            while (f < lanec_ev_lm_front.count && lanec_ev_lm_front.entries[f].physical < p) ++f;
            if (f < lanec_ev_lm_front.count && lanec_ev_lm_front.entries[f].physical == p) continue;
            c64_stops[k].bank = (uint16_t)(p / 1024u); c64_stops[k].address = (uint16_t)(02000u + p % 1024u); ++k;
        }
        m->stop_list = c64_stops; m->stop_count = k;
    }
#endif
}

static enum lanec_status run_c64(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static agc_routine_table tables[2];
    static agc_native_program program;
    static agc_hardware hardware;
    static lm_dntm_io lm;
    enum exec_status status;
    uint16_t steps = 0, count;
    if (mission != LANEC_LM) return LANEC_OUT_OF_SCOPE;
    memset(&program, 0, sizeof program);
    memset(&hardware, 0, sizeof hardware);
    memset(&lm, 0, sizeof lm);
    tables[0] = lm_interbank_routines; tables[1] = lanec_ev_lm_front;
    program.basic = tables; program.basic_count = 2u;
    program.interpreter_fetch = AGC_TRANSFER_NONE; program.loc = 0164u; program.bankset = 0165u;
    program.hardware = &hardware;
    program.operands = lm_interbank_site_table(&count); program.operand_count = count;
    hardware.channel11 = io->channel[011];
    lm.control.superbank = io->superbank; lm.control.interrupt_enabled = io->interrupt_enabled;
    status = agc_transfer_run(&program, s, &lm, &lm.control.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
    io->superbank = lm.control.superbank; io->interrupt_enabled = lm.control.interrupt_enabled;
    io->channel[011] = hardware.channel11;
    return status == EXEC_UNMAPPED && steps >= 2u ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}
const lanec_suite lanec_suite_ev_transfer_c64 = { "ev-transfer-c64", LANEC_EV_TRANSFER_C64_CASES, make_c64, run_c64 };
