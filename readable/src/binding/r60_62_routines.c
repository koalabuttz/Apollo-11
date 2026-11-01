/* SPDX-License-Identifier: MIT */
/* The readable R60_62 (src/r60_62/) as routines of each mission's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_r60_r62.c and cm_r60_r62.c): <m>_r60_r62_basic (the
 * basic code) and <m>_r60_r62_interpretive (the order pairs). The LM's R62
 * is in bank 23, its R60, BALLANGS and ROT180 in bank 26 and VECPOINT in
 * bank 27; all the CM's are in bank 27.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, MAKECADR, UPFLAG, DOWNFLAG, PHASCHNG, PRIOCHNG,
 * LINUS, BANKJUMP, ENDOFJOB, ENDEXT, GOTOPOOH, GOTOV56, a TC Q or a TC to
 * a saved return) or reaches NEWOPS through a TC INTPRET. An order pair is
 * one pass of the readable interpreter over the listing's words
 * (interpret_listing()); a pass that ends at RTB V1STO2S runs V1STO2S
 * (src/rtb_op_codes/) and the DANZIG it ends at, as the faithful pair
 * does. A pass that ends at RTB READCDUK, an EXIT or a CALL out of the
 * file stops there, as the faithful pair does.
 *
 * The device is the mission's io record with the input channels after it
 * (lm_channel_io, cm_channel_io): the LM's G+N,AUTO and ISITAUTO read
 * channels 30 and 31, the CM's TOBALLC channel 31.
 *
 * The states the faithful routines take, at every key: FB the key's bank;
 * at an order pair Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's
 * bit 15. Always ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range, every erasable word an ordinary one, NEWJOB
 * +0 or a core set and FIXLOC one of the VAC areas. A run the readable
 * code refuses part way leaves the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "rope_image.h"
#include "../r60_62/r60_62.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE 0u /* the LM's rope */
typedef lm_channel_io channel_io;
#else
#define ROPE 1u /* the CM's */
typedef cm_channel_io channel_io;
#endif

/* A bank's FB, and the key of a word in it. */
#define BANK(n)      (word)((n) << 10)
#define KEY(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))
/* The address in its bank of an interpretive label (from its I_ form). */
#define IN_BANK(i) (word)(02000u | ((i) & 01777u))

/* V1STO2S's bank (FB), from its FCADR. */
#define V1STO2S_BANK ((word)(I_V1STO2S & 076000u))

static const agc_input_channels *inputs_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channels 30 and 31, as the run latched them. */
static word channel_in(unsigned number)
{
    if (number == 030u) return inputs_now->channel30;
    if (number != 031u) not_modelled();
    return inputs_now->channel31;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled(); /* R60_62 writes no channel */
}

/* The interpreter's fixed operands (VECPOINT's constants, KEL or DEL,
 * the routines' own constants) are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, channel_io *io)
{
    bind_machine(program, state, &io->io);
    inputs_now = &io->in;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take (its sane()). */
static bool sane(void)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (newjob > 037777u) return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

