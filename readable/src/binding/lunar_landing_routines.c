/* SPDX-License-Identifier: MIT */
/* The readable THE_LUNAR_LANDING (src/lunar_landing/) as routines of the
 * LM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_lunar_landing.c): lm_lunar_landing_routines (the
 * basic code) and lm_lunar_landing_interpretive (the order pairs).
 *
 * Plumbing only. A routine runs the file's code from its key until control
 * leaves the file, as the faithful one does. Each readable function ends
 * where its code leaves it: at a TC or TCF out of the file, or at a TC
 * INTPRET, which runs INTPRET on to NEWOPS here. While Z is the file's
 * code the run goes on there: a basic word through its function (an
 * EXIT's return too), an order pair (NEWOPS, LOC on one under BANKSET) as
 * one pass of the readable interpreter over the listing's words
 * (interpret_listing()). It stops where Z leaves the file: a basic address
 * of another file, or NEWOPS with LOC outside it (a CALL, STCALL or RTB),
 * or an alarm.
 *
 * Built with AGC_FLIGHT_INTERPRETER, as the faithful port's flight program
 * builds its file, a run stops instead where it reaches NEWOPS: there the
 * flight program's interpreter takes the file's interpretive code, and
 * each EXIT comes back to a basic key of its own.
 *
 * The states the faithful routines take: each key under its bank's FB
 * (the superbank bit clear); at an order pair FB and BANKSET that bank,
 * EDOP +0 and the work area one of the five VAC areas with PUSHLOC in it.
 * A word of the file that is no function's or pair's, and anything the
 * readable code calls not_modelled(), is refused: the run leaves the state
 * and the io record as they were. The io record is lm_lld_io: channels 12
 * and 33, the interrupt and superbank latches, and the undo image the
 * caller supplies. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "flight_undo.h"
#include "lm_lunar_landing.h"
#include "rope_image.h"
#include "../lunar_landing/lunar_landing.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

#define RUN_LIMIT 600u
/* Keys: the physical address of a word of bank b. */
#define BANK_KEY(b, z) (uint16_t)((b) * 1024u + ((z) - 02000u))
#define K31(z)         BANK_KEY(031u, z)
#define K32(z)         BANK_KEY(032u, z)

static jmp_buf back;
static lm_lld_io *io_record;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* The file's channels: 12 (WAND) and 33 (RAND). */
static agc_word *channel(unsigned number)
{
    switch (number) {
    case 012u: return &io_record->chan12;
    case 033u: return &io_record->chan33;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return *channel(number);
}

static void channel_out(unsigned number, word value)
{
    *channel(number) = (agc_word)value;
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, lm_lld_io *io)
{
    bind_machine(program, state, &io->control);
    io_record = io;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

static uint16_t physical(word address, word bank_register)
{
    return agc_fixed_physical(address, bank_register, *agc.superbank);
}

/* The file's words: bank 31 02144-02214 (LANDJUNK) and bank 32
 * 02776-03252 (P63LM .. P63SPOT4). */
static bool in_file(uint16_t at)
{
    return (at >= K31(02144u) && at <= K31(02214u)) || (at >= K32(02776u) && at <= K32(03252u));
}

/* Each word of the file a basic run starts at: the keys and the words
 * after each EXIT. */
static const struct {
    uint16_t at;
    void (*routine)(void);
} basic_entries[] = {
    {K31(K_LANDJUNK), landjunk},
    {K31(K_LANDJUNK + 2u), landjunk_after_phaschng},
    {K31(K_LANDJUNK + 5u), landjunk_after_zatteror},
    {K31(K_LANDJUNK + 7u), landjunk_after_setmaxdb},
    {K31(K_LANDJUNK + 034u), landjunk_after_exit},
    {K31(K_LANDJUNK + 037u), landjunk_terminate},
    {K31(K_LANDJUNK + 040u), landjunk_proceed},
    {K31(K_LANDJUNK + 041u), landjunk_recycle},
    {K31(K_LANDJUNK + 050u), landjunk_after_refmf_exit},
    {K32(K_P63LM), p63lm},
    {K32(K_P63LM + 2u), p63lm_after_phaschng},
    {K32(K_P63LM + 4u), p63lm_after_r02both},
    {K32(K_DDUMCALC), ddumcalc},
    {K32(K_IGNALGRT), ignalgrt},
    {K32(K_ASTNCLOK), astnclok},
    {K32(K_ASTNCLOK + 3u), astnclok_after_stclok2},
    {K32(K_ASTNRET), astnret},
    {K32(K_P63SPOT2 + 6u), p63spot2_after_exit},
    {K32(K_P63SPOT2 + 013u), p63spot2_after_pflitedb},
    {K32(K_P63SPOT2 + 016u), p63spot2_after_r60lem},
    {K32(K_P63SPOT3), p63spot3},
    {K32(K_P63SPOT3 + 010u), p63spot3_terminate},
    {K32(K_P63SPOT3 + 011u), p63spot3_proceed},
    {K32(K_P63SPOT4), p63spot4},
    {K32(K_P63SPOT4 + 2u), p63spot4_after_setpos1},
};

static const agc_routine_entry pair_keys[69];

/* The function at the word Z names, under FB, or none. */
static void (*basic_at(uint16_t at))(void)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == at) return basic_entries[i].routine;
    return NULL;
}

