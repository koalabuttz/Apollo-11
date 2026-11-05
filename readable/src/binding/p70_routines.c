/* SPDX-License-Identifier: MIT */
/* The readable P70-P71 (src/p70_p71/) as routines of the LM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_p70_p71_file.c): lm_p70_p71_file_basic (the basic code)
 * and lm_p70_p71_file_interpretive (the order pairs). The tables carry
 * every key of the faithful ones, in their order (p70_keys.inc, which
 * tools/p20_p25_keys.py writes from them). The faithful port also has an
 * older module of the same routines (c64-port/src/lm_p70_p71.c), whose
 * tables the flight program tries only after the file's: here they are
 * empty.
 *
 * Plumbing only. A basic routine runs from its key until its code leaves
 * the file (TWIDDLE, FLIP, LANDISP, CLRADMOD, PHASCHNG, ...) or reaches
 * NEWOPS through a TC INTPRET. Where it comes back into the file without
 * a TC (LEGAL?'s and THROTUP's TC Q, P70A's DTCB to CONTABRT, CONTABRT's
 * RESUME), Z holds the word it comes back to, and the run goes on there
 * if the faithful port has a routine at that word: one of the keys, or
 * the words after THROTUP's two returns into UPTHROT (32,3674 and
 * UPTHROT1), which it reaches only so. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()),
 * which ends at the pair's DANZIG, at an EXIT, or where a CALL or branch
 * leaves its table; a pass that ends at TGOCOMP's RTB LOADTIME runs
 * LOADTIME and goes on, as the faithful pair does.
 *
 * The device is the LM's io record with the input channels after it
 * (lm_channel_io): R10,R11 reads channel 30 (the abort buttons). Channel
 * 11 (ABRTJASK's engine on) and channel 14 (THROTUP's) are the program's
 * hardware record's. The states the faithful routines take: FB the key's
 * bank, with the superbank bit clear in bank 32; at an order pair Z at
 * NEWOPS, BANKSET the BBANK, INTBIT15 the bank's bit 15, NEWJOB +0 or a
 * core set and FIXLOC one of the VAC areas. Always ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state, io record and channels as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "rope_image.h"
#include "../p70_p71/p70_p71.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))
#define SUPERBANK_BIT      0100u

/* LOADTIME's bank (FB), from its FCADR. */
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))

static const agc_input_channels *inputs_now;
static agc_hardware *hardware_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channel 30, as the run latched it, and 11 and 14, the hardware's
 * latches. */
static word channel_in(unsigned number)
{
    if (number == 030u) return inputs_now->channel30;
    if (!hardware_now) not_modelled();
    if (number == 011u) return hardware_now->channel11;
    if (number != 014u) not_modelled();
    return hardware_now->channel14;
}

/* Channels 11 (WRITE) and 14 (WOR). */
static void channel_out(unsigned number, word value)
{
    if (!hardware_now) not_modelled();
    if (number == 011u)
        hardware_now->channel11 = value;
    else if (number == 014u)
        hardware_now->channel14 = value;
    else
        not_modelled();
}

/* The interpreter's fixed operands are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, lm_channel_io *io)
{
    bind_machine(program, state, &io->io);
    inputs_now = &io->in;
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take (its basic_sane; with a work
 * area, its sane(), at the order pairs). */
static bool sane(bool with_work_area)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (with_work_area &&
        (newjob > 037777u || FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u))
        return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* The key's bank selected: FB, and in bank 32 the superbank bit clear
 * (with it set, FB 32 would be bank 42). */
