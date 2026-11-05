/* SPDX-License-Identifier: MIT */
/* The readable SERVICER (src/servicer/) as routines of the LM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_servicer.c): lm_servicer_routines (the basic code) and
 * lm_servicer_interpretive (the order pairs). The tables carry every key
 * of the faithful ones, in their order (servicer_keys.inc, which
 * tools/p20_p25_keys.py writes from them).
 *
 * Plumbing only. A routine runs the file's code from its key until
 * control leaves the file, as the faithful one does. Each readable
 * function ends where its code leaves it: at a TC or TCF out of the file,
 * at a TC to one of the file's subroutines, at a TC INTPRET (which runs
 * INTPRET on to NEWOPS here), or at a transfer to an address the code
 * computes (a TC Q return, SERVOUT's DXCH Z). While that address is the
 * file's code the run goes on there: a basic word through its function,
 * an order pair (NEWOPS, LOC on one under BANKSET) as one pass of the
 * readable interpreter over the listing's words (interpret_listing()). It
 * stops where Z leaves the file (a basic address of another file, or
 * NEWOPS with LOC outside it).
 *
 * The file is readable so far up to UPDATCHK (pp. 857-883): where a run
 * reaches the rest (bank 33 from 03155, and bank 34), the faithful port's
 * routines run it on from there (faithful_lm_servicer_routines, the
 * faithful tables renamed), with the state and io record as the run left
 * them.
 *
 * Built with AGC_FLIGHT_INTERPRETER, as the faithful port's flight program
 * builds its file, a run stops instead where it reaches NEWOPS: there the
 * flight program's interpreter takes the file's interpretive code, and its
 * EXITs come back to basic keys of their own (FLIGHT_BASIC).
 *
 * The states the faithful routines take: each key under its bank's FB; at
 * an order pair FB and BANKSET the pair's bank, EDOP +0 and the work area
 * one of the five VAC areas with PUSHLOC in it. A computed transfer to a
 * word of the file that is no function's (one after an EXTEND or INDEX,
 * or the middle of a routine) is not modelled, nor is anything the
 * readable code calls not_modelled(): the run is refused and leaves the
 * state and the io record as they were. The io record is lm_sv_io:
 * channels 11 (DSALMOUT), 12, 32 and 33, the interrupt and superbank
 * latches, and the undo image the caller supplies. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "flight_undo.h"
#include "lm_servicer.h"
#include "rope_image.h"
#include "../servicer/servicer.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

#define RUN_LIMIT 2000u
/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

static jmp_buf back;
static lm_sv_io *io_record;

/* For the tests: the word of the file that the last refused run reached
 * after its first step, by a computed transfer, and no readable function
 * starts at, or 0. The faithful port runs on from any word but one after
 * an EXTEND or INDEX. */
uint16_t lm_servicer_unentered;

static void leave(void)
{
    longjmp(back, 1);
}

/* The fixed words the file's code reads, its constants: the rope's. (The
 * faithful port holds just these: a read of any other fixed word, an
 * operand indexed out of them, is not modelled.) */
static bool file_constant(unsigned physical)
{
    return (physical >= KEY(033u, 02000u) && physical <= KEY(033u, 02044u)) ||
           (physical >= KEY(033u, 02677u) && physical <= KEY(033u, 02707u)) ||
           (physical >= KEY(033u, 03026u) && physical <= KEY(033u, 03031u)) ||
           (physical >= KEY(033u, 03143u) && physical <= KEY(033u, 03153u)) ||
           (physical >= KEY(023u, 02511u) && physical <= KEY(023u, 02526u));
}

static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (!file_constant(physical) || !apollo_rope_read(0u, (uint16_t)physical, &value))
        not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* The file's channels: 11 (DSALMOUT), 12, 32 and 33. */
