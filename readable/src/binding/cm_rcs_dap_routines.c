/* SPDX-License-Identifier: MIT */
/* The readable RCS-CSM_DIGITAL_AUTOPILOT (src/cm_dap) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_rcs_dap.c): cm_rcs_dap_basic (the basic code;
 * the file has no interpretive code, and cm_rcs_dap_interpretive no keys)
 * and cm_rcs_dap_hot_routines, the keys one pass reaches, which the flight
 * store packs together (the same routines).
 *
 * The table carries every key of the faithful one, in its order
 * (cm_rcs_dap_keys.inc, which tools/p20_p25_keys.py writes from it), and
 * each runs its readable routine. The side-by-side test
 * (tests/cm_rcs_dap_cases.c) checks them against the faithful port's.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (RESUME, NOVAC, IBNKCALL, SMALLMP, DPADD, AHFNOROT,
 * JETS, T6PROG) or reaches a key of another part of the faithful port's
 * code, where its run stops. The subroutines' TC Q goes on at the word Q
 * names: the faithful port runs on where that is a key of the same part,
 * and so does the binding; at any other word it stops.
 *
 * The device is the CM's io record (cm_native_io), which keeps channels
 * 12, 13 and 14; channel 31 is the program's hardware record's. The faithful
 * routines take that record, and without one refuse, so the binding does
 * too. The states they take: FB bank 21, EBANK E6, ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "agc_transfer.h"
#include "cm_rcs_dap.h"
#include "../cm_dap/cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The file's bank. */
#define RCS_DAP_BANK 021u

static jmp_buf back;