/* The table of the order pair LOC is on, or none (an operand word). */
static const struct interpretive_code *code_at(uint16_t at)
{
    static const struct interpretive_code *const codes[] = {
        &IGNALG_CODE, &DDUMCALC_CODE, &P63SPOT2_CODE, &LANDJUNK_CODE, &GSAV_CODE};
    unsigned i, k;

    for (i = 0; i < COUNT(pair_keys); ++i) {
        if (pair_keys[i].physical != at) continue;
        for (k = 0; k < COUNT(codes); ++k)
            if (at / 1024u == codes[k]->bank && LOC >= codes[k]->first &&
                LOC < codes[k]->first + codes[k]->count)
                return codes[k];
    }
    return NULL;
}

static bool work_area(void)
{
    unsigned area;

    if (EDOP != 0u) return false;
    for (area = 0401u; area <= 0661u; area += 054u)
        if (FIXLOC == area) return PUSHLOC >= area && PUSHLOC <= area + 042u;
    return false;
}

enum step { GO_ON, LEFT, REFUSED };

static enum step step(const agc_native_program *program, apollo_overlay_state *state, lm_lld_io *io)
{
    const struct interpretive_code *code;
    void (*routine)(void);
    uint16_t at;

    if (Z == NEWOPS) {
        at = physical(LOC, BANKSET);
        if (at == AGC_TRANSFER_NONE || !in_file(at)) return LEFT;
#ifdef AGC_FLIGHT_INTERPRETER
        return LEFT; /* the flight program's interpreter goes on */
#endif
        code = code_at(at);
        if (!code || ((FBANK ^ BANKSET) & 076000u) || !work_area()) return REFUSED;
        if (interpret_listing(program, state, &io->control, code) != EXEC_OK) return REFUSED;
    } else {
        at = Z < 02000u ? AGC_TRANSFER_NONE : physical(Z, FBANK);
        if (at == AGC_TRANSFER_NONE || !in_file(at)) return LEFT;
        routine = basic_at(at);
        if (!routine) return REFUSED; /* a word no function starts at */
        if (setjmp(back)) return REFUSED;
        routine();
        if (Z == INTPRET) intpret(); /* the file's TC INTPRETs, on to NEWOPS */
    }
    /* (interpret_listing pointed the machine at its own hooks) */
    bind(program, state, io);
    return GO_ON;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static const agc_native_program none;
    lm_lld_io *io = device, entry;
    unsigned n;

    if (!state || !io || !io->undo || FLIGHT_UNDO_ALIAS(io, state)) return EXEC_OUT_OF_SCOPE;
    if (!program) program = &none;
    FLIGHT_UNDO_SAVE(io, state);
    entry = *io;
    bind(program, state, io);
    for (n = 0; n < RUN_LIMIT; ++n) {
        switch (step(program, state, io)) {
        case GO_ON: continue;
        case LEFT:
            if (n > 0u) return EXEC_OK;
            break;
        case REFUSED: break;
        }
        break;
    }
    FLIGHT_UNDO_RESTORE(&entry, state);
    *io = entry;
    return EXEC_OUT_OF_SCOPE;
}