static agc_word *channel(unsigned number)
{
    switch (number) {
    case 011u: return &io_record->chan11;
    case 012u: return &io_record->chan12;
    case 032u: return &io_record->chan32;
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

static void bind(const agc_native_program *program, apollo_overlay_state *state, lm_sv_io *io)
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

/* The file's words. */
static bool in_file(uint16_t at)
{
    return (at >= KEY(021u, 02130u) && at <= KEY(021u, 02153u)) ||
           (at >= KEY(022u, 03661u) && at <= KEY(022u, 03710u)) ||
           (at >= KEY(023u, 02461u) && at <= KEY(023u, 02510u)) ||
           (at >= KEY(027u, 03321u) && at <= KEY(027u, 03357u)) ||
           (at >= KEY(032u, 03770u) && at <= KEY(032u, 03772u)) ||
           (at >= KEY(033u, 02206u) && at <= KEY(033u, 03776u)) ||
           (at >= KEY(034u, 03716u) && at <= KEY(034u, 03774u)) ||
           (at >= KEY(037u, 03410u) && at <= KEY(037u, 03654u));
}

/* The words the faithful port runs still: bank 33 from UPDATCHK, and bank
 * 34. */
static bool faithful_part(uint16_t at)
{
    return (at >= KEY(033u, UPDATCHK) && at <= KEY(033u, 03776u)) ||
           (at >= KEY(034u, 03716u) && at <= KEY(034u, 03774u));
}

/* Each word of the file a basic run starts at: the keys, the returns of
 * the file's own subroutines, and the words after the EXITs. */
static const struct {
    uint16_t at;
    void (*routine)(void);
} basic_entries[] = {
    {KEY(021u, LRHTASK), lrhtask},
    {KEY(021u, GRP2OFF), grp2off},
    {KEY(022u, AVGEND), avgend},
    {KEY(022u, AVGEND + 4u), avgend_after_upflag},
    {KEY(022u, AVGEND + 6u), avgend_after_pipfree},
    {KEY(022u, AVGEND + 015u), avgend_after_2phschng},
    {KEY(022u, AVERTRN), avertrn},
    {KEY(023u, NORMLIZE), normlize},
    {KEY(023u, ASCSPOT + 1u), ascspot_after_exit},
    {KEY(027u, SERVIDLE), servidle},
    {KEY(032u, SERVEXIT), servexit},
    {KEY(032u, SERVEXIT + 2u), servexit_after_phaschng},
    {KEY(033u, SERVICER), servicer},
    {KEY(033u, SERVICER + 5u), servicer_after_phaschng},
    {KEY(033u, GETABVAL), getabval},
    {KEY(033u, GETABVAL + 4u), getabval_after_exit},
    {KEY(033u, MASSMON), massmon},
    {KEY(033u, MOONSPOT), moonspot},
    {KEY(033u, MOONSPOT + 2u), moonspot_after_shortmp},
    {KEY(033u, MOONSPOT + 5u), moonspot_after_tmptospt},
    {KEY(033u, MOONSPOT + 7u), moonspot_after_quictrig},
    {KEY(033u, MOONSPOT + 012u), moonspot_after_fleshpot},
    {KEY(033u, GOSERV), goserv},
    {KEY(033u, COPYCYCL), copycycl},
    {KEY(033u, COPYCYCL + 1u), copycycl_after_copycyc},
    {KEY(033u, DVMON), dvmon},
    {KEY(033u, DVCNTSET), dvcntset},
    {KEY(033u, USEGTS), usegts},
    {KEY(033u, NODVMON1), nodvmon1},
    {KEY(033u, NODVMON2), nodvmon2},
    {KEY(033u, LOTHRUST), lothrust},
    {KEY(033u, LOTHRUST + 1u), lothrust_after_quikfaz5},
    {KEY(033u, LOTHRUST + 7u), lothrust_after_phaschng},
    {KEY(033u, LOTHRUST + 013u), lothrust_after_novac},
    {KEY(033u, DECCNTR), deccntr},
    {KEY(033u, DECCNTR + 2u), deccntr_after_quikfaz5},
    {KEY(033u, USEJETS), usejets},
    {KEY(033u, SERVOUT), servout},
    {KEY(033u, SERVOUT + 3u), servout_after_1_accs},
    {KEY(033u, SERVOUT + 011u), servout_after_quikfaz5},
    {KEY(033u, NORMLIZ2), normliz2},
    {KEY(033u, NORMLIZ2 + 2u), normliz2_after_copycyc},
    {KEY(033u, COPYCYC), copycyc},
    {KEY(033u, COPYCYC + 1u), copycyc_plus_1},
    {KEY(033u, TMPTOSPT), tmptospt},
    {KEY(033u, HIGATASK), higatask},
    {KEY(033u, HIGATASK + 011u), higatask_after_findvac},
    {KEY(033u, MUNRETRN + 1u), munretrn_after_exit},
    {KEY(033u, HITEST), hitest},
    {KEY(033u, POS2CHK), pos2chk},
    {KEY(033u, HIGATCHK), higatchk},
    {KEY(033u, POS1CHK), pos1chk},
    {KEY(033u, LRPOSALM), lrposalm},
    {KEY(033u, CONTSERV), contserv},
    {KEY(033u, COPYCYC1), copycyc1},
    {KEY(033u, R29_Q), r29_q},
    {KEY(033u, R29NODES), r29nodes},
    {KEY(033u, NOR29NOW), nor29now},
    {KEY(033u, COPYCYC2 + 1u), copycyc2_after_exit},
    {KEY(033u, ALTCHK), altchk},
    {KEY(033u, ALTCHK + 010u), altchk_after_branch},
    {KEY(033u, ALTCHK + 011u), altchk_after_branch_zero},
    {KEY(033u, ALTCHK + 012u), altchk_after_branch_negative},
    {KEY(033u, XORCHK), xorchk},
    {KEY(033u, XORCHK + 3u), xorchk_after_altchk},
    {KEY(033u, XORCHK + 4u), xorchk_below},
    {KEY(033u, XORCHK + 6u), xorchk_after_xovinflg},
    {KEY(033u, XORCHK + 010u), xorchk_after_xorflg},
    {KEY(033u, THIRTY_FIVE_KCHK), thirty_five_kchk},
    {KEY(033u, THIRTY_FIVE_KCHK + 3u), thirty_five_kchk_after_altchk},
    {KEY(033u, THIRTY_FIVE_KCHK + 4u), thirty_five_kchk_below},
    {KEY(033u, THIRTY_FIVE_KCHK + 6u), thirty_five_kchk_after_readlr},
    {KEY(033u, RVBOTH + 020u), rvboth_after_exit},
    {KEY(033u, RVBOTH + 021u), rvboth_after_quikfaz5},
    {KEY(033u, RVBOTH + 032u), rvboth_after_second_exit},
    {KEY(033u, RVBOTH + 033u), rvboth_after_second_quikfaz5},
    {KEY(037u, PREREAD), preread},
    {KEY(037u, PREREAD + 2u), preread_after_gnufaze5},
    {KEY(037u, BIBIBIAS), bibibias},
    {KEY(037u, BIBIBIAS + 1u), bibibias_after_pipasr},
    {KEY(037u, BIBIBIAS + 017u), bibibias_after_findvac},
    {KEY(037u, GOREADAX), goreadax},
    {KEY(037u, GOREADAX + 1u), goreadax_after_gnutfaz5},
    {KEY(037u, READACCS), readaccs},
    {KEY(037u, PIPSDONE), pipsdone},
    {KEY(037u, REDO5_5), redo5_5},
    {KEY(037u, REDO5_5 + 6u), redo5_5_after_findvac},
    {KEY(037u, MAKEACCS), makeaccs},
    {KEY(037u, AVEGOUT), avegout},
    {KEY(037u, AVEGOUT + 5u), avegout_after_gnutfaz5},
    {KEY(037u, GNUTFAZ5), gnutfaz5},
    {KEY(037u, GNUFAZE5), gnufaze5},
    {KEY(037u, PIPASR), pipasr},
    {KEY(037u, PIPASR + 3u), pipasr_plus_3},
    {KEY(037u, REPIP1), repip1},
    {KEY(037u, REPIP3), repip3},
    {KEY(037u, DODELVZ), dodelvz},
    {KEY(037u, REPIP4), repip4},
    {KEY(037u, REREADAC), rereadac},
    {KEY(037u, CHKTEMX), chktemx},
};

/* The interpretive code, by its first word. */
static const struct interpretive_code *const codes[] = {
    &AVGEND_CODE,   &NORMLIZE_CODE, &GETABVAL_CODE, &AVERAGEG_CODE, &NORMLIZ1_CODE,
    &MUNRETRN_CODE, &NOR29NOW_CODE, &CALCGRAV_CODE, &CALCRVG_CODE,  &RVBOTH_CODE,
    &RVBOTH_2_CODE, &MUNRVG_CODE,   &MUNGRAV_CODE,
};

/* The faithful tables' keys. */
#define BASIC(bank, address) {KEY(bank, address), run},
#ifdef AGC_FLIGHT_INTERPRETER
#define FLIGHT_BASIC(bank, address) {KEY(bank, address), run},
#else
#define FLIGHT_BASIC(bank, address)
#endif
#define PAIR(bank, address)
static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device);
static const agc_routine_entry basic_keys[] = {
#include "servicer_keys.inc"
};
#undef BASIC
#undef FLIGHT_BASIC
#undef PAIR
#define BASIC(bank, address)
#define FLIGHT_BASIC(bank, address)
#define PAIR(bank, address) {KEY(bank, address), run},
static const agc_routine_entry pair_keys[] = {
#include "servicer_keys.inc"
};

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

extern const agc_routine_table faithful_lm_servicer_routines;

/* The faithful routines on from here, with an undo image of their own (a
 * refusal there leaves the state as this run found it at its start: the
 * caller restores it). */
static enum exec_status faithful_rest(const agc_native_program *program,
                                      apollo_overlay_state *state, lm_sv_io *io)
{
    static apollo_overlay_state scratch;
    lm_sv_io rest = *io;
    enum exec_status status;

    rest.undo = &scratch;
    status = faithful_lm_servicer_routines.entries[0].call(program, state, &rest);
    rest.undo = io->undo;
    *io = rest;
    return status;
}

/* Whether store code w would store past the end of the E-bank window
 * (01777): the AGC then writes into fixed memory and reads it back into A
 * and L, which the readable machine does not model. DOSTORE's index (X1
 * or X2) and STORE's addressing, from w - 1 (NEWOPS's CCS); MODE says how
 * many words (DP, TP or vector). The faithful routines refuse such a store
 * too (lmi_store_spills). */
static bool store_spills(word w)
{
    unsigned v = (unsigned)w - 1u, kind = (v >> 11) & 7u, a = v & 03777u, n;

    if (kind == 1u || kind == 2u)
        a = (a + (~agc.memory[(FIXLOC + K_X1 + kind - 1u) & 03777u] & 077777u)) % 077777u;
    if (a <= 45u) return false; /* the work area, or word 45 itself */
    n = MODE == 0u || MODE == 077777u ? 1u : (MODE & 040000u) ? 5u : 2u;
    return (a & 0377u) + n > 0377u;
}

/* ADS: memory plus A, the sum overflow-corrected. */
static word ads_value(word memory, reg16 a)
{
    return overflow_corrected(add16(sign_extend(memory), a));
}

/* Whether CALCGRV1's BDDV* -MUDT,1 reads a fixed word that is not one of
 * the file's constants: -MUDT less X1, the earth's or the moon's (X1 +0
 * or -2, as CALCGRAV's LXC,1 RTX2 leaves it). The readable interpreter
 * reads the rope there, the faithful routines refuse; so the binding works
 * the address out before the pass, as the interpreter's INDEX forms it:
 * the address word less one (CS; CCS), INTBIT15 added where it is fixed
 * (MASK HIGH4), then less X1 (CS X1; ADS ADDRWD). In the work area or
 * erasable memory (E bank ADDRWD's top bits, the word 01400 on by its low
 * eight) it is the faithful routines' too, but for the window's last word
 * (the second word past it); in fixed memory, the bank its top five bits
 * name. */
static bool mu_outside_constants(word address_word)
{
    word sum = (word)(address_word - 1u);
    unsigned physical;

    if (sum & HIGH4) sum = ads_value(sum, sign_extend(INTBIT15));
    sum = ads_value(sum, (reg16)~sign_extend(agc.memory[(FIXLOC + K_X1) & 03777u]));
    if (!(sum & HIGH9)) return false;
    if (!(sum & HIGH4)) return (sum & 0377u) == 0377u;
    physical = ((sum >> 10) & 037u) * 1024u + (sum & 01777u);
    return !file_constant(physical) || !file_constant(physical + 1u);
}

/* The faithful routines' refusals at the start of a pass the readable
 * interpreter would run: a store code that spills, and CALCGRV1's BDDV*
 * off the constants. */
static bool refused_pass(uint16_t at, const struct interpretive_code *code)
{
    word first = code->words[LOC - code->first];

    if (!(first & 040000u) && store_spills(first)) return true;
    return at == KEY(033u, CALCGRV1 + 3u) &&
           mu_outside_constants(code->words[LOC + 1u - code->first]);
}

enum step { GO_ON, LEFT, REFUSED, DONE };

static enum step step(const agc_native_program *program, apollo_overlay_state *state, lm_sv_io *io,
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
        if (faithful_part(at)) return faithful_rest(program, state, io) == EXEC_OK ? DONE : REFUSED;
        code = code_at(at);
        if (!code || ((FBANK ^ BANKSET) & 076000u) || !work_area() || refused_pass(at, code))
            return REFUSED;
        if (interpret_listing(program, state, &io->control, code) != EXEC_OK) return REFUSED;
    } else {
        at = Z < 02000u ? AGC_TRANSFER_NONE : physical(Z, FBANK);
        if (at == AGC_TRANSFER_NONE || !in_file(at)) return LEFT;
        if (faithful_part(at)) return faithful_rest(program, state, io) == EXEC_OK ? DONE : REFUSED;
        routine = basic_at(at);
        if (!routine) { /* a word no function starts at */
            if (n > 0u) lm_servicer_unentered = at;
            return REFUSED;
        }
        if (setjmp(back)) return REFUSED;
        routine();
    }
    /* (interpret_listing pointed the machine at its own hooks) */
    bind(program, state, io);
    return GO_ON;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static const agc_native_program none;
    lm_sv_io *io = device, entry;
    unsigned n;

    if (!state || !io || !io->undo || FLIGHT_UNDO_ALIAS(io, state)) return EXEC_OUT_OF_SCOPE;
    if (!program) program = &none;
    FLIGHT_UNDO_SAVE(io, state);
    entry = *io;
    lm_servicer_unentered = 0;
    bind(program, state, io);
    for (n = 0; n < RUN_LIMIT; ++n) {
        switch (step(program, state, io, n)) {
        case GO_ON: continue;
        case DONE: return EXEC_OK;
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

const agc_routine_table lm_servicer_routines = {basic_keys, COUNT(basic_keys)};
const agc_routine_table lm_servicer_interpretive = {pair_keys, COUNT(pair_keys)};

#endif
