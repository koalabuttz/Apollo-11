/* SPDX-License-Identifier: MIT */
/* The readable SYSTEM_TEST_STANDARD_LEAD_INS (src/standard_lead/) as
 * routines of the flight program, under the table names and keys of the
 * faithful port's (c64-port/src/standard_lead_routines.c):
 *
 *   <LM> standard_lead_routines, <CM> cm_standard_lead_routines
 *       the basic entries: E/BKCALL, its three SWRETURN landing points,
 *       E/CALL, E/CALL's word after the EXIT, E/JOBWAK, and the CM's
 *       COMPVER, GTSCPSS1 and REDO;
 *   lm_interpreter_routines, cm_interpreter_routines
 *       the INTERPRETER entries these reach: INTPRET (from E/CALL, on
 *       through its CALL), INTPRET +2 and INTRSM;
 *   standard_lead_interpretive_routines, cm_standard_lead_interpretive_routines
 *       E/CALL's CALL and EXIT words, at the interpreter's fetch point.
 *
 * The readable build links these in their place. (The faithful file also
 * offers <m>_interpreter_rvq, an RVQ for its native interpretive callees'
 * tests; the flight program has none, and nothing here needs it.)
 *
 * Plumbing only: point the machine at the program's state, give it the
 * CADR the caller placed after its call, take the states the faithful
 * routines take, and run the readable routine. */
#include "binding.h"
#include "../standard_lead/standard_lead.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099
#define BASIC        standard_lead_routines
#define INTERPRETER  lm_interpreter_routines
#define INTERPRETIVE standard_lead_interpretive_routines
#define LEAD_BANK    04u /* E/BKCALL and E/CALL */
#define LAST_NEWJOB  84u /* core set 7 */
#else
#define BASIC        cm_standard_lead_routines
#define INTERPRETER  cm_interpreter_routines
#define INTERPRETIVE cm_standard_lead_interpretive_routines
#define LEAD_BANK    013u
#define LAST_NEWJOB  72u /* core set 6 */
#endif
#define JOBWAK_BANK 04u
#define TEST_BANK   033u /* COMPVER, GTSCPSS1, REDO */

#define KEY(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))
#define LOCCTR_NONE  077776u /* -1: JOBWAKE found no such job */

static const agc_native_program *program_now;

/* ---- Fixed memory: the CADR after the call ---- */

/* The one word of fixed memory a lead-in reads (the CADR its caller placed
 * after the call, when the caller is in fixed memory), found before the
 * routine runs: the caller's compiled operand. */
static struct {
    unsigned physical;
    word value;
} cadr_word;

static word fixed_memory(unsigned physical)
{
    return physical == cadr_word.physical ? cadr_word.value : 0u;
}

/* The CADR at address q can be read: an erasable word above the central
 * registers, or a compiled operand of the program's (a fixed caller's).
 * (An erasable caller's q is at least 100, but through EBANK 0's window
 * it can name a central register: the faithful port reads those as
 * plain words, where the AGC's CA rewrites an editing register. Such a
 * call is left to the faithful port.) */
static bool cadr_readable(word q)
{
    agc_word value;

    if (q < 02000u) return erasable_index(q) >= 0100u;
    cadr_word.physical = fixed_physical(q);
    if (!agc_site_lookup(program_now->operands, program_now->operand_count,
                         (uint16_t)cadr_word.physical, &value) ||
        value > MINUS_ZERO)
        return false;
    cadr_word.value = value;
    return true;
}

/* ---- The states the faithful routines take ---- */

/* ZERO is +0 and the bank registers agree. */
static bool banks_agree(void)
{
    return agc.memory[ZERO_ADDRESS] == 0u && EBANK <= 03400u && (EBANK & 0377u) == 0u &&
           (FBANK & ~076000u) == 0u && BBANK == (word)(FBANK | (EBANK >> 8));
}

