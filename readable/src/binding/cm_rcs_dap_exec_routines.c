/* SPDX-License-Identifier: MIT */
/* The readable RCS-CSM_DAP_EXECUTIVE_PROGRAMS (src/cm_dap/dap_executive.c)
 * as routines of the CM's flight program, under the table names and keys
 * of the faithful port's (c64-port/src/cm_rcs_dap_exec.c):
 * cm_rcs_dap_exec_basic (the basic code; the file has no interpretive
 * code, and cm_rcs_dap_exec_interpretive no keys).
 *
 * The table carries every key of the faithful one, in its order
 * (cm_rcs_dap_exec_keys.inc, which tools/p20_p25_keys.py writes from it),
 * and every key runs a readable routine. The side-by-side test
 * (tests/cm_rcs_dap_exec_cases.c) checks that no key is left without one.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (SPSIN, SPCOS, ENDOFJOB); each call out returns to a key
 * of its own, so a run never goes on at one.
 *
 * The file reads and writes no channel. The faithful routines take the
 * program's hardware record, and without one refuse, so the binding does
 * too. The states they take: FB bank 22, any EBANK, ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "agc_transfer.h"
#include "cm_rcs_dap_exec.h"
#include "../cm_dap/cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The file's bank. */
#define DAP_EXEC_BANK 022u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The file reads and writes no channel. */
static word channel_in(unsigned number)
{
    (void)number;
    not_modelled();
    return 0u;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled();
}

/* The file's constants are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(1u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, cm_native_io *io)
{
    bind_machine(program, state, io);
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take: the bank registers, the latches,
 * and every erasable word an ordinary one. */
static bool basic_sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

/* The words a function starts at: the keys. */
static const struct basic_entry {
    word at;
    void (*routine)(void);
} basic_entries[] = {
    {AMBGUPDT, ambgupdt},
    {AMBGUPDT + 010u, ambgupdt_sin_psi},
    {AMBGUPDT + 013u, ambgupdt_cos_psi},
    {AMBGUPDT + 021u, ambgupdt_cos_phi},
    {AMBGUPDT + 032u, ambgupdt_sin_phi},
};

static const struct basic_entry *basic_entry(word at)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == at) return &basic_entries[i];
    return NULL;
}

/* ---- The keys, in the faithful table's order ---- */

/* A readable routine, from a state the faithful one takes. */
static enum exec_status run_basic(void (*routine)(void), apollo_overlay_state *state)
{
    apollo_overlay_state before = *state;

    if (setjmp(back)) {
        *state = before;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

/* Each key's routine. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry((word)(02000u + key % 1024u));

    if (!entry || !program || !program->hardware || !state || !device) return EXEC_OUT_OF_SCOPE;
    bind(program, state, device);
    if (FBANK != (word)(DAP_EXEC_BANK << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state);
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_rcs_dap_exec_readable(uint16_t key)
{
    return key / 1024u == DAP_EXEC_BANK && basic_entry((word)(02000u + key % 1024u)) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)
#include "cm_rcs_dap_exec_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_rcs_dap_exec_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_rcs_dap_exec_interpretive = {NULL, 0};
const agc_routine_table cm_rcs_dap_exec_basic = {basic_keys, COUNT(basic_keys)};

#endif
