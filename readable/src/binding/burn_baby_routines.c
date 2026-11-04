/* SPDX-License-Identifier: MIT */
/* The readable BURN, BABY, BURN (src/burn_baby/) as routines of the LM's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_burn_baby.c): lm_burn_baby_routines (the basic code)
 * and lm_burn_baby_interpretive (P41SPOT's and the ABVEL display's order
 * pairs).
 *
 * Plumbing only. A routine runs the file's code from its key until control
 * leaves the file, as the faithful one does. Each readable function ends
 * where its code leaves it: at a TC or TCF out of the file, at a TC INTPRET
 * (which runs INTPRET on to NEWOPS here), or at a transfer to an address
 * the code computes (a TC Q return, NULLCLOK's TC P40/RET, DEAD's DTCB,
 * CLOKJOB's INDEX A). While that address is the file's code the run goes
 * on there: a basic word through its function, an order pair (NEWOPS, LOC
 * on one under BANKSET) as one pass of the readable interpreter over the
 * listing's words (interpret_listing()). It stops where Z leaves the file
 * (a basic address of another file, or NEWOPS with LOC outside it).
 *
 * Built with AGC_FLIGHT_INTERPRETER, as the faithful port's flight program
 * builds its file, a run stops instead where it reaches NEWOPS: there the
 * flight program's interpreter takes the file's interpretive code, and the
 * ABVEL display's EXIT comes back to one more basic key (CALLT-35 +026).
 *
 * The states the faithful routines take: each key under its bank's FB (the
 * LM's superbank bit clear), KILLTASK under any; at an order pair FB and
 * BANKSET bank 36, EDOP +0 and the work area one of the five VAC areas with
 * PUSHLOC in it. KILLTASK only from one of the program's TC KILLTASKs (the
 * word before Q). A computed transfer to a word of the file that is no
 * function's (one after an EXTEND or INDEX, or the middle of a routine) is
 * not modelled, nor is anything the readable code calls not_modelled():
 * the run is refused and leaves the state and the io record as they were.
 * The io record is lm_bb_io: channels 11, 14 and 30, the interrupt and
 * superbank latches, and the undo image the caller supplies. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "flight_undo.h"
#include "lm_burn_baby.h"
#include "rope_image.h"
#include "../burn_baby/burn_baby.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

#define RUN_LIMIT 600u
/* Keys: the physical address of a word of bank b, or of fixed-fixed. */
#define BANK_KEY(b, z) (uint16_t)((b) * 1024u + ((z) - 02000u))
#define K17(z)         BANK_KEY(017u, z)
#define K27(z)         BANK_KEY(027u, z)
#define K35(z)         BANK_KEY(035u, z)
#define K36(z)         BANK_KEY(036u, z)
#define KFF(z)         (uint16_t)(z)

static jmp_buf back;
static lm_bb_io *io_record;

/* For the tests: the word of the file that the last refused run reached
 * after its first step, by a computed transfer, and no readable function
 * starts at, or 0. The faithful port runs on from any word but one after
 * an EXTEND or INDEX. */
uint16_t lm_burn_baby_unentered;

static void leave(void)
{
    longjmp(back, 1);
}

