/* SPDX-License-Identifier: MIT */
/* pin-transfer: PINBALL_GAME_BUTTONS_AND_LIGHTS through the control-transfer
 * ABI. Two kinds of case:
 *  - a genuine caller outside the file whose TC BANKCALL or POSTJUMP names a
 *    Pinball routine: where a suite case runs from its own entry to that TC,
 *    the case starts there with that case's state; otherwise at the TC
 *    itself (Z at the INTER-BANK entry, Q at the caller's CADR, the caller's
 *    bank), with the rest of the state from an accepted case of the first
 *    suite that claims the destination;
 *  - every registered routine from its own entry, with accepted cases of the
 *    first module it tries (module index | 0200), chained through the
 *    routines that follow it.
 * Channels the device records do not carry (all but 11-14, CM 11-16) are 0.
 *
 * The native side runs agc_transfer_run over a program of the mission's
 * INTER-BANK routines, the display-interface routines, the Pinball routines
 * (lanec_pin_lm_routines / lanec_pin_cm_routines) and, for Luminary099, the
 * Executive's job services, and ends where a continuation has no registered
 * routine (EXEC_UNMAPPED). The oracle runs the original program until Z
 * leaves the words of those files and the other files' code the modules run
 * inline, or reaches a service that hands control away for good (ENDOFJOB,
 * ...; LM also the Executive's idle loop ADVAN). A chain that ends where the
 * oracle would go on (a point inside those words with no routine, reached
 * through a random return address or cell) is refused with the state
 * restored, and so is a chain whose routine refuses its state midway. */
#include <string.h>
#include "lanec_suites.h"
#include "lanec_symbols.h"
#include "lanec_di_routines.h"
#include "lanec_pin_routines.h"
#include "lm_interbank_routines.h"
#include "lm_exec_routines.h"
#include "cm_interbank_routines.h"
#include "lm_downtelemetry.h"
#include "cm_downtelemetry.h"

typedef struct { uint16_t site, callee; uint16_t n; uint8_t module; uint8_t mission; } transfer_case;
#include "lanec_pin_transfer_cases.inc"
#ifndef LANEC_PIN_TRANSFER_C64
#define MODULE(i) modules[i]
#else   /* a C64 program runs the cases of one module's suite */
#define MODULE(i) (&LANEC_PIN_C64_SUITE)
#endif

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

/* The file's words, the code of other files its modules run inline
 * (external_ranges), the erasable return cells Pinball executes with a TC
 * (lanec_pin_cells) while they hold a TC or a TCF into fixed memory
 * (LANEC_TC_CELL), and the INTER-BANK return cell BUF2 (SWRETURN's TC BUF2). */
#define LANEC_PIN_COUNT(a) (sizeof a / sizeof a[0])
#define LANEC_PIN_LM_RANGES (LANEC_PIN_COUNT(lm_file_ranges) + LANEC_PIN_COUNT(lm_external_ranges) + LANEC_PIN_CELLS + 1u)
#define LANEC_PIN_CM_RANGES (LANEC_PIN_COUNT(cm_file_ranges) + LANEC_PIN_COUNT(cm_external_ranges) + LANEC_PIN_CELLS + 1u)
#ifdef LANEC_PIN_HAS_LM
static lanec_range lm_ranges[LANEC_PIN_LM_RANGES];
#endif
#ifdef LANEC_PIN_HAS_CM
static lanec_range cm_ranges[LANEC_PIN_CM_RANGES];
#endif
static void ranges_of(lanec_meta *m, lanec_range *out, const lanec_range *file, uint16_t files,
                      const lanec_range *external, uint16_t externals)
{
    uint16_t i, k = (uint16_t)(files + externals);
    memcpy(out, file, files * sizeof *file);
    memcpy(out + files, external, externals * sizeof *external);
    for (i = 0; i <= LANEC_PIN_CELLS; ++i, ++k) {
        out[k].bank = LANEC_TC_CELL;
        out[k].lo = out[k].hi = i < LANEC_PIN_CELLS ? lanec_pin_cells[i] : 0133u;
    }
    m->range_list = out; m->range_count = k;
}

/* The oracle stops where a module hands control to another file's code
 * (external_exits: ENDOFJOB, JOBSLEEP, ...) unless the program registers a
 * routine there (the Executive's, INTER-BANK's, ...). */