/* Keys, by physical address: the faithful tables'. The words after the
 * EXITs are keys of the flight program's alone. */
#ifdef AGC_FLIGHT_INTERPRETER
#define FLIGHT_KEY(z) {z, run},
#else
#define FLIGHT_KEY(z)
#endif
static const agc_routine_entry basic_keys[] = {
    {K31(K_LANDJUNK), run},
    {K31(K_LANDJUNK + 2u), run},
    {K31(K_LANDJUNK + 5u), run},
    {K31(K_LANDJUNK + 7u), run},
    FLIGHT_KEY(K31(K_LANDJUNK + 034u)){K31(K_LANDJUNK + 037u), run},
    {K31(K_LANDJUNK + 040u), run},
    {K31(K_LANDJUNK + 041u), run},
    FLIGHT_KEY(K31(K_LANDJUNK + 050u)){K32(K_P63LM), run},
    {K32(K_P63LM + 2u), run},
    {K32(K_P63LM + 4u), run},
    {K32(K_DDUMCALC), run},
    FLIGHT_KEY(K32(K_IGNALGRT)){K32(K_ASTNCLOK), run},
    {K32(K_ASTNCLOK + 3u), run},
    {K32(K_ASTNRET), run},
    FLIGHT_KEY(K32(K_P63SPOT2 + 6u)){K32(K_P63SPOT2 + 013u), run},
    {K32(K_P63SPOT2 + 016u), run},
    {K32(K_P63SPOT3), run},
    {K32(K_P63SPOT3 + 010u), run},
    {K32(K_P63SPOT3 + 011u), run},
    {K32(K_P63SPOT4), run},
    {K32(K_P63SPOT4 + 2u), run},
};
/* Each order pair's first word (and each store code's that starts a
 * line): LANDJUNK's, the P57 set-up's, P63's flags to IGNALOOP's,
 * DDUMCALC's to R60INIT's, ASTNRET's and P63SPOT2's. */
#define PAIR31(z) {K31(z), run}
#define PAIR32(z) {K32(z), run}
static const agc_routine_entry pair_keys[69] = {
    PAIR31(02154u), PAIR31(02157u), PAIR31(02162u), PAIR31(02164u), PAIR31(02167u), PAIR31(02172u),
    PAIR31(02174u), PAIR31(02176u), PAIR31(02177u), PAIR31(02207u), PAIR31(02211u), PAIR31(02213u),
    PAIR32(03020u), PAIR32(03023u), PAIR32(03026u), PAIR32(03031u), PAIR32(03034u), PAIR32(03036u),
    PAIR32(03040u), PAIR32(03042u), PAIR32(03044u), PAIR32(03047u), PAIR32(03051u), PAIR32(03055u),
    PAIR32(03057u), PAIR32(03061u), PAIR32(03063u), PAIR32(03065u), PAIR32(03067u), PAIR32(03070u),
    PAIR32(03072u), PAIR32(03074u), PAIR32(03076u), PAIR32(03100u), PAIR32(03104u), PAIR32(03107u),
    PAIR32(03111u), PAIR32(03114u), PAIR32(03116u), PAIR32(03120u), PAIR32(03122u), PAIR32(03125u),
    PAIR32(03127u), PAIR32(03132u), PAIR32(03133u), PAIR32(03134u), PAIR32(03136u), PAIR32(03140u),
    PAIR32(03141u), PAIR32(03143u), PAIR32(03146u), PAIR32(03151u), PAIR32(03153u), PAIR32(03155u),
    PAIR32(03157u), PAIR32(03161u), PAIR32(03163u), PAIR32(03166u), PAIR32(03170u), PAIR32(03172u),
    PAIR32(03174u), PAIR32(03176u), PAIR32(03200u), PAIR32(03201u), PAIR32(03211u), PAIR32(03215u),
    PAIR32(03217u), PAIR32(03221u), PAIR32(03222u)};
const agc_routine_table lm_lunar_landing_routines = {basic_keys, COUNT(basic_keys)};
const agc_routine_table lm_lunar_landing_interpretive = {pair_keys, COUNT(pair_keys)};

#endif