/* The run's io and hardware records. */
static cm_native_io *io_now;
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* The channels the file reads and writes, where the faithful port keeps
 * them: 12 (IMU control), 13 (T6's enable) and 14 (the DAC activity bits)
 * in the io record, 31 (the autopilot's switches and the RHC, read only)
 * in the hardware record. */
static agc_word *channel(unsigned number)
{
    switch (number) {
    case 012u: return &io_now->channel12;
    case 013u: return &io_now->channel13;
    case 014u: return &io_now->channel14;
    case 031u: return &hardware_now->channel31;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return (word)*channel(number);
}

/* Channel 13's bits 12-14, as the faithful port's io record keeps them,
 * set the HANDRUPT traps and are not kept. */
static void channel_out(unsigned number, word value)
{
    if (number == 031u) not_modelled(); /* the file writes no input channel */
    if (number == 013u) {
        if (value & 004000u) io_now->trap31a = 1u;
        if (value & 010000u) io_now->trap31b = 1u;
        if (value & 020000u) io_now->trap32 = 1u;
        value &= 043777u;
    }
    *channel(number) = (agc_word)value;
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
    io_now = io;
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take: the bank registers (EBANK E6),
 * the latches, and every erasable word an ordinary one. */
static bool basic_sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK != 03000u ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

/* The words a function starts at: every key of the faithful table. */
static const struct basic_entry {
    word at;
    void (*routine)(void);
} basic_entries[] = {
    {REDORCS, redorcs},
    {RCSATT, rcsatt},
    {RCSATT + 032u, rcsatt_after_zerojet},
    {DRHOLOOP, drholoop},
    {DRHOLOOP + 3u, drholoop_drho},
    {DRHOLOOP + 4u, drholoop_drho_unindexed},
    {DRHOLOOP + 010u, drholoop_das},
    {DRHOLOOP + 020u, drholoop_after_smallmp},
    {ADOTLOOP, adotloop},
    {ADOTLOOP + 6u, adotloop_drho},
    {ADOTLOOP + 013u, adotloop_merrorx},
    {KMATRIX, kmatrix},
    {KMATRIX + 010u, kmatrix_after_novac},
    {DELOOP, deloop},
    {DELOOP + 6u, deloop_delcdux},
    {DELOOP + 012u, deloop_dpadd},
    {DELOOP + 013u, deloop_after_dpadd},
    {DELOOP + 021u, deloop_next},
    {DACNDLS, dacndls},
    {KRESUME1, kresume1},
    {NEEDLER, needler},
    {NEEDLER + 5u, needler_first_pass},
    {NEEDLE11, needle11},
    {NEEDLER2, needler2},
    {NEEDLER2 + 3u, needler2_reset},
    {NEEDLER2 + 4u, needler2_mask},
    {NEEDLES3, needles3},
    {NEEDLES3 + 3u, needles3_test},
    {NEEDLES, needles},
    {DACLOOP, dacloop},
    {DACLOOP + 6u, dacloop_limit},
    {DACLOOP + 034u, dacloop_return},
    {REINIT, reinit},
    {FRESHDAP, freshdap},
    {REDAP, redap},
    {REDAP + 2u, redap_s40_14},
    {REDAP + 4u, redap_zerot5},
    {REDAP + 014u, redap_after_zerojet},
    {REDAP + 015u, redap_chantemp},
    {REDAP + 035u, redap_imu},
    {IMUAOK + 4u, imuaok_after_novac},
    {ZEROJET, zerojet},
    {ZEROJET + 026u, zerojet_return},
    {T5PHASE2, t5phase2},
    {NOHIAUTO - 1u, t5phase2_hiauto},
    {NOHIAUTO, nohiauto},
    {NOHIAUTO + 025u, nohiauto_damping},
    {NOCHANGE, nochange},
    {NOCHANGE + 6u, nochange_after_stickchk},
    {SETWBODY, setwbody},
    {SETWBODY + 2u, setwbody_dpndx},
    {SETWBODY + 3u, setwbody_rmanndx},
    {SETWBODY + 4u, setwbody_rmanndx_unindexed},
    {SETWBODY + 015u, setwbody_wbody},
    {ZEROER, zeroer},
    {NORATE, norate},
    {MERUPDAT, merupdat},
    {SPNDXCHK, spndxchk},
    {SPNDXCHK + 2u, spndxchk_errorx},
    {FREEFUNC, freefunc},
    {T6PROGM, t6progm},
    {STICKCHK, stickchk},
    {KALUPDT, kalupdt},
    {KRESUME2 + 1u, kresume2_t5phase},
    {FDAIDSP2, fdaidsp2},
    {FDAIDSP2 + 011u, fdaidsp2_ak},
    {FDAITOTL, fdaitotl},
    {WRTN22 + 2u, wrtn22_wtemp},
    {GETAKS, getaks},
    {GETAKS + 026u, getaks_amgb5},
    {WRTN17, wrtn17},
};

static const struct basic_entry *basic_entry(word at)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == at) return &basic_entries[i];
    return NULL;
}

/* The first word of each part of the faithful port's code: a run that
 * reaches a key of another part stops there. */
static const word part_starts[] = {02105u, 02221u, 02317u, 02374u, 02465u,
                                   02546u, 02660u, 02755u, 03023u, 03156u};

static unsigned part_of(word at)
{
    unsigned part = 0;

    while (part + 1u < COUNT(part_starts) && part_starts[part + 1u] <= at) ++part;
    return part;
}

/* After a TC Q, at the word Q named: on at a key of the same part, as the
 * faithful port does; at any other word (in fixed-fixed or erasable
 * memory, or another part) the run stops there. */
static void run_on(word key)
{
    const struct basic_entry *entry;

    while (Z >= 02000u && Z <= 03777u && part_of(Z) == part_of(key) &&
           (entry = basic_entry(Z)) != NULL)
        entry->routine();
}

/* ---- The keys, in the faithful table's order ---- */

/* Each key's routine, from a state the faithful one takes. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    word at = (word)(02000u + key % 1024u);
    const struct basic_entry *entry = basic_entry(at);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry || !program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)(RCS_DAP_BANK << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    run_on(at);
    return EXEC_OK;
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_rcs_dap_readable(uint16_t key)
{
    return key / 1024u == RCS_DAP_BANK && basic_entry((word)(02000u + key % 1024u)) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)
#include "cm_rcs_dap_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_rcs_dap_keys.inc"
};
#undef PAIR
#undef BASIC

/* The keys one pass reaches, as the faithful hot table lists them. */
static const agc_routine_entry hot_keys[] = {
    {KEY(RCS_DAP_BANK, 02122u), basic_1},  {KEY(RCS_DAP_BANK, 02216u), basic_3},
    {KEY(RCS_DAP_BANK, 02221u), basic_4},  {KEY(RCS_DAP_BANK, 02236u), basic_7},
    {KEY(RCS_DAP_BANK, 02311u), basic_8},  {KEY(RCS_DAP_BANK, 02317u), basic_9},
    {KEY(RCS_DAP_BANK, 02360u), basic_12}, {KEY(RCS_DAP_BANK, 02411u), basic_18},
    {KEY(RCS_DAP_BANK, 02417u), basic_19}, {KEY(RCS_DAP_BANK, 02471u), basic_26},
    {KEY(RCS_DAP_BANK, 02661u), basic_43}, {KEY(RCS_DAP_BANK, 03160u), basic_64},
};

const agc_routine_table cm_rcs_dap_interpretive = {NULL, 0};
const agc_routine_table cm_rcs_dap_basic = {basic_keys, COUNT(basic_keys)};
const agc_routine_table cm_rcs_dap_hot_routines = {hot_keys, COUNT(hot_keys)};

#endif