#ifdef LANEC_PIN_HAS_LM
static lanec_point lm_stops[LANEC_PIN_COUNT(lm_external_exits)];
#endif
#ifdef LANEC_PIN_HAS_CM
static lanec_point cm_stops[LANEC_PIN_COUNT(cm_external_exits)];
#endif
#ifndef LANEC_PIN_TRANSFER_C64
static int registered(const agc_routine_table *const *tables, uint16_t physical)
{
    uint16_t i;
    for (; *tables; ++tables)
        for (i = 0; i < (*tables)->count; ++i)
            if ((*tables)->entries[i].physical == physical) return 1;
    return 0;
}
static const agc_routine_table *const lm_program[] = { &lm_interbank_routines, LANEC_DI_LM_TABLES, &lm_exec_routines,
                                                       &lm_exec_job_routines, 0 };
static const agc_routine_table *const cm_program[] = { &cm_interbank_routines, LANEC_DI_CM_TABLES, 0 };
#endif
static void stops_of(lanec_meta *m, lanec_point *out, const lanec_point *exits, uint16_t n)
{
    uint16_t i, k = 0, physical;
    for (i = 0; i < n; ++i) {
        physical = exits[i].bank == LANEC_ANY_BANK ? (uint16_t)(2048u + exits[i].address - 04000u)
                                                   : (uint16_t)(exits[i].bank * 1024u + exits[i].address - 02000u);
#ifndef LANEC_PIN_TRANSFER_C64
        if (registered(m->mission == LANEC_LM ? lm_program : cm_program, physical)) continue;
#else
        (void)physical;
#endif
        out[k++] = exits[i];
    }
    m->stop_list = out; m->stop_count = k;
}

static uint16_t min_steps;   /* routines the case must run: set by make, read by run */

/* The case's oracle boundaries, kept by make for run. */
static const lanec_meta *cur_meta;

static int tc_cell(uint16_t z, uint16_t w)
{
    return (w <= 07777u && w != 3u && w != 4u && w != 6u && w != z) || (w >= 012000u && w <= 017777u);
}

static int in_range(const lanec_range *r, const apollo_overlay_state *s, uint16_t z, uint16_t fb)
{
    return z >= r->lo && z <= r->hi && (r->bank == LANEC_ANY_BANK || (z >= 02000u && z < 04000u && r->bank == fb) ||
                                        (r->bank == LANEC_TC_CELL && tc_cell(z, s->words[z])));
}

/* The chain stopped where the original program goes on: not at one of the
 * case's stops and inside one of its ranges (the file at a point no routine
 * is registered for, another file's code that the earlier modules ran
 * inline, the Executive after a return into its middle). The ABI has no
 * routine there, so the case is out of scope rather than a result. */
static int unregistered_call(const apollo_overlay_state *s, const lanec_io *io)
{
    const lanec_meta *m = cur_meta;
    uint16_t z = s->words[LANEC_Z], fb = lanec_current_bank(s, io), i;
    for (i = 0; i < m->stops; ++i)
        if (m->stop[i].address == z && (m->stop[i].bank == LANEC_ANY_BANK || m->stop[i].bank == fb) &&
            !(i == 0 && m->stop_q && s->words[LANEC_Q] != m->stop_q_value)) return 0;
    for (i = 0; i < m->stop_count; ++i)
        if (m->stop_list[i].address == z && (m->stop_list[i].bank == LANEC_ANY_BANK || m->stop_list[i].bank == fb)) return 0;
    for (i = 0; i < m->ranges; ++i)
        if (in_range(&m->range[i], s, z, fb)) return 1;
    for (i = 0; i < m->range_count; ++i)
        if (in_range(&m->range_list[i], s, z, fb)) return 1;
    return 0;
}

