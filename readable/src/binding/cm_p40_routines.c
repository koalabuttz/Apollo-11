/* SPDX-License-Identifier: MIT */
/* The readable P40-P47 of the CM (src/p40_p47_cm/) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_p40_p47.c): cm_p40_p47_basic (the basic code)
 * and cm_p40_p47_interpretive (the order pairs).
 *
 * The file is written part by part, so the tables carry every key of the
 * faithful ones, in their order (cm_p40_keys.inc, which
 * tools/p20_p25_keys.py writes from them): a key the parts written so far
 * cover runs a readable routine; any other runs the faithful port's,
 * built into the program as faithful_cm_p40_p47_basic and
 * faithful_cm_p40_p47_interpretive (tests/x86_readable.py renames them).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (BANKCALL, DOWNFLAG, WAITLIST, PHASCHNG, ...) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()),
 * which ends at the pair's DANZIG, at an EXIT, or where a CALL or GOTO
 * leaves it.
 *
 * SETMINDB, SETMAXDB, SPSOFF and TVCZAP end in TC Q. Entered at their
 * keys, Q can hold any return; the faithful port runs on where it is a
 * word of bank 24 it has code for, and so does the binding, up to the last
 * word the parts written so far reach. Past it, up to the file's last word
 * in the bank, the binding refuses (the side-by-side test counts these);
 * beyond that both stop.
 *
 * The device is the CM's io record (cm_native_io); the faithful basic
 * routines take the program's hardware record for their channel orders,
 * and without one refuse, so the binding does too: channel 5 is the
 * hardware record's, channels 11 (DSALMOUT) and 12 the io record's. The
 * states the faithful routines take, at every key: FB the
 * key's bank, ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range, and every erasable word an ordinary one; at
 * an order pair Z at NEWOPS, BANKSET the BBANK, INTBIT15 the bank's bit
 * 15, NEWJOB +0 or a core set and FIXLOC one of the VAC areas or 0110. A
 * run the readable code refuses part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p40_p47.h"
#include "../p40_p47_cm/p40_p47_cm.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_p40_p47_basic, faithful_cm_p40_p47_interpretive;

static jmp_buf back;

/* The run's io record and hardware record. */
static cm_native_io *io_now;
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* SPSOFF's and TVCZAP's channel 12 (WOR, WAND), and ULAGEOFF's channel 5
 * (WRITE), as the faithful port keeps them. */
static word channel_in(unsigned number)
{
    if (number == 012u) return (word)io_now->channel12;
    not_modelled(); /* the readable parts read no other channel */
    return 0;
}

static void channel_out(unsigned number, word value)
{
    if (number == 012u)
        io_now->channel12 = (agc_word)value;
    else if (number == 05u && hardware_now)
        hardware_now->channel5 = (agc_word)value;
    else
        not_modelled(); /* the readable parts write no other channel */
}

/* The interpreter's fixed operands are the rope's. */
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

/* The states the faithful routines take (its basic_sane(); at an order
 * pair, its sane()). */
