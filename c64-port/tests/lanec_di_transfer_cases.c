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

/* site: the caller's physical address, or with module | 0200 the routine's
 * own (the case starts at the routine's entry, as the module's case does). */
typedef struct { uint16_t site, callee; uint16_t n; uint8_t module; uint8_t mission; } transfer_case;
#include "lanec_di_transfer_cases.inc"
#if defined(LANEC_DI_TRANSFER_C64)
/* the C64 program's cases all start from display-start or MAKEPLAY cases */
static const lanec_suite *const modules[] = { &lanec_suite_di_dsp, 0, &lanec_suite_di_play };
#elif defined(LANEC_DI_TRANSFER_C64M)
/* a di-transfer-c64m program's cases all start from its one module's cases
 * (LANEC_DI_C64M_MODULE: its index in module order, LANEC_DI_C64M_SUITE) */
static const lanec_suite *const modules[] = {
    LANEC_DI_C64M_MODULE == 0 ? &LANEC_DI_C64M_SUITE : 0, LANEC_DI_C64M_MODULE == 1 ? &LANEC_DI_C64M_SUITE : 0,
    LANEC_DI_C64M_MODULE == 2 ? &LANEC_DI_C64M_SUITE : 0, LANEC_DI_C64M_MODULE == 3 ? &LANEC_DI_C64M_SUITE : 0,
    LANEC_DI_C64M_MODULE == 4 ? &LANEC_DI_C64M_SUITE : 0, LANEC_DI_C64M_MODULE == 5 ? &LANEC_DI_C64M_SUITE : 0 };
#else
static const lanec_suite *const modules[] = { &lanec_suite_di_dsp, &lanec_suite_di_mark, &lanec_suite_di_play,
                                              &lanec_suite_di_sleep, &lanec_suite_di_idle, &lanec_suite_di_nvdsp };