/* The superbank is one of the eight, and interrupts are on or off. */
static bool latches_in_range(void)
{
    return (*agc.superbank & ~0160u) == 0u && *agc.interrupts <= 1u;
}

/* FIXLOC is one of the VAC areas. */
static bool fixloc_a_vac_area(void)
{
    unsigned area;

    for (area = 0401u; area <= 0661u; area += 054u)
        if (FIXLOC == area) return true;
    return false;
}

/* E/JOBWAK's JOBWAKE as the faithful port's takes it: called inhibited,
 * returning through RUPTREG3, A an ordinary word (the job's address), every
 * core set's priority nonzero, NEWJOB -0 or a core set, and the sleeping job
 * found (if any) with a plain BANKSET (EBANK and superbank bits). */
static bool jobwak_takes(const mission_io *io)
{
    word wake = overflow_corrected(A);
    unsigned set, found = CORE_SETS;

    if (*agc.interrupts != 0u || FBANK != JOBWAK_BANK << 10 || RUPTREG3 < 0100u ||
        RUPTREG3 > 07777u || A != sign_extend(wake) || !banks_agree() ||
        (*agc.superbank & ~0160u) != 0u)
        return false;
#if COMANCHE055
    if (io->executive_alarm != 0u) return false;
#else
    (void)io;
#endif
    if (NEWJOB != MINUS_ZERO && (NEWJOB > LAST_NEWJOB || NEWJOB % 12u != 0u)) return false;
    for (set = 0; set < CORE_SETS; ++set) {
        word priority = IN_CORE_SET(PRIORITY, 12u * set), loc = IN_CORE_SET(LOC, 12u * set);
        if (priority == 0u) return false;
        if (found == CORE_SETS && priority >= 040000u && priority < MINUS_ZERO &&
            (loc == wake ||
             ((loc == 0u || loc == MINUS_ZERO) && (wake == 0u || wake == MINUS_ZERO))))
            found = set;
    }
    return found == CORE_SETS ||
           ((IN_CORE_SET(BANKSET, 12u * found) & ~0167u) == 0u && !(found == 0u && NEWJOB == 0u));
}

/* Whether E/BKCALL's and E/CALL's entries take the state: in their bank,
 * with the CADR or return they read in range. */
static bool lead_takes(word entry)
{
    if (!banks_agree() || FBANK != LEAD_BANK << 10) return false;
    switch (entry) {
    case E_BKCALL: return BUF2(0) >= 0100u && BUF2(0) <= 07775u && cadr_readable(BUF2(0));
    case E_BKCALL + 011u:
    case E_BKCALL + 012u:
    case E_BKCALL + 013u: return EBUF2(0) >= 0100u && EBUF2(0) <= 07775u;
    case E_CALL: return LOC >= 077u && LOC <= 07774u && cadr_readable((word)(LOC + 1u));
    default: return true;
    }
}

/* Whether the basic entry takes the state. */
static bool basic_takes(word entry, const mission_io *io)
{
    if (Z != entry) return false;
    if (entry == E_JOBWAK) return jobwak_takes(io);
#if COMANCHE055
    if (entry >= COMPVER && entry <= REDO) return agc.memory[ZERO_ADDRESS] == 0u;
#endif
    return lead_takes(entry);
}

/* ---- Running a routine ---- */