static bool sane(bool order_pair)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (order_pair &&
        (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u ||
         (FIXLOC != 0110u && (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u))))
        return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* Part 1: P40CSM to P40TSK */
    {KEY(024u, P40CSM), p40csm},
    {KEY(024u, P40CSM + 2u), p40csm_after_downflag},
    {KEY(024u, P40CSM + 017u), p40csm_after_exit},
    {KEY(024u, P40PVA), p40pva},
    {KEY(024u, P40SXTY), p40sxty},
    {KEY(024u, P40SXTY + 4u), p40sxty_after_r60csm},
    {KEY(024u, P40SXTY + 010u), p40sxty_after_upflag},
    {KEY(024u, P40TTOG + 011u), p40ttog_after_exit},
    {KEY(024u, P40GMB + 3u), p40gmb_terminate},
    {KEY(024u, P40GMB + 4u), p40gmb_proceed},
    {KEY(024u, TRIMONLY), trimonly},
    {KEY(024u, TRIMONLY + 010u), trimonly_after_waitlist},
    {KEY(024u, TRIMONLY + 016u), trimonly_after_delayjob},
    {KEY(024u, P40S_RS), p40s_rs},
    {KEY(024u, P40S_RS + 4u), p40s_rs_after_waitlist},
    {KEY(024u, P40S_SV), p40s_sv},
    {KEY(024u, P40S_SV + 010u), p40s_sv_after_midtoav1},
    {KEY(024u, P40S_SV + 011u), p40s_sv_after_midtoav1_fail},
    {KEY(024u, P40SET + 013u), p40set_after_longcall},
    {KEY(024u, P40SET + 015u), p40set_after_phaschng},
    {KEY(024u, P40BLNKR), p40blnkr},
    {KEY(024u, P40BLNKR + 2u), p40blnkr_after_cleandsp},
    {KEY(024u, POSTBURN), postburn},
    {KEY(024u, POSTBURN + 3u), postburn_terminate},
    {KEY(024u, POSTBURN + 4u), postburn_proceed},
    {KEY(024u, POSTBURN + 5u), postburn_recycle},
    {KEY(024u, P40MINDB), p40mindb},
    {KEY(024u, TIGNOW), tignow},
    {KEY(024u, TIGNOW + 3u), tignow_after_phaschng},
    {KEY(024u, TIGNOW + 6u), tignow_terminate},
    {KEY(024u, TIGNOW + 7u), tignow_proceed},
    {KEY(024u, TIGNOW + 010u), tignow_recycle},
    {KEY(024u, SETMINDB), setmindb},
    {KEY(024u, SETMAXDB - 1u), setmaxdb_inhint},
    {KEY(024u, SETMAXDB), setmaxdb},
    {KEY(024u, TTG_0), ttg_0},
    {KEY(024u, P40CLK), p40clk},
    {KEY(024u, P40CLK + 2u), p40clk_after_downflag},
    {KEY(024u, P41_DSP + 6u), p41_dsp_after_exit},
    {KEY(024u, P41_DSP + 014u), p41_dsp_after_2phschng},
    {KEY(024u, P41REDSP + 6u), p41redsp_after_waitlist},
    {KEY(024u, CALCN85), calcn85},
    {KEY(024u, CALCN85 + 6u), calcn85_after_exit},
    {KEY(024u, TIGBLNK), tigblnk},
    {KEY(024u, TIGBLNK + 4u), tigblnk_after_waitlist},
    {KEY(024u, TIGBLNK + 012u), tigblnk_after_novac},
    {KEY(024u, P40TSK + 2u), p40tsk_after_newphase},
    /* Part 2: P47CSM to TVCZAP */
    {KEY(024u, P47CSM), p47csm},
    {KEY(024u, P47CSM + 2u), p47csm_after_r02both},
    {KEY(024u, P47CSM + 5u), p47csm_after_midtoav2},
    {KEY(024u, P47CSM + 012u), p47csm_after_waitlist},
    {KEY(024u, P47CSM + 014u), p47csm_after_phaschng},
    {KEY(024u, TIGON), tigon},
    {KEY(024u, TIGON + 7u), tigon_after_novac},
    {KEY(024u, CALCN83), calcn83},
    {KEY(024u, CALCN83 + 014u), calcn83_after_exit},
    {KEY(024u, CALCN83 + 016u), calcn83_after_phaschng},
    {KEY(024u, CALCN83 + 022u), calcn83_after_gentran},
    {KEY(024u, P47BODY), p47body},
    {KEY(024u, P47BOD), p47bod},
    {KEY(024u, P47BOD + 2u), p47bod_after_priochng},
    {KEY(024u, P47_DSP), p47_dsp},
    {KEY(024u, P47_DSP + 3u), p47_dsp_terminate},
    {KEY(024u, P47_DSP + 4u), p47_dsp_proceed},
    {KEY(024u, P47_DSP + 5u), p47_dsp_recycle},
    {KEY(024u, TIGAVEG), tigaveg},
    {KEY(024u, TIGAVEG + 010u), tigaveg_after_waitlist},
    {KEY(024u, PRECHECK), precheck},
    {KEY(024u, TIG_5), tig_5},
    {KEY(024u, TIG_5 + 4u), tig_5_after_waitlist},
    {KEY(024u, TIG_5 + 011u), tig_5_after_2phschng},
    {KEY(024u, TIG_5 + 015u), tig_5_after_findvac},
    {KEY(024u, TIG_0), tig_0},
    {KEY(024u, IGNITION), ignition},
    {KEY(024u, PREPTVC + 011u), preptvc_after_newphase},
    {KEY(024u, DOTVCON), dotvcon},
    {KEY(024u, DOTVCON + 022u), dotvcon_after_newphase},
    {KEY(024u, DOSTRULL), dostrull},
    {KEY(024u, IMPLBURN + 020u), implburn_after_2phschng},
    {KEY(024u, IMPLCONT), implcont},
    {KEY(024u, ENGINOFF), enginoff},
    {KEY(024u, DOSPSOFF), dospsoff},
    {KEY(024u, DOSPSOFF + 4u), dospsoff_after_newphase},
    {KEY(024u, DOTVCRCS), dotvcrcs},
    {KEY(024u, DOTVCRCS + 3u), dotvcrcs_after_rcsdapon},
    {KEY(024u, DOTVCRCS + 5u), dotvcrcs_after_massprop},
    {KEY(024u, DOTVCRCS + 010u), dotvcrcs_after_phaschng},
    {KEY(024u, DOTVCRCS + 014u), dotvcrcs_after_novac},
    {KEY(024u, SPSOFF), spsoff},
    {KEY(024u, TVCZAP), tvczap},
    /* Part 1: P41CSM */
    {KEY(027u, P41CSM), p41csm},
    {KEY(027u, P41CSM + 2u), p41csm_after_upflag},
};