static const struct {
    word at, bank;
    void (*routine)(void);
} basic_entries[] = {
#if LUMINARY099
    {R62DISP, BANK(023u), r62disp},
    {R62DISP + 3u, BANK(023u), r62disp_terminate},
    {R62DISP + 4u, BANK(023u), r62disp_proceed},
    {R62DISP + 5u, BANK(023u), r62disp_enter},
    {GOMOVE + 2u, BANK(023u), gomove_after_upflag},
    {GOMOVE + 4u, BANK(023u), gomove_after_bankcall},
    {R60LEM, BANK(026u), r60lem},
    {R60LEM + 1u, BANK(026u), r60lem_after_makecadr},
    {TOBALL, BANK(026u), toball},
    {TOBALLA, BANK(026u), toballa},
    {TOBALLA + 3u, BANK(026u), toballa_terminate},
    {TOBALLA + 4u, BANK(026u), toballa_proceed},
    {TOBALLA + 5u, BANK(026u), toballa_enter},
    {TOBALLA + 6u, BANK(026u), toballa_immediate_return},
    {TOBALLA + 7u, BANK(026u), toballa_after_chklinus},
    {TOBALLC, BANK(026u), toballc},
    {TOBALLC + 2u, BANK(026u), toballc_after_ballangs},
    {TOBALLC + 3u, BANK(026u), toballc_after_g_n_auto},
    {AUTOMANV + 3u, BANK(026u), automanv_after_godspr},
    {STARTMNV, BANK(026u), startmnv},
    {ENDMANUV, BANK(026u), endmanuv},
    {ENDMANU1 + 2u, BANK(026u), endmanu1_after_downflag},
    {CHKLINUS + 013u, BANK(026u), chklinus_after_phaschng},
    {CHKLINUS + 015u, BANK(026u), chklinus_after_linus},
    {RELINUS, BANK(026u), relinus},
    {RELINUS + 2u, BANK(026u), relinus_after_priochng},
    {RELINUS + 012u, BANK(026u), relinus_not_p20},
    {RELINUS + 014u, BANK(026u), relinus_after_p25_phaschng},
    {RELINUS + 017u, BANK(026u), relinus_after_p20_phaschng},
    {RER60 + 2u, BANK(026u), rer60_after_upflag},
    {G_N_AUTO, BANK(026u), g_n_auto},
    {ISITAUTO, BANK(026u), isitauto},
    {BALLANGS, BANK(026u), ballangs},
    {BALLANGS + 1u, BANK(026u), ballangs_after_makecadr},
    {ENDBALL, BANK(026u), endball},
#else
    {R60CSM, BANK(027u), r60csm},
    {R60CSM + 1u, BANK(027u), r60csm_after_makecadr},
    {TOBALL, BANK(027u), toball},
    {TOBALL + 3u, BANK(027u), toball_terminate},
    {TOBALL + 4u, BANK(027u), toball_proceed},
    {TOBALL + 5u, BANK(027u), toball_enter},
    {TOBALL + 6u, BANK(027u), toball_immediate_return},
    {TOBALL + 7u, BANK(027u), toball_after_chklinus},
    {TOBALLC, BANK(027u), toballc},
    {TOBALLC + 012u, BANK(027u), toballc_after_godspr},
    {STARTMNV, BANK(027u), startmnv},
    {ENDMANUV, BANK(027u), endmanuv},
    {ENDMANU1 + 2u, BANK(027u), endmanu1_after_downflag},
    {CHKLINUS + 013u, BANK(027u), chklinus_after_phaschng},
    {CHKLINUS + 015u, BANK(027u), chklinus_after_linus},
    {RELINUS, BANK(027u), relinus},
    {RELINUS + 6u, BANK(027u), relinus_after_upflag},
    {RELINUS + 010u, BANK(027u), relinus_after_targ1flg},
    {RELINUS + 014u, BANK(027u), relinus_after_priochng},
    {GOREDO20 + 2u, BANK(027u), goredo20_after_phaschng},
    {R62DISP, BANK(027u), r62disp},
    {R62DISP + 3u, BANK(027u), r62disp_terminate},
    {R62DISP + 4u, BANK(027u), r62disp_proceed},
    {R62DISP + 5u, BANK(027u), r62disp_enter},
    {GOMOVE + 2u, BANK(027u), gomove_after_upflag},
    {GOMOVE + 4u, BANK(027u), gomove_after_bankcall},
#endif
};

static enum exec_status basic(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    channel_io *io = device, kept;
    apollo_overlay_state before;
    unsigned i;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == Z && basic_entries[i].bank == FBANK) break;
    if (i == COUNT(basic_entries) || !sane()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    basic_entries[i].routine();
    return EXEC_OK;
}

/* ---- The order pairs ---- */

/* The table LOC is in, under FB, or none. */
static const struct interpretive_code *code_at(void)
{
    static const struct interpretive_code *const codes[] = {
        &REDOMANN_CODE, &REDOMANC_CODE, &VECPOINT_CODE,
#if LUMINARY099
        &BALLANGS_CODE, &ROT180_CODE,
#endif
    };
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (FBANK == BANK(codes[i]->bank) && LOC >= codes[i]->first &&
            LOC < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* One pass; after RTB V1STO2S, V1STO2S and its DANZIG. */
static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             channel_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, &io->io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != V1STO2S || FBANK != V1STO2S_BANK) return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    v1sto2s();
    return interpret_listing(program, state, &io->io, code);
}

