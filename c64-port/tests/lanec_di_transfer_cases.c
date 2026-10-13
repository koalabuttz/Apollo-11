/* SPDX-License-Identifier: MIT */
/* di-transfer: DISPLAY_INTERFACE_ROUTINES through the control-transfer ABI.
 * Each case starts at a genuine caller's TC BANKCALL or POSTJUMP (Z at the
 * INTER-BANK entry, Q at the caller's CADR, the caller's bank), with the rest
 * of the state taken from an accepted case of the destination's own suite.
 * The native side runs agc_transfer_run over a program of the mission's
 * INTER-BANK routines (lm_interbank_routines / cm_interbank_routines, with
 * their compiled site tables), the display-interface routines
 * (lanec_di_lm_routines / lanec_di_cm_routines) and, for Luminary099, the
 * Executive's job services (lm_exec_routines, lm_exec_job_routines, with
 * lm_exec_sites), and ends where a continuation has no registered routine
 * (EXEC_UNMAPPED). The oracle runs the original program until Z leaves the
 * words of those files (or reaches the Executive's idle loop ADVAN), so every
 * point at which the original continues inside them must be a registered
 * routine that reproduces it: back through PRIOCHNG or JOBSLEEP's return and
 * SWRETURN into the caller, or on to another job, Pinball or an untranslated
 * Executive service (Comanche055's are not registered yet). */
#include <string.h>
#include "lanec_suites.h"
#include "lanec_symbols.h"
#include "lanec_di_routines.h"
#include "lm_interbank_routines.h"
#include "lm_exec_routines.h"
#include "cm_interbank_routines.h"
#include "p66_bias.h"
#include "cm_native_image.h"

typedef struct { uint16_t site, callee; uint16_t n; uint8_t module; uint8_t mission; } transfer_case;
#define LANEC_DI_TRANSFER_C64_STOPS 128u
#include "lanec_di_transfer_cases.inc"
#ifndef LANEC_DI_TRANSFER_C64
static const lanec_suite *const modules[] = { &lanec_suite_di_dsp, &lanec_suite_di_mark, &lanec_suite_di_play,
                                              &lanec_suite_di_sleep, &lanec_suite_di_idle, &lanec_suite_di_nvdsp };
#else   /* the C64 program's cases all start from display-start cases */
static const lanec_suite *const modules[] = { &lanec_suite_di_dsp };
#endif

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static void make_case(const transfer_case *c, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t bank = (uint16_t)(c->site / 1024u), offset = (uint16_t)(c->site % 1024u), i;
    modules[c->module]->make(c->n, s, io, m);
    memset(m, 0, sizeof *m);
    m->mission = (enum lanec_mission)c->mission;
    if (bank == 2u || bank == 3u) {   /* fixed-fixed caller: keep the case's FB */
        lanec_enter(s, io, lanec_current_bank(s, io), c->callee);
        s->words[LANEC_Q] = (uint16_t)(04000u + (bank - 2u) * 02000u + offset);
    } else {
        lanec_enter(s, io, bank, c->callee);
        s->words[LANEC_Q] = (uint16_t)(02000u + offset);
    }
    range(m, LANEC_ANY_BANK, 0u, 7u);                /* TC Q, TC L: code in the central registers */
    if (m->mission == LANEC_LM) {
        /* One job (the caller, in core set 0, priority 20, no VAC area) and
         * no job change pending: the other seven core sets free (PRIORITY
         * -0), the five VAC areas free (each first word its own address),
         * NEWJOB +0. The display lock is clear or set (DSPLOCK +0 or 1), as
         * Pinball leaves it. */
        s->words[0167] = 020000u;
        s->words[LANEC_LM_DSPLOCK] &= 1u;
        for (i = 1u; i < 8u; ++i) s->words[0167u + i * 12u] = 077777u;
        for (i = 0; i < 5u; ++i) s->words[0400u + i * 44u] = (uint16_t)(0400u + i * 44u);
        s->words[067] = 0;
        range(m, LANEC_ANY_BANK, 04616u, 04731u);   /* INTER-BANK_COMMUNICATION */
        range(m, LANEC_ANY_BANK, 05464u, 05503u);   /* DISPLAY_INTERFACE_ROUTINES */
        range(m, 010u, 02326u, 03543u);
        range(m, LANEC_ANY_BANK, 05072u, 05172u);   /* EXECUTIVE */
        range(m, 01u, 02601u, 03230u);
        range(m, LANEC_ANY_BANK, 06741u, 06742u);   /* Q+2, TCQ: the Executive's return */
        m->stop[0].bank = 01u; m->stop[0].address = LM_EXEC_ADVAN; m->stops = 1u;
    } else {
        range(m, LANEC_ANY_BANK, 04662u, 04775u);
        range(m, LANEC_ANY_BANK, 05526u, 05545u);
        range(m, 010u, 02553u, 03765u);
    }
}