/* The other words of bank 24 the faithful port runs on at after a TC Q:
 * the returns of the file's own subroutines. */
static const struct basic_entry returns[] = {
    {KEY(024u, P40SXTY + 1u), p40sxty_after_setmindb},
    {KEY(024u, P40SXTY + 011u), p40sxty_p41},
    {KEY(024u, P40TTOG), p40ttog},
    {KEY(024u, P40S_SV + 1u), p40s_sv_after_e7setter},
    {KEY(024u, P40SET), p40set},
    {KEY(024u, P40MINDB + 1u), p40mindb_after_setmindb},
    {KEY(024u, TIGAVEG + 1u), tigaveg_p41},
    {KEY(024u, TIGAVEG + 2u), tigaveg_p40},
    {KEY(024u, DOSTRULL + 4u), dostrull_after_steerull},
    {KEY(024u, DOSTRULL + 5u), dostrull_after_ulageoff},
    {KEY(024u, IMPLBURN + 4u), implburn_after_e7setter},
    {KEY(024u, IMPLCONT + 4u), implcont_after_e6setter},
    {KEY(024u, ENGINOFF + 1u), enginoff_after_e6setter},
    {KEY(024u, DOSPSOFF + 1u), dospsoff_after_spsoff},
    {KEY(024u, DOTVCRCS + 1u), dotvcrcs_after_setmaxdb},
    {KEY(024u, DOTVCRCS + 6u), dotvcrcs_after_tvczap},
};

static const struct basic_entry *entry_in(const struct basic_entry *entries, unsigned count,
                                          uint16_t key)
{
    unsigned i;

    for (i = 0; i < count; ++i)
        if (entries[i].key == key) return &entries[i];
    return NULL;
}

static const struct basic_entry *basic_entry(uint16_t key)
{
    return entry_in(basic_entries, COUNT(basic_entries), key);
}

/* The last word of bank 24 the parts written so far reach. */
#define LAST_WRITTEN (TVCZAP + 012u)

/* Whether the binding follows a TC Q to a word of bank 24: one in the
 * parts written so far (or below them, another file's), but not FENG,
 * the constant after CALCN85's TC SERVXT, where the faithful port fails;
 * or one past ENDV97E, the file's last word in the bank, where it stops. */
static bool follows(word z)
{
    if (z > ENDV97E) return true;
    return z <= LAST_WRITTEN && z != (word)(CALCN85 + 7u);
}

/* After the TC Q of SETMINDB, SETMAXDB, SPSOFF or TVCZAP, to a word of
 * bank 24: on at a key
 * or one of the returns above, as the faithful port does; at any other
 * word it stops there too. Refused where the binding does not follow (the
 * side-by-side test counts these). Erasable and fixed-fixed returns end
 * the run. */
static void run_on(void)
{
    const struct basic_entry *entry;

    if (Z < 02000u || Z > 03777u) return;
    if (!follows(Z)) not_modelled();
    entry = basic_entry(KEY(024u, Z));
    if (!entry) entry = entry_in(returns, COUNT(returns), KEY(024u, Z));
    if (entry) entry->routine();
}

/* For the side-by-side test: whether the binding follows a TC Q to a word
 * of bank 24. */
bool cm_p40_p47_runs_on(uint16_t z)
{
    return follows(z);
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
    &P40CSM_CODE,  &P40PVA_CODE, &P40TTOG_CODE, &P40S_SV_CODE, &P41_DSP_CODE, &P40CNV85_CODE,
    &CALCN85_CODE, &P41CSM_CODE, &P47CSM_CODE,  &CALCN83_CODE, &P47BODY_CODE,
};

/* The table an address is in, in a bank, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each key's routine: the readable one, or the faithful port's. A basic
 * routine runs from its key, whatever Z holds (the trampoline calls it
 * there). */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry) return faithful_cm_p40_p47_basic.entries[index].call(program, state, device);
    if (!program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)(key / 1024u << 10) || !sane(false)) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    if (entry->routine == setmindb || entry->routine == setmaxdb_inhint ||
        entry->routine == setmaxdb || entry->routine == spsoff || entry->routine == tvczap)
        run_on();
    return EXEC_OK;
}

static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    cm_native_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!code_at(bank, (word)(02000u + key % 1024u)))
        return faithful_cm_p40_p47_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane(true))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_p40_p47_readable(uint16_t key, bool order_pair)
{
    return order_pair ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL
                      : basic_entry(key) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(i, KEY(bank, address), program, state, device);                               \
    }
#define PAIR(i, bank, address)                                                                     \
    static enum exec_status pair_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return pair(i, KEY(bank, address), program, state, device);                                \
    }
#include "cm_p40_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_p40_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_p40_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_p40_p47_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p40_p47_basic = {basic_keys, COUNT(basic_keys)};

#endif