/* The file's fixed words are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* The file's channels: 11 (DSALMOUT), 14 and 30. */
static agc_word *channel(unsigned number)
{
    switch (number) {
    case 011u: return &io_record->chan11;
    case 014u: return &io_record->chan14;
    case 030u: return &io_record->chan30;
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

static void bind(const agc_native_program *program, apollo_overlay_state *state, lm_bb_io *io)
{
    bind_machine(program, state, &io->control);
    io_record = io;
    agc.dsalmout = &io->chan11;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

static uint16_t physical(word address, word bank_register)
{
    return agc_fixed_physical(address, bank_register, *agc.superbank);
}

/* The file's words: bank 36 02022-03146, bank 35 03747-03775, bank 17
 * 02000-02026, bank 27 02200-02246 and KILLTASK's in fixed-fixed. */
static bool in_file(uint16_t at)
{
    return (at >= K36(02022u) && at <= K36(03146u)) || (at >= K35(03747u) && at <= K35(03775u)) ||
           (at >= K17(02000u) && at <= K17(02026u)) || (at >= K27(02200u) && at <= K27(02246u)) ||
           (at >= KFF(06027u) && at <= KFF(06036u));
}

/* Each word of the file a basic run starts at: the keys and the returns of
 * the file's own subroutines (NOULLAGE, KILLTASK, NULLCLOK). */
static const struct {
    uint16_t at;
    void (*routine)(void);
} basic_entries[] = {
    {KFF(K_KILLTASK), killtask},
    {K17(K_TIGTASK1), tigtask1},
    {K17(K_TIGTASK1 + 4u), tigtask1_after_novac},
    {K17(K_TIGTASK1 + 6u), tigtask1_after_phaschng},
    {K17(K_P63ZOOM), p63zoom},
    {K17(K_P63ZOOM + 5u), p63zoom_after_flatout},
    {K17(K_P40ZOOM), p40zoom},
    {K17(K_P40ZOOMA), p40zooma},
    {K17(K_P40ZOOMA + 2u), p40zooma_after_phaschng},
    {K27(K_KILLTSK2), killtsk2},
    {K27(K_ADRSCAN), adrscan},
    {K27(K_LETITLIV), letitliv},
    {K27(K_DEAD), dead},
    {K27(K_TSTFBANK), tstfbank},
    {K27(K_KILLDEAD), killdead},
    {K35(K_P40AUTO), p40auto},
    {K35(K_P40AUTO + 1u), p40auto_after_makecadr},
    {K35(K_P40A_P), p40a_p},
    {K35(K_P40A_P + 2u), p40a_p_after_gpn_auto},
    {K35(K_TURNITON), turniton},
    {K35(K_TURNITON + 3u), turniton_terminate},
    {K35(K_TURNITON + 4u), turniton_recycle},
    {K35(K_GOBACK), goback},
    {K36(K_P41TABLE), p41table},
    {K36(K_BURNBABY), burnbaby},
    {K36(K_BURNBABY + 2u), burnbaby_after_phaschng},
    {K36(K_B_STAR_RNB_STAR_B_STAR), b_star_rnb_star_b_star},
    {K36(K_B_STAR_RNB_STAR_B_STAR + 6u), b_star_rnb_star_b_star_after_enginof3},
    {K36(K_P40SPOT), p40spot},
    {K36(K_P41SPOT), p41spot},
    {K36(K_GOMIDAV + 2u), gomidav_after_midtoav1},
    {K36(K_GOMIDAV + 3u), gomidav_after_midtoav1_slipped},
    {K36(K_CALLT_35), callt_35},
    {K36(K_CALLT_35 + 012u), callt_35_after_longcall},
    {K36(K_CALLT_35 + 014u), callt_35_after_phaschng},
    {K36(K_CALLT_35 + 016u), callt_35_not_p63},
    {K36(K_CALLT_35 + 017u), callt_35_p63},
    {K36(K_CALLT_35 + 026u), callt_35_after_exit},
    {K36(K_TIG_35), tig_35},
    {K36(K_TIG_35 + 3u), tig_35_after_twiddle},
    {K36(K_TIG_35 + 5u), tig_35_after_phaschng},
    {K36(K_TIG_35 + 016u), tig_35_after_second_twiddle},
    {K36(K_TIG_35 + 022u), tig_35_after_novac},
    {K36(K_P41BLANK), p41blank},
    {K36(K_P41BLANK + 2u), p41blank_after_cleandsp},
    {K36(K_TIG_30_1), tig_30_1},
    {K36(K_TIG_30_1 + 4u), tig_30_1_after_novac},
    {K36(K_TIG_30A), tig_30a},
    {K36(K_TIG_30), tig_30},
    {K36(K_TIG_30 + 3u), tig_30_after_twiddle},
    {K36(K_TIG_30 + 014u), tig_30_after_second_twiddle},
    {K36(K_WANTAPS), wantaps},
    {K36(K_ULLGNOT), ullgnot},
    {K36(K_REDO2_17), redo2_17},
    {K36(K_REDO4_2), redo4_2},
    {K36(K_ULLGTASK), ullgtask},
    {K36(K_ULLGTASK + 1u), ullgtask_after_onullage},
    {K36(K_ULLGTASK + 3u), ullgtask_after_phaschng},
    {K36(K_TIG_5), tig_5},
    {K36(K_TIG_5 + 6u), tig_5_after_twiddle},
    {K36(K_TIG_5 + 010u), tig_5_after_downflag},
    {K36(K_TIG_5 + 012u), tig_5_after_second_downflag},
    {K36(K_P40SJUNK), p40sjunk},
    {K36(K_P40SJUNK + 6u), p40sjunk_after_findvac},
    {K36(K_DISPCHNG), dispchng},
    {K36(K_COMMON), common},
    {K36(K_COMMON + 2u), common_after_phaschng},
    {K36(K_TIG_0), tig_0},
    {K36(K_TIG_0 + 5u), tig_0_not_p63},
    {K36(K_TIG_0 + 6u), tig_0_p63},
    {K36(K_TIG_0 + 012u), tig_0_after_waitlist},
    {K36(K_IGNYET_Q), ignyet_q},
    {K36(K_IGNITION), ignition},
    {K36(K_P63IGN), p63ign},
    {K36(K_P40IGN), p40ign},
    {K36(K_P63IGN1), p63ign1},
    {K36(K_P63IGN1 + 4u), p63ign1_after_2phschng},
    {K36(K_P12IGN), p12ign},
    {K36(K_ABRTIGN), abrtign},
    {K36(K_P42IGN), p42ign},
    {K36(K_DVMONCON), dvmoncon},
    {K36(K_DVMONCON + 2u), dvmoncon_after_downflag},
    {K36(K_DVMONCON + 4u), dvmoncon_after_second_downflag},
    {K36(K_DVMONCON + 6u), dvmoncon_after_third_downflag},
    {K36(K_DVMONCON + 010u), dvmoncon_after_phaschng},
    {K36(K_ULLAGOFF), ullagoff},
    {K36(K_WAITABIT), waitabit},
    {K36(K_TIGTASK), tigtask},
    {K36(K_COMFAIL), comfail},
    {K36(K_COMFAIL + 2u), comfail_after_upflag},
    {K36(K_COMFAIL + 4u), comfail_after_second_upflag},
    {K36(K_COMFAIL + 012u), comfail_after_stclok1},
    {K36(K_COMFAIL + 016u), comfail_after_phaschng},
    {K36(K_COMFAIL1), comfail1},
    {K36(K_COMFAIL3), comfail3},
    {K36(K_COMFAIL4), comfail4},
    {K36(K_COMFAIL4 + 4u), comfail4_after_downflag},
    {K36(K_COMFAIL4 + 6u), comfail4_after_second_downflag},
    {K36(K_COMFAIL2), comfail2},
    {K36(K_COMFAIL2 + 2u), comfail2_after_phaschng},
    {K36(K_COMFAIL2 + 5u), comfail2_after_killtask},
    {K36(K_COMFAIL2 + 7u), comfail2_after_enginof4},
    {K36(K_COMFAIL2 + 011u), comfail2_after_upflag},
    {K36(K_COMFAIL2 + 013u), comfail2_after_invflag},
    {K36(K_COMFAIL2 + 015u), comfail2_after_second_upflag},
    {K36(K_COMFAIL2 + 021u), comfail2_after_twiddle},
    {K36(K_INVFLAG), invflag},
    {K36(K_INVFLAG + 2u), invflag_after_debit},
    {K36(K_NOULLAGE), noullage},
    {K36(K_ONULLAGE), onullage},
    {K36(K_STCLOK1), stclok1},
    {K36(K_STCLOK2), stclok2},
    {K36(K_STCLOK3), stclok3},
    {K36(K_STCLOK3 + 1u), stclok3_after_makecadr},
    {K36(K_STCLOK3 + 011u), stclok3_after_tpagree},
    {K36(K_STCLOK3 + 024u), stclok3_after_twiddle},
    {K36(K_STCLOK3 + 030u), stclok3_after_2phschng},
    {K36(K_CLOKTASK), cloktask},
    {K36(K_CLOKTASK + 011u), cloktask_after_novac},
    {K36(K_CLOKTASK + 013u), cloktask_after_fixdelay},
    {K36(K_KILLCLOK), killclok},
    {K36(K_CLOKJOB), clokjob},
    {K36(K_DISPNOT - 035u), dispnot_minus35},
    {K36(K_DISPNOT - 030u), dispnot_minus35_terminate},
    {K36(K_DISPNOT - 027u), dispnot_minus35_proceed},
    {K36(K_DISPNOT - 026u), dispnot_minus35_enter},
    {K36(K_DISPNOT - 025u), dispnot_minus25},
    {K36(K_DISPNOT - 022u), dispnot_minus25_terminate},
    {K36(K_DISPNOT - 021u), dispnot_minus25_proceed},
    {K36(K_DISPNOT - 020u), dispnot_minus25_enter},
    {K36(K_DISPNOT - 017u), dispnot_minus17},
    {K36(K_DISPNOT - 013u), dispnot_minus13},
    {K36(K_DISPNOT - 5u), dispnot_minus13_terminate},
    {K36(K_DISPNOT - 4u), dispnot_minus13_proceed},
    {K36(K_DISPNOT - 3u), dispnot_minus13_enter},
    {K36(K_DISPNOT - 2u), dispnot_minus2},
    {K36(K_DISPNOT), dispnot},
    {K36(K_STOPCLOK), stopclok},
    {K36(K_STOPCLOK + 1u), stopclok_after_nullclok},
    {K36(K_NULLCLOK), nullclok},
    {K36(K_NULLCLOK + 4u), nullclok_after_noullage},
    {K36(K_NULLCLOK + 6u), nullclok_after_killtask},
    {K36(K_NULLCLOK + 010u), nullclok_after_phaschng},
    {K36(K_ASTNRETN), astnretn},
    {K36(K_ASTNRETN + 2u), astnretn_after_phaschng},
    {K36(K_ASTNRETN + 010u), astnretn_after_findvac},
    {K36(K_STAR_PROCEED), star_proceed},
    {K36(K_STAR_PROCEED + 2u), star_proceed_after_upflag},
    {K36(K_STAR_ENTER), star_enter},
    {K36(K_GOPOST), gopost},
    {K36(K_GOPOST + 4u), gopost_after_findvac},
    {K36(K_GOPOST + 7u), gopost_after_allcoast},
    {K36(K_GOPOST + 010u), gopost_after_nullclok},
    {K36(K_GOPOST + 012u), gopost_after_phaschng},
    {K36(K_GOCUTOFF), gocutoff},
    {K36(K_GOCUTOFF + 4u), gocutoff_after_findvac},
    {K36(K_GOCUTOFF + 6u), gocutoff_after_downflag},
    {K36(K_GOCUTOFF + 011u), gocutoff_after_allcoast},
    {K36(K_GOCUTOFF + 012u), gocutoff_after_nullclok},
    {K36(K_GOCUTOFF + 017u), gocutoff_after_phaschng},
    {K36(K_IGNITE), ignite},
    {K36(K_IGNITE + 010u), ignite_after_twiddle},
    {K36(K_IGNITE1), ignite1},
    {K36(K_P40ALM), p40alm},
    {K36(K_REP40ALM), rep40alm},
    {K36(K_REP40ALM + 3u), rep40alm_terminate},
    {K36(K_REP40ALM + 4u), rep40alm_proceed},
    {K36(K_REP40ALM + 5u), rep40alm_recycle},
};

static const agc_routine_entry pair_keys[14];

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
    static const struct interpretive_code *const codes[] = {&P41SPOT_CODE, &ABVEL_CODE};
    unsigned i, k;

    for (i = 0; i < COUNT(pair_keys); ++i) {
        if (pair_keys[i].physical != at) continue;
        for (k = 0; k < COUNT(codes); ++k)
            if (LOC >= codes[k]->first && LOC < codes[k]->first + codes[k]->count) return codes[k];
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

/* KILLTASK's caller: the word before Q is a TC KILLTASK (the word 06027,
 * TC's code being 0), and Q names the task's CADR after it. */
static bool a_killtask_call(void)
{
    return Q > 02000u && Q <= 07777u && physical((word)(Q - 1u), FBANK) != AGC_TRANSFER_NONE &&
           fetch_fixed((word)(Q - 1u)) == KILLTASK;
}

enum step { GO_ON, LEFT, REFUSED };

static enum step step(const agc_native_program *program, apollo_overlay_state *state, lm_bb_io *io,
                      unsigned n)
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
        if (!routine) { /* a word no function starts at */
            if (n > 0u) lm_burn_baby_unentered = at;
            return REFUSED;
        }
        if (routine == killtask && !a_killtask_call()) return REFUSED;
        if (setjmp(back)) return REFUSED;
        routine();
        /* P41SPOT's and CALLT-35's TC INTPRET, on to NEWOPS */
        if (routine == p41table || routine == p41spot || routine == callt_35_p63) intpret();
    }
    /* (interpret_listing pointed the machine at its own hooks) */
    bind(program, state, io);
    return GO_ON;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static const agc_native_program none;
    lm_bb_io *io = device, entry;
    unsigned n;

    if (!state || !io || !io->undo || FLIGHT_UNDO_ALIAS(io, state)) return EXEC_OUT_OF_SCOPE;
    if (!program) program = &none;
    FLIGHT_UNDO_SAVE(io, state);
    entry = *io;
    lm_burn_baby_unentered = 0;
    bind(program, state, io);
    for (n = 0; n < RUN_LIMIT; ++n) {
        switch (step(program, state, io, n)) {
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

/* Keys, by physical address: the faithful tables'. */
#ifdef AGC_FLIGHT_INTERPRETER
#define FLIGHT_KEY(z) {z, run},
#else
#define FLIGHT_KEY(z)
#endif
static const agc_routine_entry basic_keys[] = {
    {KFF(K_KILLTASK), run},
    {K17(K_TIGTASK1), run},
    {K17(K_TIGTASK1 + 4u), run},
    {K17(K_TIGTASK1 + 6u), run},
    {K17(K_P63ZOOM), run},
    {K17(K_P63ZOOM + 5u), run},
    {K17(K_P40ZOOM), run},
    {K17(K_P40ZOOMA), run},
    {K17(K_P40ZOOMA + 2u), run},
    {K27(K_KILLTSK2), run},
    {K27(K_ADRSCAN), run},
    {K27(K_LETITLIV), run},
    {K27(K_DEAD), run},
    {K27(K_TSTFBANK), run},
    {K27(K_KILLDEAD), run},
    {K35(K_P40AUTO), run},
    {K35(K_P40AUTO + 1u), run},
    {K35(K_P40A_P), run},
    {K35(K_P40A_P + 2u), run},
    {K35(K_TURNITON), run},
    {K35(K_TURNITON + 3u), run},
    {K35(K_TURNITON + 4u), run},
    {K35(K_GOBACK), run},
    {K36(K_P41TABLE), run},
    {K36(K_BURNBABY), run},
    {K36(K_BURNBABY + 2u), run},
    {K36(K_B_STAR_RNB_STAR_B_STAR), run},
    {K36(K_B_STAR_RNB_STAR_B_STAR + 6u), run},
    {K36(K_P40SPOT), run},
    {K36(K_P41SPOT), run},
    {K36(K_GOMIDAV + 2u), run},
    {K36(K_GOMIDAV + 3u), run},
    {K36(K_CALLT_35), run},
    {K36(K_CALLT_35 + 012u), run},
    {K36(K_CALLT_35 + 014u), run},
    {K36(K_CALLT_35 + 016u), run},
    {K36(K_CALLT_35 + 017u), run},
    FLIGHT_KEY(K36(K_CALLT_35 + 026u)){K36(K_TIG_35), run},
    {K36(K_TIG_35 + 3u), run},
    {K36(K_TIG_35 + 5u), run},
    {K36(K_TIG_35 + 016u), run},
    {K36(K_TIG_35 + 022u), run},
    {K36(K_P41BLANK), run},
    {K36(K_P41BLANK + 2u), run},
    {K36(K_TIG_30_1), run},
    {K36(K_TIG_30_1 + 4u), run},
    {K36(K_TIG_30A), run},
    {K36(K_TIG_30), run},
    {K36(K_TIG_30 + 3u), run},
    {K36(K_TIG_30 + 014u), run},
    {K36(K_WANTAPS), run},
    {K36(K_ULLGNOT), run},
    {K36(K_REDO2_17), run},
    {K36(K_REDO4_2), run},
    {K36(K_ULLGTASK), run},
    {K36(K_ULLGTASK + 3u), run},
    {K36(K_TIG_5), run},
    {K36(K_TIG_5 + 6u), run},
    {K36(K_TIG_5 + 010u), run},
    {K36(K_TIG_5 + 012u), run},
    {K36(K_P40SJUNK), run},
    {K36(K_P40SJUNK + 6u), run},
    {K36(K_DISPCHNG), run},
    {K36(K_COMMON), run},
    {K36(K_COMMON + 2u), run},
    {K36(K_TIG_0), run},
    {K36(K_TIG_0 + 5u), run},
    {K36(K_TIG_0 + 6u), run},
    {K36(K_TIG_0 + 012u), run},
    {K36(K_IGNYET_Q), run},
    {K36(K_IGNITION), run},
    {K36(K_P63IGN), run},
    {K36(K_P40IGN), run},
    {K36(K_P63IGN1), run},
    {K36(K_P63IGN1 + 4u), run},
    {K36(K_P12IGN), run},
    {K36(K_ABRTIGN), run},
    {K36(K_P42IGN), run},
    {K36(K_DVMONCON), run},
    {K36(K_DVMONCON + 2u), run},
    {K36(K_DVMONCON + 4u), run},
    {K36(K_DVMONCON + 6u), run},
    {K36(K_DVMONCON + 010u), run},
    {K36(K_ULLAGOFF), run},
    {K36(K_WAITABIT), run},
    {K36(K_TIGTASK), run},
    {K36(K_COMFAIL), run},
    {K36(K_COMFAIL + 2u), run},
    {K36(K_COMFAIL + 4u), run},
    {K36(K_COMFAIL + 012u), run},
    {K36(K_COMFAIL + 016u), run},
    {K36(K_COMFAIL1), run},
    {K36(K_COMFAIL3), run},
    {K36(K_COMFAIL4), run},
    {K36(K_COMFAIL4 + 4u), run},
    {K36(K_COMFAIL4 + 6u), run},
    {K36(K_COMFAIL2), run},
    {K36(K_COMFAIL2 + 2u), run},
    {K36(K_COMFAIL2 + 7u), run},
    {K36(K_COMFAIL2 + 011u), run},
    {K36(K_COMFAIL2 + 013u), run},
    {K36(K_COMFAIL2 + 015u), run},
    {K36(K_COMFAIL2 + 021u), run},
    {K36(K_INVFLAG), run},
    {K36(K_INVFLAG + 2u), run},
    {K36(K_NOULLAGE), run},
    {K36(K_ONULLAGE), run},
    {K36(K_STCLOK1), run},
    {K36(K_STCLOK2), run},
    {K36(K_STCLOK3), run},
    {K36(K_STCLOK3 + 1u), run},
    {K36(K_STCLOK3 + 011u), run},
    {K36(K_STCLOK3 + 024u), run},
    {K36(K_STCLOK3 + 030u), run},
    {K36(K_CLOKTASK), run},
    {K36(K_CLOKTASK + 011u), run},
    {K36(K_CLOKTASK + 013u), run},
    {K36(K_KILLCLOK), run},
    {K36(K_CLOKJOB), run},
    {K36(K_DISPNOT - 035u), run},
    {K36(K_DISPNOT - 030u), run},
    {K36(K_DISPNOT - 027u), run},
    {K36(K_DISPNOT - 026u), run},
    {K36(K_DISPNOT - 025u), run},
    {K36(K_DISPNOT - 022u), run},
    {K36(K_DISPNOT - 021u), run},
    {K36(K_DISPNOT - 020u), run},
    {K36(K_DISPNOT - 017u), run},
    {K36(K_DISPNOT - 013u), run},
    {K36(K_DISPNOT - 5u), run},
    {K36(K_DISPNOT - 4u), run},
    {K36(K_DISPNOT - 3u), run},
    {K36(K_DISPNOT - 2u), run},
    {K36(K_DISPNOT), run},
    {K36(K_STOPCLOK), run},
    {K36(K_NULLCLOK), run},
    {K36(K_NULLCLOK + 010u), run},
    {K36(K_ASTNRETN), run},
    {K36(K_ASTNRETN + 2u), run},
    {K36(K_ASTNRETN + 010u), run},
    {K36(K_STAR_PROCEED), run},
    {K36(K_STAR_PROCEED + 2u), run},
    {K36(K_STAR_ENTER), run},
    {K36(K_GOPOST), run},
    {K36(K_GOPOST + 4u), run},
    {K36(K_GOPOST + 7u), run},
    {K36(K_GOPOST + 012u), run},
    {K36(K_GOCUTOFF), run},
    {K36(K_GOCUTOFF + 4u), run},
    {K36(K_GOCUTOFF + 6u), run},
    {K36(K_GOCUTOFF + 011u), run},
    {K36(K_GOCUTOFF + 017u), run},
    {K36(K_IGNITE), run},
    {K36(K_IGNITE + 010u), run},
    {K36(K_IGNITE1), run},
    {K36(K_P40ALM), run},
    {K36(K_REP40ALM), run},
    {K36(K_REP40ALM + 3u), run},
    {K36(K_REP40ALM + 4u), run},
    {K36(K_REP40ALM + 5u), run},
};
/* Each order pair's first word: P41SPOT's and the ABVEL display's. */
#define PAIR(z) {K36(z), run}
static const agc_routine_entry pair_keys[14] = {
    PAIR(K_P41SPOT + 1u),    PAIR(K_P41SPOT + 4u),   PAIR(K_P41SPOT + 6u),
    PAIR(K_P41SPOT + 012u),  PAIR(K_P41SPOT + 015u), PAIR(K_P41SPOT + 016u),
    PAIR(K_P41SPOT + 020u),  PAIR(K_P41SPOT + 022u), PAIR(K_P41SPOT + 024u),
    PAIR(K_P41SPOT + 026u),  PAIR(K_GOMIDAV),        PAIR(K_CALLT_35 + 022u),
    PAIR(K_CALLT_35 + 024u), PAIR(K_CALLT_35 + 025u)};
const agc_routine_table lm_burn_baby_routines = {basic_keys, COUNT(basic_keys)};
const agc_routine_table lm_burn_baby_interpretive = {pair_keys, COUNT(pair_keys)};

#endif