#endif

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static void make_case(const transfer_case *c, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t bank = (uint16_t)(c->site / 1024u), offset = (uint16_t)(c->site % 1024u), i;
    modules[c->module & 0177u]->make(c->n, s, io, m);
    memset(m, 0, sizeof *m);
    m->mission = (enum lanec_mission)c->mission;
    if (c->module & 0200u) {          /* the routine's own entry: the module case as it is */
    } else if (bank == 2u || bank == 3u) {   /* fixed-fixed caller: keep the case's FB */
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

#if !defined(LANEC_DI_TRANSFER_C64) && !defined(LANEC_DI_TRANSFER_C64M)
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
 * Every Luminary099 and Comanche055 caller of the display-start and MAKEPLAY
 * entries (BANKCALL GOFLASH, GODSP, PRIODSP, CLOCPLAY...), through SAVELOCS,
 * MAKECADR and BOTHJOBS to MAKEPLAY and BANKJUMP, with only the INTER-BANK,
 * display-start (lanec_di_dsp) and MAKEPLAY (lanec_di_play) routines
 * registered. The oracle also stops at every point of the other four
 * modules, where the native program has no routine, and (as the C64 program
 * has no Executive) at none of the Executive's. The C64 rows are picked by
 * branch arc (tools/lanec/cover.py). */
static uint8_t c64_entry;              /* the case starts at the routine's own entry */
static const lanec_meta *c64_meta;

/* The chain stopped where the original goes on: inside the case's ranges
 * (INTER-BANK or the file, at a point no routine of this program claims,
 * reached through a random return address) and not at a stop. The ABI has no
 * routine there, so the case is out of scope rather than a result. */
static int unregistered_c64(const apollo_overlay_state *s, const lanec_io *io)
{
    const lanec_meta *m = c64_meta;
    uint16_t z = s->words[LANEC_Z], fb = lanec_current_bank(s, io), i;
    for (i = 0; i < m->stop_count; ++i)
        if (m->stop_list[i].address == z && (m->stop_list[i].bank == LANEC_ANY_BANK || m->stop_list[i].bank == fb)) return 0;
    for (i = 0; i < m->ranges; ++i)
        if (z >= m->range[i].lo && z <= m->range[i].hi &&
            (m->range[i].bank == LANEC_ANY_BANK || (z >= 02000u && z < 04000u && m->range[i].bank == fb))) return 1;
    return 0;
}

#ifndef LANEC_DI_TRANSFER_C64M
/* The C64 build runs one mission per program (both do not fit one):
 * LANEC_DI_C64_CM selects Comanche055, otherwise Luminary099. */
#ifndef LANEC_DI_TRANSFER_C64
#define C64_LM 1
#define C64_CM 1
#elif defined(LANEC_DI_C64_CM)
#define C64_LM 0
#define C64_CM 1
#else
#define C64_LM 1
#define C64_CM 0
#endif
static const transfer_case *c64_case;
static uint8_t c64_skip;               /* the other mission's row (tools/lanec/c64_diag.c skips it) */
int lanec_di_c64_skip(void);
int lanec_di_c64_skip(void) { return c64_skip; }

static void make_c64(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
#ifdef LANEC_DI_TRANSFER_C64            /* the C64 program carries its mission's frozen rows only */
    uint16_t i = 0;
    while (i < sizeof c64_frozen / sizeof c64_frozen[0] && c64_frozen[i].n != n) ++i;
    c64_skip = i == sizeof c64_frozen / sizeof c64_frozen[0];
    if (c64_skip) return;
    c64_case = &c64_frozen[i].c;
#else
    c64_case = &c64_cases[n];
#endif
    make_case(c64_case, s, io, m);
    c64_entry = (uint8_t)(c64_case->module & 0200u);
    c64_meta = m;
    m->ranges = 4u; m->stops = 0;       /* no Executive routines in this program */
    /* the other four modules' points: the oracle stops there */
    m->stop_list = m->mission == LANEC_CM ? c64_stops_cm : c64_stops_lm;
    m->stop_count = (uint16_t)(m->mission == LANEC_CM ? sizeof c64_stops_cm / sizeof c64_stops_cm[0]
                                                       : sizeof c64_stops_lm / sizeof c64_stops_lm[0]);
}

static enum lanec_status run_c64(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static agc_routine_table tables[3];
    static agc_native_program program;
    static agc_hardware hardware;
    static p66_bias_io lm;
    static cm_native_io cm;
    enum exec_status status = EXEC_OUT_OF_SCOPE;
    uint16_t steps = 0, count;
    memset(&program, 0, sizeof program);
    memset(&hardware, 0, sizeof hardware);
    program.basic = tables; program.basic_count = 3u;
    program.interpreter_fetch = AGC_TRANSFER_NONE; program.loc = 0164u; program.bankset = 0165u;
    program.hardware = &hardware;
    hardware.channel11 = io->channel[011];
    if (C64_LM && mission == LANEC_LM) {
        tables[0] = lm_interbank_routines; tables[1] = lanec_di_lm_dsp; tables[2] = lanec_di_lm_play;
        program.operands = lm_interbank_site_table(&count); program.operand_count = count;
        lm.superbank = io->superbank; lm.interrupt_enabled = io->interrupt_enabled;
        status = agc_transfer_run(&program, s, &lm, &lm.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = lm.superbank; io->interrupt_enabled = lm.interrupt_enabled;
        io->channel[011] = hardware.channel11;
    } else if (C64_CM && mission == LANEC_CM) {
        tables[0] = cm_interbank_routines; tables[1] = lanec_di_cm_dsp; tables[2] = lanec_di_cm_play;
        program.operands = cm_interbank_site_table(&count); program.operand_count = count;
        memset(&cm, 0, sizeof cm);
        cm.superbank = io->superbank; cm.interrupt_enabled = io->interrupt_enabled; cm.channel11 = io->channel[011];
        status = agc_transfer_run(&program, s, &cm, &cm.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, &steps);
        io->superbank = cm.superbank; io->interrupt_enabled = cm.interrupt_enabled;
        io->channel[011] = cm.channel11;
    }
    /* a call site's chain runs at least the INTER-BANK entry and the routine */
    if (status == EXEC_UNMAPPED && steps >= (c64_entry ? 1u : 2u) && !unregistered_c64(s, io)) return LANEC_OK;
    {   /* refused: the entry state again (a copy does not fit the C64 program) */
        static lanec_meta again;
        memset(&again, 0, sizeof again);
        make_case(c64_case, s, io, &again);
    }
    return LANEC_OUT_OF_SCOPE;
}
const lanec_suite lanec_suite_di_transfer_c64 = { "di-transfer-c64", LANEC_DI_TRANSFER_C64_CASES, make_c64, run_c64 };
#endif

#ifndef LANEC_DI_TRANSFER_C64
/* di-transfer-c64m: the round trips into the other four modules (mark, sleep,
 * idle, nvdsp), which di-transfer-c64's program has no room for. Every
 * Luminary099 and Comanche055 caller of their entries (POSTJUMP PINBRNCH and
 * MARKEND, BANKCALL GOMARK*, ENDMARK, KLEENEX, INITDSP...) and every one of
 * their points from its own entry, with only INTER-BANK and the case's own
 * module registered: one C64 program per module and mission
 * (LANEC_DI_C64M_<MODULE>_<LM|CM>, with LANEC_DI_C64M_TABLE and
 * LANEC_DI_C64M_STOPS naming its table and stops). The oracle stops at every
 * point of the other five modules. The C64 rows are picked by branch arc
 * (tools/lanec/cover.py). */
static const transfer_case *c64m_case;
static uint8_t c64m_skip;              /* a row of another program (tools/lanec/c64_diag.c skips it) */
int lanec_di_c64m_skip(void);
int lanec_di_c64m_skip(void) { return c64m_skip; }

#ifndef LANEC_DI_TRANSFER_C64M
/* the host runs every program's cases: the case's module's table and stops */
static const agc_routine_table *const c64m_lm_tables[6] = { 0, &lanec_di_lm_mark, 0, &lanec_di_lm_sleep,
                                                            &lanec_di_lm_idle, &lanec_di_lm_nvdsp };
static const agc_routine_table *const c64m_cm_tables[6] = { 0, &lanec_di_cm_mark, 0, &lanec_di_cm_sleep,
                                                            &lanec_di_cm_idle, &lanec_di_cm_nvdsp };
#define C64M_STOPS(v) { v, (uint16_t)(sizeof v / sizeof v[0]) }
static const struct { const lanec_point *list; uint16_t count; } c64m_lm_stops[6] = {
    { 0, 0 }, C64M_STOPS(c64m_stops_lm_mark), { 0, 0 }, C64M_STOPS(c64m_stops_lm_sleep),
    C64M_STOPS(c64m_stops_lm_idle), C64M_STOPS(c64m_stops_lm_nvdsp) };
static const struct { const lanec_point *list; uint16_t count; } c64m_cm_stops[6] = {
    { 0, 0 }, C64M_STOPS(c64m_stops_cm_mark), { 0, 0 }, C64M_STOPS(c64m_stops_cm_sleep),
    C64M_STOPS(c64m_stops_cm_idle), C64M_STOPS(c64m_stops_cm_nvdsp) };
#endif

static void make_c64m(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
#ifdef LANEC_DI_TRANSFER_C64M          /* the program carries its own frozen rows only */
    uint16_t i = 0;
    while (i < sizeof c64m_frozen / sizeof c64m_frozen[0] && c64m_frozen[i].n != n) ++i;
    c64m_skip = i == sizeof c64m_frozen / sizeof c64m_frozen[0];
    if (c64m_skip) return;
    c64m_case = &c64m_frozen[i].c;
#else
    c64m_case = &c64m_cases[n];
#endif
    make_case(c64m_case, s, io, m);
    c64_entry = (uint8_t)(c64m_case->module & 0200u);
    c64_meta = m;
    m->ranges = 4u; m->stops = 0;       /* no Executive routines in this program */
#ifdef LANEC_DI_TRANSFER_C64M
    m->stop_list = LANEC_DI_C64M_STOPS;
    m->stop_count = (uint16_t)(sizeof LANEC_DI_C64M_STOPS / sizeof LANEC_DI_C64M_STOPS[0]);
#else
    if (m->mission == LANEC_CM) {
        m->stop_list = c64m_cm_stops[c64m_case->module & 0177u].list;
        m->stop_count = c64m_cm_stops[c64m_case->module & 0177u].count;
    } else {
        m->stop_list = c64m_lm_stops[c64m_case->module & 0177u].list;
        m->stop_count = c64m_lm_stops[c64m_case->module & 0177u].count;
    }
#endif
}

#if !defined(LANEC_DI_TRANSFER_C64M) || !defined(LANEC_DI_C64M_CM)
static enum exec_status run_c64m_lm(agc_native_program *program, apollo_overlay_state *s, lanec_io *io, uint16_t *steps)
{
    static p66_bias_io lm;
    enum exec_status status;
    uint16_t count;
    program->operands = lm_interbank_site_table(&count); program->operand_count = count;
    lm.superbank = io->superbank; lm.interrupt_enabled = io->interrupt_enabled;
    status = agc_transfer_run(program, s, &lm, &lm.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, steps);
    io->superbank = lm.superbank; io->interrupt_enabled = lm.interrupt_enabled;
    io->channel[011] = program->hardware->channel11;
    return status;
}
#endif
#if !defined(LANEC_DI_TRANSFER_C64M) || defined(LANEC_DI_C64M_CM)
static enum exec_status run_c64m_cm(agc_native_program *program, apollo_overlay_state *s, lanec_io *io, uint16_t *steps)
{
    static cm_native_io cm;
    enum exec_status status;
    uint16_t count;
    program->operands = cm_interbank_site_table(&count); program->operand_count = count;
    memset(&cm, 0, sizeof cm);
    cm.superbank = io->superbank; cm.interrupt_enabled = io->interrupt_enabled; cm.channel11 = io->channel[011];
    status = agc_transfer_run(program, s, &cm, &cm.superbank, (uint16_t)(AGC_TRANSFER_NONE - 1u), 64u, steps);
    io->superbank = cm.superbank; io->interrupt_enabled = cm.interrupt_enabled;
    io->channel[011] = cm.channel11;
    return status;
}
#endif

static enum lanec_status run_c64m(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    static agc_routine_table tables[2];
    static agc_native_program program;
    static agc_hardware hardware;
    enum exec_status status;
    uint16_t steps = 0;
    memset(&program, 0, sizeof program);
    memset(&hardware, 0, sizeof hardware);
    program.basic = tables; program.basic_count = 2u;
    program.interpreter_fetch = AGC_TRANSFER_NONE; program.loc = 0164u; program.bankset = 0165u;
    program.hardware = &hardware;
    hardware.channel11 = io->channel[011];
#ifdef LANEC_DI_TRANSFER_C64M          /* one module and mission: the program's own */
    (void)mission;
    tables[1] = LANEC_DI_C64M_TABLE;
#ifdef LANEC_DI_C64M_CM
    tables[0] = cm_interbank_routines;
    status = run_c64m_cm(&program, s, io, &steps);
#else
    tables[0] = lm_interbank_routines;
    status = run_c64m_lm(&program, s, io, &steps);
#endif
#else
    if (mission == LANEC_LM) {
        tables[0] = lm_interbank_routines; tables[1] = *c64m_lm_tables[c64m_case->module & 0177u];
        status = run_c64m_lm(&program, s, io, &steps);
    } else {
        tables[0] = cm_interbank_routines; tables[1] = *c64m_cm_tables[c64m_case->module & 0177u];
        status = run_c64m_cm(&program, s, io, &steps);
    }
#endif
    /* a call site's chain runs at least the INTER-BANK entry and the routine */
    if (status == EXEC_UNMAPPED && steps >= (c64_entry ? 1u : 2u) && !unregistered_c64(s, io)) return LANEC_OK;
    {   /* refused: the entry state again (a copy does not fit the C64 program) */
        static lanec_meta again;
        memset(&again, 0, sizeof again);
        make_case(c64m_case, s, io, &again);
    }
    return LANEC_OUT_OF_SCOPE;
}
const lanec_suite lanec_suite_di_transfer_c64m = { "di-transfer-c64m", LANEC_DI_TRANSFER_C64M_CASES, make_c64m, run_c64m };
#endif