/* An order pair of the key's bank (bank): the faithful routine of each key
 * takes only its own bank's FB, though the LM's tables in banks 26 and 27
 * cover some of the same addresses. */
static enum exec_status interpretive(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device, word bank)
{
    channel_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at();
    if (Z != NEWOPS || FBANK != bank || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = pair(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

#if LUMINARY099
static enum exec_status interpretive_26(const agc_native_program *program,
                                        apollo_overlay_state *state, void *device)
{
    return interpretive(program, state, device, BANK(026u));
}
#endif

static enum exec_status interpretive_27(const agc_native_program *program,
                                        apollo_overlay_state *state, void *device)
{
    return interpretive(program, state, device, BANK(027u));
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each where an order pair starts that the faithful port enters. */
#define PAIR(bank, z)  {KEY(0##bank##u, z), interpretive_##bank}
#define BASIC(bank, z) {KEY(bank, z), basic}
#if LUMINARY099
static const agc_routine_entry pairs[] = {
    /* REDOMANN +5, REDOMANC +5 */
    PAIR(26, K_REDOMANN + 5u),
    PAIR(26, K_REDOMANN + 7u),
    PAIR(26, K_REDOMANN + 010u),
    PAIR(26, K_REDOMANC + 5u),
    PAIR(26, K_REDOMANC + 7u),
    PAIR(26, K_REDOMANC + 010u),
    /* BALLANGS +011 */
    PAIR(26, K_BALLANGS + 011u),
    PAIR(26, K_BALLANGS + 014u),
    PAIR(26, K_BALLANGS + 017u),
    PAIR(26, K_BALLANGS + 020u),
    PAIR(26, K_BALLANGS + 022u),
    PAIR(26, K_BALLANGS + 024u),
    PAIR(26, K_BALLANGS + 026u),
    PAIR(26, K_BALLANGS + 030u),
    PAIR(26, K_BALLANGS + 033u),
    PAIR(26, K_BALLANGS + 034u),
    PAIR(26, K_BALLANGS + 036u),
    PAIR(26, K_BALLANGS + 041u),
    PAIR(26, K_BALLANGS + 043u),
    PAIR(26, K_BALLANGS + 046u),
    PAIR(26, K_BALLANGS + 050u),
    PAIR(26, K_BALLANGS + 053u),
    PAIR(26, K_BALLANGS + 054u),
    PAIR(26, K_BALLANGS + 056u),
    PAIR(26, K_BALLANGS + 057u),
    /* ROT180 */
    PAIR(26, IN_BANK(I_ROT180)),
    PAIR(26, IN_BANK(I_ROT180) + 3u),
    PAIR(26, IN_BANK(I_ROT180) + 5u),
    PAIR(26, IN_BANK(I_ROT180) + 7u),
    PAIR(26, IN_BANK(I_ROT180) + 011u),
    PAIR(26, IN_BANK(I_ROT180) + 014u),
    PAIR(26, IN_BANK(I_ROT180) + 016u),
    PAIR(26, IN_BANK(I_ROT180) + 020u),
    PAIR(26, IN_BANK(I_ROT180) + 022u),
    /* VECPNT1, VECPOINT */
    PAIR(27, IN_BANK(I_VECPNT1)),
    PAIR(27, IN_BANK(I_VECPNT1) + 3u),
    PAIR(27, IN_BANK(I_VECPNT1) + 6u),
    PAIR(27, IN_BANK(I_VECPNT1) + 011u),
    PAIR(27, IN_BANK(I_VECPNT1) + 014u),
    PAIR(27, IN_BANK(I_VECPNT1) + 016u),
    PAIR(27, IN_BANK(I_VECPNT1) + 021u),
    PAIR(27, IN_BANK(I_VECPNT1) + 022u),
    PAIR(27, IN_BANK(I_VECPNT1) + 023u),
    PAIR(27, IN_BANK(I_VECPNT1) + 025u),
    PAIR(27, IN_BANK(I_VECPNT1) + 027u),
    PAIR(27, IN_BANK(I_VECPNT1) + 031u),
    PAIR(27, IN_BANK(I_VECPNT1) + 034u),
    PAIR(27, IN_BANK(I_VECPNT1) + 037u),
    PAIR(27, IN_BANK(I_VECPNT1) + 040u),
    PAIR(27, IN_BANK(I_VECPNT1) + 042u),
    PAIR(27, IN_BANK(I_VECPNT1) + 045u),
    PAIR(27, IN_BANK(I_VECPNT1) + 047u),
    PAIR(27, IN_BANK(I_VECPNT1) + 051u),
    PAIR(27, IN_BANK(I_VECPNT1) + 054u),
    PAIR(27, IN_BANK(I_VECPNT1) + 056u),
    PAIR(27, IN_BANK(I_VECPNT1) + 061u),
    PAIR(27, IN_BANK(I_VECPNT1) + 062u),
    PAIR(27, IN_BANK(I_VECPNT1) + 064u),
    PAIR(27, IN_BANK(I_VECPNT1) + 066u),
    PAIR(27, IN_BANK(I_VECPNT1) + 071u),
    PAIR(27, IN_BANK(I_VECPNT1) + 073u),
    PAIR(27, IN_BANK(I_VECPNT1) + 076u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0100u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0102u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0104u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0105u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0107u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0112u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0115u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0117u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0121u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0124u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0126u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0131u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0134u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0136u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0141u),
    PAIR(27, IN_BANK(I_VECPNT1) + 0144u),
};

static const agc_routine_entry basic_keys[] = {
    BASIC(023u, K_R62FLASH),       BASIC(023u, K_R62FLASH + 3u),   BASIC(023u, K_R62FLASH + 4u),
    BASIC(023u, K_R62FLASH + 5u),  BASIC(023u, K_GOMOVE + 2u),     BASIC(023u, K_GOMOVE + 4u),
    BASIC(026u, K_R60LEM),         BASIC(026u, K_R60LEM + 1u),     BASIC(026u, K_TOBALL),
    BASIC(026u, K_TOBALLA),        BASIC(026u, K_TOBALLA + 3u),    BASIC(026u, K_TOBALLA + 4u),
    BASIC(026u, K_TOBALLA + 5u),   BASIC(026u, K_TOBALLA + 6u),    BASIC(026u, K_TOBALLA + 7u),
    BASIC(026u, K_TOBALLC),        BASIC(026u, K_TOBALLC + 2u),    BASIC(026u, K_TOBALLC + 3u),
    BASIC(026u, K_AUTOMANV + 3u),  BASIC(026u, K_STARTMNV),        BASIC(026u, K_ENDMANUV),
    BASIC(026u, K_ENDMANU1 + 2u),  BASIC(026u, K_CHKLINUS + 013u), BASIC(026u, K_CHKLINUS + 015u),
    BASIC(026u, K_RELINUS),        BASIC(026u, K_RELINUS + 2u),    BASIC(026u, K_RELINUS + 012u),
    BASIC(026u, K_RELINUS + 014u), BASIC(026u, K_RELINUS + 017u),  BASIC(026u, K_RER60 + 2u),
    BASIC(026u, K_GPN_AUTO),       BASIC(026u, K_ISITAUTO),        BASIC(026u, K_BALLANGS),
    BASIC(026u, K_BALLANGS + 1u),  BASIC(026u, K_ENDBALL),
};

const agc_routine_table lm_r60_r62_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_r60_r62_basic = {basic_keys, COUNT(basic_keys)};
#else
static const agc_routine_entry pairs[] = {
    /* REDOMANN +5, REDOMANC +5 */
    PAIR(27, K_REDOMANN + 5u),
    PAIR(27, K_REDOMANN + 7u),
    PAIR(27, K_REDOMANN + 010u),
    PAIR(27, K_REDOMANC + 5u),
    PAIR(27, K_REDOMANC + 7u),
    PAIR(27, K_REDOMANC + 010u),
    /* VECPOINT, ROT180 */
    PAIR(27, IN_BANK(I_VECPOINT)),
    PAIR(27, IN_BANK(I_VECPOINT) + 3u),
    PAIR(27, IN_BANK(I_VECPOINT) + 6u),
    PAIR(27, IN_BANK(I_VECPOINT) + 010u),
    PAIR(27, IN_BANK(I_VECPOINT) + 013u),
    PAIR(27, IN_BANK(I_VECPOINT) + 014u),
    PAIR(27, IN_BANK(I_VECPOINT) + 015u),
    PAIR(27, IN_BANK(I_VECPOINT) + 017u),
    PAIR(27, IN_BANK(I_VECPOINT) + 021u),
    PAIR(27, IN_BANK(I_VECPOINT) + 023u),
    PAIR(27, IN_BANK(I_VECPOINT) + 026u),
    PAIR(27, IN_BANK(I_VECPOINT) + 031u),
    PAIR(27, IN_BANK(I_VECPOINT) + 032u),
    PAIR(27, IN_BANK(I_VECPOINT) + 034u),
    PAIR(27, IN_BANK(I_VECPOINT) + 037u),
    PAIR(27, IN_BANK(I_VECPOINT) + 041u),
    PAIR(27, IN_BANK(I_VECPOINT) + 043u),
    PAIR(27, IN_BANK(I_VECPOINT) + 046u),
    PAIR(27, IN_BANK(I_VECPOINT) + 050u),
    PAIR(27, IN_BANK(I_VECPOINT) + 053u),
    PAIR(27, IN_BANK(I_VECPOINT) + 054u),
    PAIR(27, IN_BANK(I_VECPOINT) + 056u),
    PAIR(27, IN_BANK(I_VECPOINT) + 060u),
    PAIR(27, IN_BANK(I_VECPOINT) + 063u),
    PAIR(27, IN_BANK(I_VECPOINT) + 065u),
    PAIR(27, IN_BANK(I_VECPOINT) + 070u),
    PAIR(27, IN_BANK(I_VECPOINT) + 072u),
    PAIR(27, IN_BANK(I_VECPOINT) + 074u),
    PAIR(27, IN_BANK(I_VECPOINT) + 076u),
    PAIR(27, IN_BANK(I_VECPOINT) + 077u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0101u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0104u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0107u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0111u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0113u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0116u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0120u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0123u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0126u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0130u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0133u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0136u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0140u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0143u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0145u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0147u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0151u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0154u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0156u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0160u),
    PAIR(27, IN_BANK(I_VECPOINT) + 0162u),
};

static const agc_routine_entry basic_keys[] = {
    BASIC(027u, K_R60CSM),         BASIC(027u, K_R60CSM + 1u),     BASIC(027u, K_TOBALL),
    BASIC(027u, K_TOBALL + 3u),    BASIC(027u, K_TOBALL + 4u),     BASIC(027u, K_TOBALL + 5u),
    BASIC(027u, K_TOBALL + 6u),    BASIC(027u, K_TOBALL + 7u),     BASIC(027u, K_TOBALLC),
    BASIC(027u, K_TOBALLC + 012u), BASIC(027u, K_STARTMNV),        BASIC(027u, K_ENDMANUV),
    BASIC(027u, K_ENDMANU1 + 2u),  BASIC(027u, K_CHKLINUS + 013u), BASIC(027u, K_CHKLINUS + 015u),
    BASIC(027u, K_RELINUS),        BASIC(027u, K_RELINUS + 6u),    BASIC(027u, K_RELINUS + 010u),
    BASIC(027u, K_RELINUS + 014u), BASIC(027u, K_GOREDO20 + 2u),   BASIC(027u, K_R62DISP),
    BASIC(027u, K_R62DISP + 3u),   BASIC(027u, K_R62DISP + 4u),    BASIC(027u, K_R62DISP + 5u),
    BASIC(027u, K_GOMOVE + 2u),    BASIC(027u, K_GOMOVE + 4u),
};

const agc_routine_table cm_r60_r62_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_r60_r62_basic = {basic_keys, COUNT(basic_keys)};
#endif