#ifndef LANEC_DI_TRANSFER_C64
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
    static agc_routine_table tables[4];
    static agc_native_program program;
    static agc_hardware hardware;
    static p66_bias_io lm;
    static cm_native_io cm;
    enum exec_status status;
    uint16_t steps = 0, count;
    memset(&program, 0, sizeof program);
    memset(&hardware, 0, sizeof hardware);
    program.basic = tables; program.basic_count = 2u;
    program.interpreter_fetch = AGC_TRANSFER_NONE; program.loc = 0164u; program.bankset = 0165u;
    program.hardware = &hardware;
    hardware.channel11 = io->channel[011];
    if (mission == LANEC_LM) {
        tables[0] = lm_interbank_routines; tables[1] = lanec_di_lm_routines;
        tables[2] = lm_exec_routines; tables[3] = lm_exec_job_routines; program.basic_count = 4u;
        program.operands = lm_operands(); program.operand_count = lm_operand_count;
        lm.superbank = io->superbank; lm.interrupt_enabled = io->interrupt_enabled;
        status = agc_transfer_run(&program, s, &lm, &lm.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = lm.superbank; io->interrupt_enabled = lm.interrupt_enabled;
        io->channel[011] = hardware.channel11;
    } else {
        tables[0] = cm_interbank_routines; tables[1] = lanec_di_cm_routines;
        program.operands = cm_interbank_site_table(&count); program.operand_count = count;
        memset(&cm, 0, sizeof cm);
        cm.superbank = io->superbank; cm.interrupt_enabled = io->interrupt_enabled; cm.channel11 = io->channel[011];
        status = agc_transfer_run(&program, s, &cm, &cm.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = cm.superbank; io->interrupt_enabled = cm.interrupt_enabled;
        io->channel[011] = cm.channel11;
    }
    /* at least the INTER-BANK entry and the display-interface routine ran */
    return status == EXEC_UNMAPPED && steps >= 2u ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m) { make_case(&cases[n], s, io, m); }
const lanec_suite lanec_suite_di_transfer = { "di-transfer", LANEC_DI_TRANSFER_CASES, make, run };
#endif

/* di-transfer-c64: the part of the round trip that fits one C64 program.
 * Luminary099 callers of the display-start entries (BANKCALL GOFLASH,
 * GODSP, PRIODSP...) through SAVELOCS, MAKECADR and BOTHJOBS to MAKEPLAY
 * and BANKJUMP, with only the INTER-BANK, display-start (lanec_di_dsp) and
 * MAKEPLAY (lanec_di_play) routines registered. The oracle also stops at
 * every point of the other four modules, where the native program has no
 * routine. */
#ifndef LANEC_DI_TRANSFER_C64
static lanec_point c64_stops[LANEC_DI_TRANSFER_C64_STOPS];
#endif
static void make_c64(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    make_case(&c64_cases[n], s, io, m);
    m->ranges = 4u; m->stops = 0;       /* no Executive routines in this program */
#ifndef LANEC_DI_TRANSFER_C64           /* the oracle's stops (the C64 program has no oracle) */
    {
        const agc_routine_table *other[4] = { &lanec_di_lm_mark, &lanec_di_lm_sleep, &lanec_di_lm_idle, &lanec_di_lm_nvdsp };
        uint16_t i, j, k = 0, p;
        for (i = 0; i < 4u; ++i)
            for (j = 0; j < other[i]->count && k < LANEC_DI_TRANSFER_C64_STOPS; ++j, ++k) {
                p = other[i]->entries[j].physical;
                c64_stops[k].bank = p < 4096u ? LANEC_ANY_BANK : (uint16_t)(p / 1024u);
                c64_stops[k].address = (uint16_t)(p < 4096u ? 04000u + (p - 2048u) : 02000u + p % 1024u);
            }
        m->stop_list = c64_stops; m->stop_count = k;
    }
#endif
}

static enum lanec_status run_c64(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static agc_routine_table tables[3];
    static agc_native_program program;
    static agc_hardware hardware;
    static p66_bias_io lm;
    enum exec_status status;
    uint16_t steps = 0, count;
    if (mission != LANEC_LM) return LANEC_OUT_OF_SCOPE;
    memset(&program, 0, sizeof program);
    memset(&hardware, 0, sizeof hardware);
    tables[0] = lm_interbank_routines; tables[1] = lanec_di_lm_dsp; tables[2] = lanec_di_lm_play;
    program.basic = tables; program.basic_count = 3u;
    program.interpreter_fetch = AGC_TRANSFER_NONE; program.loc = 0164u; program.bankset = 0165u;
    program.hardware = &hardware;
    program.operands = lm_interbank_site_table(&count); program.operand_count = count;
    hardware.channel11 = io->channel[011];
    lm.superbank = io->superbank; lm.interrupt_enabled = io->interrupt_enabled;
    status = agc_transfer_run(&program, s, &lm, &lm.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
    io->superbank = lm.superbank; io->interrupt_enabled = lm.interrupt_enabled;
    io->channel[011] = hardware.channel11;
    return status == EXEC_UNMAPPED && steps >= 2u ? LANEC_OK : LANEC_OUT_OF_SCOPE;
}
const lanec_suite lanec_suite_di_transfer_c64 = { "di-transfer-c64", LANEC_DI_TRANSFER_C64_CASES, make_c64, run_c64 };