static bool in_bank(unsigned bank)
{
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The basic code ---- */

/* Each word the faithful port has a routine at, and the readable one. */
static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* CONTABRT and ABRTJASK (contabrt.c) */
    {KEY(05u, CONTABRT), contabrt},
    {KEY(05u, ABRTJASK), abrtjask},
    {KEY(05u, ABRTJASK + 037u), abrtjask_after_clradmod},
    /* R10,R11, P70, P71 and LEGAL? (abort.c) */
    {KEY(021u, R10_R11), r10_r11},
    {KEY(021u, R10_R11A), r10_r11a},
    {KEY(021u, FLASHV_Q), flashv_q},
    {KEY(021u, TEN_11), ten_11},
    {KEY(021u, P70), p70},
    {KEY(021u, P70A), p70a},
    {KEY(021u, P71), p71},
    {KEY(021u, P71A), p71a},
    {KEY(021u, LEGAL_Q), legal_q},
    {KEY(021u, ABORTALM + 1u), abortalm_after_falton},
    {KEY(021u, ABORTALM + 2u), abortalm_after_reldsp},
    /* GOABORT, P70INIT, UPTHROT, P71RET and THROTUP (goabort.c) */
    {KEY(032u, GOABORT), goabort},
    {KEY(032u, GOABORT + 4u), goabort_after_exit},
    {KEY(032u, GOABORT + 012u), goabort_after_flrcs},
    {KEY(032u, GOABORT + 014u), goabort_after_flundisp},
    {KEY(032u, GOABORT + 016u), goabort_after_idleflag},
    {KEY(032u, GOABORT + 020u), goabort_after_acc4_2fl},
    {KEY(032u, GOABORT + 022u), goabort_after_checkmm},
    {KEY(032u, P70INIT), p70init},
    {KEY(032u, UPTHROT + 2u), upthrot_after_exit},
    {KEY(032u, UPTHROT + 4u), upthrot_after_rotflag},
    {KEY(032u, UPTHROT + 5u), upthrot_after_throtup}, /* (not a key) */
    {KEY(032u, UPTHROT + 7u), upthrot1_minus3},
    {KEY(032u, UPTHROT1 - 1u), upthrot1_after_p40auto},
    {KEY(032u, UPTHROT1), upthrot1}, /* (not a key) */
    {KEY(032u, GRP4OFF + 2u), grp4off_after_phaschng},
    {KEY(032u, P71RET), p71ret},
    {KEY(032u, P71RET + 2u), p71ret_after_letabort},
    {KEY(032u, OLDTIME + 5u), p71ret_after_exit},
    {KEY(032u, OLDTIME + 7u), p71ret_after_phaschng},
    {KEY(032u, THROTUP), throtup},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* The routine at the word Z holds, if the run came back into the file
 * there: none in erasable memory. */
static const struct basic_entry *entry_at_z(void)
{
    return Z < 02000u ? NULL : basic_entry((uint16_t)fixed_physical(Z));
}

/* A routine, then each routine the run comes back into. */
static void run_from(const struct basic_entry *entry)
{
    do entry->routine();
    while ((entry = entry_at_z()) != NULL);
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {&GOABORT_CODE, &P70INIT_CODE, &P71RET_CODE,
                                                        &TGOCOMP_CODE};

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

/* A basic key's routine, and on (run_from). */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    agc_word channel11, channel14;
    uint16_t in_interrupt;

    if (!entry || !program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank(key / 1024u) || !sane(false)) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    channel11 = hardware_now ? hardware_now->channel11 : 0u;
    channel14 = hardware_now ? hardware_now->channel14 : 0u;
    in_interrupt = hardware_now ? hardware_now->in_interrupt : 0u;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        if (hardware_now) {
            hardware_now->channel11 = channel11;
            hardware_now->channel14 = channel14;
            hardware_now->in_interrupt = in_interrupt;
        }
        return EXEC_OUT_OF_SCOPE;
    }
    run_from(entry);
    return EXEC_OK;
}

/* One pass; after TGOCOMP's RTB LOADTIME, LOADTIME and the DANZIG it ends
 * at, on through the rest of the pair, as the faithful pair does. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 lm_channel_io *io, const struct interpretive_code *code)
{
    enum exec_status status = interpret_listing(program, state, &io->io, code);

    if (status != EXEC_OK) return status;
    bind(program, state, io);
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (!sane(true)) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, &io->io, code);
}

static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane(true))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = one_pass(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool p70_p71_readable(uint16_t key, bool order_pair)
{
    return order_pair ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL
                      : basic_entry(key) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)                                                                     \
    static enum exec_status pair_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return pair(KEY(bank, address), program, state, device);                                   \
    }
#include "p70_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "p70_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "p70_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table lm_p70_p71_file_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_p70_p71_file_basic = {basic_keys, COUNT(basic_keys)};

/* The older module's tables: empty. */
static const agc_routine_entry none[] = {{0xFFFFu, NULL}};
const agc_routine_table lm_p70_p71_basic = {none, 0u};
const agc_routine_table lm_p70_p71_interpretive = {none, 0u};

#endif