static enum exec_status run_basic(const agc_native_program *program, apollo_overlay_state *state,
                                  void *device, word entry)
{
    if (!bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    program_now = program;
    agc.fixed_memory = fixed_memory;
    cadr_word.physical = ~0u;
    if (!basic_takes(entry, device)) return EXEC_OUT_OF_SCOPE;
    switch (entry) {
    case E_BKCALL: e_bkcall(); break;
    case E_BKCALL + 011u: e_bkcall_bad_return(); break;
    case E_BKCALL + 012u: e_bkcall_proceed_return(); break;
    case E_BKCALL + 013u: e_bkcall_enter_return(); break;
    case E_CALL: e_call(); break;
    case E_CALL + 012u: e_call_after_exit(); break;
    case E_JOBWAK: e_jobwak(); return LOCCTR == LOCCTR_NONE ? EXEC_NOT_FOUND : EXEC_OK;
#if COMANCHE055
    case COMPVER: compver(); break;
    case GTSCPSS1: gtscpss1(); break;
    case REDO: redo(); break;
#endif
    default: return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

#define BASIC_ENTRY(name, entry)                                                                   \
    static enum exec_status name(const agc_native_program *program, apollo_overlay_state *state,   \
                                 void *device)                                                     \
    {                                                                                              \
        return run_basic(program, state, device, (entry));                                         \
    }
BASIC_ENTRY(run_e_bkcall, E_BKCALL)
BASIC_ENTRY(run_bad_return, E_BKCALL + 011u)
BASIC_ENTRY(run_proceed_return, E_BKCALL + 012u)
BASIC_ENTRY(run_enter_return, E_BKCALL + 013u)
BASIC_ENTRY(run_e_call, E_CALL)
BASIC_ENTRY(run_after_exit, E_CALL + 012u)
BASIC_ENTRY(run_e_jobwak, E_JOBWAK)
#if COMANCHE055
BASIC_ENTRY(run_compver, COMPVER)
BASIC_ENTRY(run_gtscpss1, GTSCPSS1)
BASIC_ENTRY(run_redo, REDO)
#endif

/* The faithful interpretive routines and INTERPRETER entries read no
 * program tables: callers may pass none. */
static const agc_native_program no_program;

/* Whether E/CALL's CALL (LOC E/CALL +7) or EXIT (+11) takes the state: Z
 * at the fetch point, the bank registers as the interpreter sets them for
 * the job (BANKSET in this file's bank), FIXLOC a VAC area, and for the
 * CALL a fixed CADR in EBUF2. */
static bool standard_lead_code_takes(word loc)
{
    return banks_agree() && latches_in_range() && fixloc_a_vac_area() && Z == NEWOPS &&
           LOC == loc && (BANKSET & 076000u) == LEAD_BANK << 10 &&
           BBANK == (word)(BANKSET & 076007u) && (loc != E_CALL + 7u || EBUF2(0) >= 04000u);
}

/* One pass of the readable interpreter over E/CALL's words, from a state
 * standard_lead_code_takes(); the state as it was before (saved by the
 * caller) put back if the pass refuses. */
static enum exec_status run_standard_lead_code(const agc_native_program *program,
                                               apollo_overlay_state *state, void *device,
                                               const apollo_overlay_state *before,
                                               const mission_io *io_before)
{
    enum exec_status status =
        interpret_listing_any_newjob(program, state, device, &STANDARD_LEAD_CODE);
    if (status != EXEC_OK) {
        *state = *before;
        *(mission_io *)device = *io_before;
    }
    return status;
}

static apollo_overlay_state state_before;

static enum exec_status run_interpretive(const agc_native_program *program,
                                         apollo_overlay_state *state, void *device, word loc)
{
    mission_io io_before;

    if (!program) program = &no_program;
    if (!bind_machine(program, state, device) || !standard_lead_code_takes(loc))
        return EXEC_OUT_OF_SCOPE;
    state_before = *state;
    io_before = *(mission_io *)device;
    return run_standard_lead_code(program, state, device, &state_before, &io_before);
}

static enum exec_status run_call(const agc_native_program *program, apollo_overlay_state *state,
                                 void *device)
{
    return run_interpretive(program, state, device, E_CALL + 7u);
}

static enum exec_status run_exit(const agc_native_program *program, apollo_overlay_state *state,
                                 void *device)
{
    return run_interpretive(program, state, device, E_CALL + 011u);
}

/* INTPRET, INTPRET +2 and INTRSM, to NEWOPS. INTPRET from E/CALL (Q at
 * its CALL, FBANK its bank) goes on through the CALL, as the faithful port
 * does, or is refused. */
static enum exec_status run_interpreter(const agc_native_program *program,
                                        apollo_overlay_state *state, void *device, word entry)
{
    mission_io io_before;

    if (!program) program = &no_program;
    if (!bind_machine(program, state, device) || Z != entry || !banks_agree() ||
        !latches_in_range())
        return EXEC_OUT_OF_SCOPE;
    switch (entry) {
    case INTPRET:
        if (Q == E_CALL + 7u && FBANK == LEAD_BANK << 10) {
            if (!fixloc_a_vac_area() || EBUF2(0) < 04000u) return EXEC_OUT_OF_SCOPE;
            state_before = *state;
            io_before = *(mission_io *)device;
            intpret();
            return run_standard_lead_code(program, state, device, &state_before, &io_before);
        }
        intpret();
        break;
    case INTPRET + 2u:
        /* "INTERPRETIVE BRANCHES FINISH HERE": the new LOC is in the
         * interpretive program's bank, or erasable. */
        if (overflow_corrected(L) > 03777u) return EXEC_OUT_OF_SCOPE;
        intpret_plus2();
        break;
    default: intrsm(); break;
    }
    return EXEC_OK;
}

static enum exec_status run_intpret(const agc_native_program *program, apollo_overlay_state *state,
                                    void *device)
{
    return run_interpreter(program, state, device, INTPRET);
}

static enum exec_status run_intpret_plus2(const agc_native_program *program,
                                          apollo_overlay_state *state, void *device)
{
    return run_interpreter(program, state, device, INTPRET + 2u);
}

static enum exec_status run_intrsm(const agc_native_program *program, apollo_overlay_state *state,
                                   void *device)
{
    return run_interpreter(program, state, device, K_INTRSM);
}

/* ---- The routine tables ---- */

#if LUMINARY099
static const agc_routine_entry basic[] = {{KEY(LEAD_BANK, E_BKCALL), run_e_bkcall},
                                          {KEY(LEAD_BANK, E_BKCALL + 011u), run_bad_return},
                                          {KEY(LEAD_BANK, E_BKCALL + 012u), run_proceed_return},
                                          {KEY(LEAD_BANK, E_BKCALL + 013u), run_enter_return},
                                          {KEY(LEAD_BANK, E_CALL), run_e_call},
                                          {KEY(LEAD_BANK, E_CALL + 012u), run_after_exit},
                                          {KEY(JOBWAK_BANK, E_JOBWAK), run_e_jobwak}};
#else
static const agc_routine_entry basic[] = {{KEY(JOBWAK_BANK, E_JOBWAK), run_e_jobwak},
                                          {KEY(LEAD_BANK, E_BKCALL), run_e_bkcall},
                                          {KEY(LEAD_BANK, E_BKCALL + 011u), run_bad_return},
                                          {KEY(LEAD_BANK, E_BKCALL + 012u), run_proceed_return},
                                          {KEY(LEAD_BANK, E_BKCALL + 013u), run_enter_return},
                                          {KEY(LEAD_BANK, E_CALL), run_e_call},
                                          {KEY(LEAD_BANK, E_CALL + 012u), run_after_exit},
                                          {KEY(TEST_BANK, COMPVER), run_compver},
                                          {KEY(TEST_BANK, GTSCPSS1), run_gtscpss1},
                                          {KEY(TEST_BANK, REDO), run_redo}};
#endif
const agc_routine_table BASIC = {basic, COUNT(basic)};

static const agc_routine_entry interpreter[] = {
    {FF(INTPRET), run_intpret}, {FF(INTPRET + 2u), run_intpret_plus2}, {FF(K_INTRSM), run_intrsm}};
const agc_routine_table INTERPRETER = {interpreter, COUNT(interpreter)};

static const agc_routine_entry interpretive[] = {{KEY(LEAD_BANK, E_CALL + 7u), run_call},
                                                 {KEY(LEAD_BANK, E_CALL + 011u), run_exit}};
const agc_routine_table INTERPRETIVE = {interpretive, COUNT(interpretive)};