static void make_case(const transfer_case *c, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t bank = (uint16_t)(c->site / 1024u), offset = (uint16_t)(c->site % 1024u), i;
    MODULE(c->module & 0177u)->make(c->n, s, io, m);
    for (i = 0; i < LANEC_CHANNELS; ++i)    /* the device record carries channels 11-14 (CM 11-16) only */
        if (i < 011u || i > (c->mission == LANEC_LM ? 014u : 016u)) io->channel[i] = 0;
    memset(m, 0, sizeof *m);
    m->mission = (enum lanec_mission)c->mission;
    min_steps = c->module & 0200u ? 1u : 2u;
    if (c->module & 0200u) {          /* a routine's own case, from its entry */
    } else if (bank == 2u || bank == 3u) {   /* fixed-fixed caller: keep the case's FB */
        lanec_enter(s, io, lanec_current_bank(s, io), c->callee);
        s->words[LANEC_Q] = (uint16_t)(04000u + (bank - 2u) * 02000u + offset);
    } else {
        lanec_enter(s, io, bank, c->callee);
        s->words[LANEC_Q] = (uint16_t)(02000u + offset);
    }
    range(m, LANEC_TC_CELL, 0u, 7u);                 /* TC Q, TC L: a return held in a register */
#ifdef LANEC_PIN_HAS_LM
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
        range(m, 01u, 02601u, 03230u);
        range(m, LANEC_ANY_BANK, 06741u, 06742u);   /* Q+2, TCQ: the Executive's return */
        m->stop[0].bank = 01u; m->stop[0].address = LM_EXEC_ADVAN; m->stops = 1u;
        ranges_of(m, lm_ranges, lm_file_ranges, LANEC_PIN_COUNT(lm_file_ranges), lm_external_ranges, LANEC_PIN_COUNT(lm_external_ranges));
        stops_of(m, lm_stops, lm_external_exits, LANEC_PIN_COUNT(lm_external_exits));
    }
#endif
#ifdef LANEC_PIN_HAS_CM
    if (m->mission == LANEC_CM) {
        range(m, LANEC_ANY_BANK, 04662u, 04775u);
        range(m, LANEC_ANY_BANK, 05526u, 05545u);
        range(m, 010u, 02553u, 03765u);
        ranges_of(m, cm_ranges, cm_file_ranges, LANEC_PIN_COUNT(cm_file_ranges), cm_external_ranges, LANEC_PIN_COUNT(cm_external_ranges));
        stops_of(m, cm_stops, cm_external_exits, LANEC_PIN_COUNT(cm_external_exits));
    }
#endif
}

#ifndef LANEC_PIN_TRANSFER_C64
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

static const agc_routine_table *const lm_di[LANEC_DI_TABLE_COUNT] = { LANEC_DI_LM_TABLES };
static const agc_routine_table *const cm_di[LANEC_DI_TABLE_COUNT] = { LANEC_DI_CM_TABLES };

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static agc_routine_table tables[4 + LANEC_DI_TABLE_COUNT];
    static agc_native_program program;
    static agc_hardware hardware;
    static lm_dntm_io lm;
    static cm_dntm_io cm;
    static apollo_overlay_state entry;
    static lanec_io entry_io;
    enum exec_status status;
    uint16_t steps = 0, count;
    entry = *s; entry_io = *io;
    memset(&program, 0, sizeof program);
    memset(&hardware, 0, sizeof hardware);
    program.basic = tables;
    program.interpreter_fetch = AGC_TRANSFER_NONE; program.loc = 0164u; program.bankset = 0165u;
    program.hardware = &hardware;
    hardware.channel11 = io->channel[011];
    if (mission == LANEC_LM) {
        tables[0] = lm_interbank_routines; program.basic_count = (uint8_t)(1u + lanec_put_tables(tables + 1, lm_di, LANEC_DI_TABLE_COUNT));
        tables[program.basic_count++] = lanec_pin_lm_routines;
        tables[program.basic_count++] = lm_exec_routines; tables[program.basic_count++] = lm_exec_job_routines;
        program.operands = lm_operands(); program.operand_count = lm_operand_count;
        memset(&lm, 0, sizeof lm);
        lm.control.superbank = io->superbank; lm.control.interrupt_enabled = io->interrupt_enabled;
        lm.channel12 = io->channel[012]; lm.channel13 = io->channel[013]; lm.channel14 = io->channel[014];
        status = agc_transfer_run(&program, s, &lm, &lm.control.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = lm.control.superbank; io->interrupt_enabled = lm.control.interrupt_enabled;
        io->channel[011] = hardware.channel11;
        io->channel[012] = lm.channel12; io->channel[013] = lm.channel13; io->channel[014] = lm.channel14;
    } else {
        tables[0] = cm_interbank_routines; program.basic_count = (uint8_t)(1u + lanec_put_tables(tables + 1, cm_di, LANEC_DI_TABLE_COUNT));
        tables[program.basic_count++] = lanec_pin_cm_routines;
        program.operands = cm_interbank_site_table(&count); program.operand_count = count;
        memset(&cm, 0, sizeof cm);
        cm.io.superbank = io->superbank; cm.io.interrupt_enabled = io->interrupt_enabled;
        cm.io.channel11 = io->channel[011]; cm.io.channel12 = io->channel[012];
        cm.io.channel13 = io->channel[013]; cm.io.channel14 = io->channel[014];
        cm.io.main_keys = io->channel[015]; cm.io.navigation_keys = io->channel[016];
        status = agc_transfer_run(&program, s, &cm, &cm.io.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = cm.io.superbank; io->interrupt_enabled = cm.io.interrupt_enabled;
        io->channel[011] = cm.io.channel11; io->channel[012] = cm.io.channel12;
        io->channel[013] = cm.io.channel13; io->channel[014] = cm.io.channel14;
        io->channel[015] = cm.io.main_keys; io->channel[016] = cm.io.navigation_keys;
    }
    /* at least the INTER-BANK entry and a Pinball routine ran (from a site),
     * or the routine the case enters (from an entry). A chain that reaches a
     * routine refusing its state (an input outside that routine's domain)
     * is refused whole, with the state as it was. */
    if (status == EXEC_UNMAPPED && steps >= min_steps && !unregistered_call(s, io)) return LANEC_OK;
    *s = entry; *io = entry_io;
    return LANEC_OUT_OF_SCOPE;
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m) { make_case(&cases[n], s, io, m); cur_meta = m; }
const lanec_suite lanec_suite_pin_transfer = { "pin-transfer", LANEC_PIN_TRANSFER_CASES, make, run };

/* pin-refuse: entry states outside the routines' domain. Each case takes a
 * routine's own-entry case of pin-transfer (every routine in turn, both
 * missions; 510 routines) and breaks the register contract every Pinball routine checks
 * first (lanec_registers_valid): BB not FB|EB>>8, the ZERO register nonzero,
 * or FB selecting bank 7, where the file has no code (no routine is
 * registered there). The chain must be refused with the image and I/O
 * unchanged; diff.py --refusals-only. */
static uint16_t own[LANEC_PIN_TRANSFER_CASES], owns;
static void make_refuse(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint32_t i = n / 3u;
    uint16_t *w = s->words, z, k;
    if (!owns)
        for (k = 0; k < LANEC_PIN_TRANSFER_CASES; ++k)
            if (cases[k].module & 0200u) own[owns++] = k;
    make_case(&cases[own[(i * 4u + n % 3u) % owns]], s, io, m);   /* 4 cases per routine */
    cur_meta = m;
    z = w[LANEC_Z];
    switch (n % 3u) {
    case 0: w[LANEC_BB] ^= 1u; break;                              /* BB disagrees with EB */
    case 1: w[7] = (uint16_t)(1u + i % 077776u); break;            /* ZERO nonzero */
    default:
        if (z >= 02000u && z < 04000u) lanec_enter(s, io, 07u, z);   /* bank 7 */
        else w[LANEC_BB] ^= 1u;
        break;
    }
}
const lanec_suite lanec_suite_pin_refuse = { "pin-refuse", 3u * 510u, make_refuse, run };
#endif

/* pin-transfer-c64: the round trips as the C64 runs them. Every module
 * does not fit one C64 program, so each program registers INTER-BANK and one
 * module's points (those it claims first, lanec_pin_c64_select) and runs that
 * module alone, both missions. The cases are every routine from its own
 * entry with cases of that module. The oracle has the host cases' ranges and
 * stops at every service that hands control away (the program has no
 * Executive). A chain that ends where the original goes on (at another
 * module's point, the display interface or the Executive, which the program
 * lacks) is refused, as in pin-transfer; the host suite compares those. The
 * other modules' points are not stops: a module runs through some of them
 * inline. */
static const transfer_case *c64_case;
#ifdef LANEC_PIN_TRANSFER_C64
static uint8_t c64_skip;
int lanec_pin_c64_skip(void) { return c64_skip; }
#endif

static void make_c64(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    const transfer_case *c;
    uint8_t module;
#ifdef LANEC_PIN_TRANSFER_C64
    uint16_t i;
    for (i = 0; i < LANEC_PIN_COUNT(c64_frozen) && c64_frozen[i].n != n; ++i) {}
    c64_skip = i == LANEC_PIN_COUNT(c64_frozen);   /* the program has its module's rows only */
    if (c64_skip) return;
    c = &c64_frozen[i].c;
#else
    c = &c64_cases[n];
#endif
    module = (uint8_t)(c->module & 0177u);
    c64_case = c;
    make_case(c, s, io, m);
    lanec_pin_c64_select(module, m->mission == LANEC_CM);
    m->stop_list = m->mission == LANEC_LM ? lm_external_exits : cm_external_exits;
    m->stop_count = m->mission == LANEC_LM ? LANEC_PIN_COUNT(lm_external_exits) : LANEC_PIN_COUNT(cm_external_exits);
    cur_meta = m;
}

static enum lanec_status run_c64(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static agc_routine_table tables[2];
    static agc_native_program program;
    static agc_hardware hardware;
    static lanec_meta again;
    enum exec_status status;
    uint16_t steps = 0, count;
    memset(&program, 0, sizeof program);
    memset(&hardware, 0, sizeof hardware);
    tables[1] = lanec_pin_c64;
    program.basic = tables; program.basic_count = 2u;
    program.interpreter_fetch = AGC_TRANSFER_NONE; program.loc = 0164u; program.bankset = 0165u;
    program.hardware = &hardware;
    hardware.channel11 = io->channel[011];
#ifndef LANEC_PIN_C64_CM   /* a C64 program runs one mission */
    if (mission == LANEC_LM) {
        static lm_dntm_io lm;
        tables[0] = lm_interbank_routines;
        program.operands = lm_interbank_site_table(&count); program.operand_count = count;
        memset(&lm, 0, sizeof lm);
        lm.control.superbank = io->superbank; lm.control.interrupt_enabled = io->interrupt_enabled;
        lm.channel12 = io->channel[012]; lm.channel13 = io->channel[013]; lm.channel14 = io->channel[014];
        status = agc_transfer_run(&program, s, &lm, &lm.control.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = lm.control.superbank; io->interrupt_enabled = lm.control.interrupt_enabled;
        io->channel[011] = hardware.channel11;
        io->channel[012] = lm.channel12; io->channel[013] = lm.channel13; io->channel[014] = lm.channel14;
    } else
#endif
#if !defined(LANEC_PIN_C64) || defined(LANEC_PIN_C64_CM)
    if (mission == LANEC_CM) {
        static cm_dntm_io cm;
        tables[0] = cm_interbank_routines;
        program.operands = cm_interbank_site_table(&count); program.operand_count = count;
        memset(&cm, 0, sizeof cm);
        cm.io.superbank = io->superbank; cm.io.interrupt_enabled = io->interrupt_enabled;
        cm.io.channel11 = io->channel[011]; cm.io.channel12 = io->channel[012];
        cm.io.channel13 = io->channel[013]; cm.io.channel14 = io->channel[014];
        cm.io.main_keys = io->channel[015]; cm.io.navigation_keys = io->channel[016];
        status = agc_transfer_run(&program, s, &cm, &cm.io.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = cm.io.superbank; io->interrupt_enabled = cm.io.interrupt_enabled;
        io->channel[011] = cm.io.channel11; io->channel[012] = cm.io.channel12;
        io->channel[013] = cm.io.channel13; io->channel[014] = cm.io.channel14;
        io->channel[015] = cm.io.main_keys; io->channel[016] = cm.io.navigation_keys;
    } else
#endif
        status = EXEC_OUT_OF_SCOPE;
    if (status == EXEC_UNMAPPED && steps >= min_steps && !unregistered_call(s, io)) return LANEC_OK;
    memset(&again, 0, sizeof again);   /* refused: the case's entry state again */
    make_case(c64_case, s, io, &again);
    return LANEC_OUT_OF_SCOPE;
}
const lanec_suite lanec_suite_pin_transfer_c64 = { "pin-transfer-c64", LANEC_PIN_TRANSFER_C64_CASES, make_c64, run_c64 };
