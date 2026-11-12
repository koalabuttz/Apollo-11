/* SPDX-License-Identifier: MIT */
/* The readable display interface routines (src/display_interface/) as
 * routines of the flight program. The faithful port keeps
 * DISPLAY_INTERFACE_ROUTINES in six modules (c64-port/src/lanec_di_<part>.c),
 * which its routine tables (lanec_di_routines.c) call for every key of the
 * part, with the state, a record of the channels (lanec_io,
 * c64-port/include/lanec.h) and the mission. The readable code takes the
 * tables' places under their names (at the end of this file), and the
 * modules', which the faithful port's own tests call: the display starts
 * and the R displays
 * are lanec_di_dsp_dispatch; the mark displays lanec_di_mark_dispatch;
 * MAKEPLAY, WITCHONE, XCHTOEND and LINUSCHR lanec_di_play_dispatch; the
 * sleeping and waking displays and INITDSP lanec_di_sleep_dispatch; ENDIDLE
 * and the returns to the user lanec_di_idle_dispatch; NVDSP and its
 * branches lanec_di_nvdsp_dispatch.
 *
 * Plumbing only: point the machine at the state and the channel record
 * (channel 11, DSALMOUT, which WITCHONE's WAND clears the KEY REL light
 * in), and take the states the faithful module takes: the mission this
 * object is built for, the bank registers agreeing, Z a 12-bit address and
 * ZERO +0, bank 10 selected (any bank for the fixed-fixed BLANKET, ENDMARK
 * and CLEARMRK). The rope is read for the tables the code indexes (PRIOOCT,
 * WAKECADR, BITS5+11 and the rest). Each key runs one routine, to the
 * point where control leaves the module's words, as the faithful module
 * does. A step the readable code does not model leaves the state as it was.
 *
 * Refused besides, as the faithful modules refuse them (states the
 * program does not produce, where an index or a TC Q would leave the
 * words the module holds):
 *   - CLEARMRK, CLEARMRK +2, BLANKET, COPYTOGO and COPYPACS, and WITCHONE,
 *     XCHTOEND, XCHNYLOC and LINUSCHR, with Q not a return into fixed
 *     memory outside the code of this file;
 *   - COPYNORM, COPIES and COPIES2 with an index above 0300, or Q not in
 *     fixed-fixed; SAVELOCS +7 with L not one of the returns SAVELOCR
 *     saves (or BOTHJOBS +1 with LOCCTR past the last job's words);
 *   - MAKEPLAY +5, MAKEPRIO, IFLEGAL and LINUSCHR with DSPLOCK negative;
 *     OKTOPLAY and LINUSCHR with COPINDEX above 0300.
 * The checks of COPINDEX, FACEREG, TEMPOR2, COPMPAC and LOCCTR on the way
 * are in the code (display_kind(), sleeper(), not_modelled()). */
#include <setjmp.h>
#include <stddef.h>
#include "lanec_di_dsp.h"
#include "lanec_di_idle.h"
#include "lanec_di_mark.h"
#include "lanec_di_nvdsp.h"
#include "lanec_di_play.h"
#include "lanec_di_sleep.h"
#include "binding.h"
#include "rope_image.h"
#include "../display_interface/display_interface.h"
#include "../agc/names.h"

#if LUMINARY099
#define THIS_MISSION LANEC_LM
#define ROPE         0u
#else
#define THIS_MISSION LANEC_CM
#define ROPE         1u
#endif

/* The superbank bit (channel 7's bit 7): FBANK's banks 30-33 are then
 * banks 40-43. */
#define SUPERBANK_BIT 0100u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* No channel but DSALMOUT (agc.dsalmout) is read or written. */
static word channel_in(unsigned number)
{
    (void)number;
    not_modelled();
    return 0;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled();
}

static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind_lanec(apollo_overlay_state *state, lanec_io *io)
{
    agc.memory = state->words;
    agc.superbank = &io->superbank;
    agc.interrupts = &io->interrupt_enabled;
    agc.dsalmout = &io->channel[011];
    agc.in_interrupt = NULL;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The faithful module's states (lanec_registers_valid). */
static bool as_expected(void)
{
    return (EBANK & ~03400u) == 0u && (FBANK & ~076000u) == 0u &&
           BBANK == (word)(FBANK | (EBANK >> 8)) && Z <= 07777u && agc.memory[ZERO_ADDRESS] == 0u;
}

/* The fixed bank selected: FBANK's, or with the superbank bit 40-43 for
 * 30-33. */
static word bank_selected(void)
{
    word bank = (word)((FBANK >> 10) & 037u);
    return bank >= 030u && (*agc.superbank & SUPERBANK_BIT) ? (word)(bank + 010u) : bank;
}

/* ---- The states the faithful modules take ---- */

/* A TC Q back to a mark routine's caller: a TC into fixed memory outside
 * BLANKET..CLEARMRK +7 and, in bank 10, outside MARKEND..PRIODSP +2,
 * EXDSPRET..SKIPADD +3 and RELINTQ's two words. */
static bool mark_caller(void)
{
    if (Q < 02000u || Q > 07777u) return false;
    if (Q >= 04000u) return Q < BLANKET || Q > CLEARMRK + 7u;
    if (bank_selected() != 010u) return true;
    return (Q < MARKEND || Q > PRIODSP + 2u) && (Q < EXDSPRET || Q > SKIPADD + 3u) &&
           (Q < RELINTQ || Q > RELINTQ + 1u);
}

/* WITCHONE, XCHTOEND, XCHNYLOC and LINUSCHR's caller: Q a TC into fixed
 * memory outside LINUSCHR's words and, in bank 10, outside
 * MARKEND..RELINTQ +1. */
static bool play_caller(void)
{
    if (Q < 02000u || Q > 07777u) return false;
    if (Q >= 04000u || bank_selected() != 010u) return true;
    if (Q >= LINUSCHR && Q <= LINUSCHR + 014u) return false;
    return Q < MARKEND || Q >= RELINTQ + 2u;
}

static bool copies_return(word index)
{
    return index <= 0300u && Q >= 04000u && Q <= 07777u;
}

static bool copynorm_index(void)
{
    return copies_return(0u);
}

static bool copies_index(void)
{
    return copies_return(overflow_corrected(A));
}

static bool copies2_index(void)
{
    return copies_return(fetch(ADDRESS_OF(COPINDEX)));
}

/* MAKECADR's return through L (SAVELOCR's LXCH Q): GOFLASH2 +2, BOTHJOBS
 * +014, or BOTHJOBS +1 with the job's words in unswitched erasable. */
static bool saved_return(void)
{
    return L == GOFLASH2 + 2u || L == BOTHJOBS + 014u ||
           (L == BOTHJOBS + 1u && LOCCTR <= 01376u - (ADDRESS_OF(MPAC) + 4u));
}

static bool dsplock_positive(void)
{
    return fetch(ADDRESS_OF(DSPLOCK)) < 040000u;
}

static bool copindex_in_tables(void)
{
    return fetch(ADDRESS_OF(COPINDEX)) <= 0300u;
}

static bool linus_caller(void)
{
    return play_caller() && dsplock_positive() && copindex_in_tables();
}

/* ---- Running a key ---- */

struct entry {
    word at;
    void (*routine)(void);
    bool (*in_domain)(void);
};

/* The entry at Z, or NULL. */
static const struct entry *entry_at_z(const struct entry *entries, size_t count)
{
    size_t i;

    for (i = 0; i < count; ++i)
        if (entries[i].at == Z) return &entries[i];
    return NULL;
}

/* Run the routine at Z, if it is one of the part's entries: in bank 10,
 * or for the fixed-fixed ones in any bank. */
static enum lanec_status run(const struct entry *entries, size_t count, apollo_overlay_state *state,
                             lanec_io *io, enum lanec_mission mission)
{
    apollo_overlay_state before;
    lanec_io io_before;
    const struct entry *entry;

    if (mission != THIS_MISSION || !state || !io) return LANEC_OUT_OF_SCOPE;
    bind_lanec(state, io);
    if (!as_expected()) return LANEC_OUT_OF_SCOPE;
    if (Z < 04000u && bank_selected() != 010u) return LANEC_OUT_OF_SCOPE;
    entry = entry_at_z(entries, count);
    if (!entry || (entry->in_domain && !entry->in_domain())) return LANEC_OUT_OF_SCOPE;
    before = *state;
    io_before = *io;
    if (setjmp(back)) {
        *state = before;
        *io = io_before;
        return LANEC_OUT_OF_SCOPE;
    }
    entry->routine();
    return LANEC_OK;
}

#define RUN(entries) run(entries, sizeof entries / sizeof entries[0], state, io, mission)

/* ---- The display starts and the R displays ---- */

static const struct entry dsp[] = {
    {CHKPRIO, chkprio, NULL},
    {MARKPERF, markperf, NULL},
    {GODSP, godsp, NULL},
    {GODSP2, godsp2, NULL},
    {GODSPRET, godspret, NULL},
    {GODSPR, godspr, NULL},
    {GODSPR1, godspr1, NULL},
    {GODSPR2, godspr2, NULL},
    {CLEANDSP, cleandsp, NULL},
    {REFLASH, reflash, NULL},
#if LUMINARY099
    {REFLASHR, reflashr, NULL},
#endif
    {REGODSP, regodsp, NULL},
    {REGODSPR, regodspr, NULL},
    {CLOCPLAY, clocplay, NULL},
    {GOFLASH, goflash, NULL},
    {GOFLASH2, goflash2, NULL},
    {PRIODSPR, priodspr, NULL},
    {PRIODSP, priodsp, NULL},
    {SETPRIO, setprio, NULL},
    {EXDSPRET, exdspret, NULL},
    {GOPERF1, goperf1, NULL},
    {GOPERFS, goperfs, NULL},
    {GOPERF2, goperf2, NULL},
    {GOPERF4, goperf4, NULL},
    {GOFLASHR, goflashr, NULL},
    {GODSPRS, godsprs, NULL},
    {GODSPRS1, godsprs1, NULL},
    {GODSPRS1 + 015u, godsprs1_after_novac, NULL},
    {BOTHJOBS, bothjobs, NULL},
    {GOPERF1R, goperf1r, NULL},
    {GOPERFRS, goperfrs, NULL},
    {GOPERF2R, goperf2r, NULL},
    {GOPERF4R, goperf4r, NULL},
    {SAVELOCS + 7u, savelocs_after_makecadr, saved_return},
    {COPYNORM, copynorm, copynorm_index},
    {COPIES, copies, copies_index},
    {COPIES2, copies2, copies2_index},
};

enum lanec_status lanec_di_dsp_dispatch(apollo_overlay_state *state, lanec_io *io,
                                        enum lanec_mission mission)
{
    return RUN(dsp);
}

/* ---- The mark displays ---- */

static const struct entry mark[] = {
    {BLANKET, blanket, mark_caller},
    {ENDMARK, endmark, NULL},
    {CLEARMRK, clearmrk, mark_caller},
    {CLEARMRK + 2u, clearmrk_plus2, mark_caller},
    {MARKEND, markend, NULL},
    {GOMARK, gomark, NULL},
    {GOMARS, gomars, NULL},
    {KLEENEX, kleenex, NULL},
    {GOMARKF, gomarkf, NULL},
    {GOMARK2, gomark2, NULL},
    {MARKFORM, markform, NULL},
    {GOMARK3, gomark3, NULL},
    {GOMARK4, gomark4, NULL},
    {GOMARKR, gomarkr, NULL},
    {GOMARKFR, gomarkfr, NULL},
    {GOMARK2R, gomark2r, NULL},
    {GOMARK3R, gomark3r, NULL},
    {MAKEMARK, makemark, NULL},
    {MARKPLAY, markplay, NULL},
    {GOGOMARK, gogomark, NULL},
    {MARKCOP, markcop, NULL},
    {COPYTOGO, copytogo, mark_caller},
    {COPYPACS, copypacs, mark_caller},
};

enum lanec_status lanec_di_mark_dispatch(apollo_overlay_state *state, lanec_io *io,
                                         enum lanec_mission mission)
{
    return RUN(mark);
}

/* ---- MAKEPLAY, and the services the display jobs share ---- */

static const struct entry play[] = {
    {MAKEPRIO, makeprio, dsplock_positive},
    {MAKEPRIO + 4u, makeprio_after_linuschr, NULL},
    {HIPRIO, hiprio, NULL},
    {SETMARK, setmark, NULL},
    {ASKIFNRM, askifnrm, NULL},
    {SETNORM, setnorm, NULL},
    {OKTOCOPY, oktocopy, NULL},
    {OKTOCOPY + 3u, oktocopy_after_jobwake, NULL},
    {REDOPRIO, redoprio, NULL},
    {KEEPPRIO, keepprio, NULL},
    {MAKEPLAY, makeplay, NULL},
    {MAKEPLAY + 5u, makeplay_after_priochng, dsplock_positive},
    {IFLEGAL, iflegal, dsplock_positive},
    {IFLEGAL + 4u, iflegal_after_linuschr, NULL},
    {PRIOBORT, priobort, NULL},
    {OKTOPLAY, oktoplay, copindex_in_tables},
    {OKTOPLAY + 015u, oktoplay_after_jobwake, NULL},
    {PLAYJUM1, playjum1, NULL},
    {PRIOPLAY, prioplay, NULL},
    {WITCHONE, witchone, play_caller},
    {XCHTOEND, xchtoend, play_caller},
    {XCHNYLOC, xchnyloc, play_caller},
    {LINUSCHR, linuschr, linus_caller},
};

enum lanec_status lanec_di_play_dispatch(apollo_overlay_state *state, lanec_io *io,
                                         enum lanec_mission mission)
{
    return RUN(play);
}

/* ---- The display jobs put to sleep and woken ---- */

static const struct entry sleep[] = {
    {GOSLEEPS, gosleeps, NULL},
    {GOSLEEPS + 5u, gosleeps_after_upent2, NULL},
    {XCHSLEEP, xchsleep, NULL},
    {XCHSLEEP + 4u, xchsleep_after_jobwake, NULL},
    {XCHSLEEP + 5u, xchsleep_after_xchtoend, NULL},
    {JOBXCHS, jobxchs, NULL},
    {JOBXCHS + 3u, jobxchs_after_jobwake, NULL},
    {JOBXCHS + 010u, jobxchs_after_xchnyloc, NULL},
    {JOBXCHS + 015u, idlemask_after_downent2, NULL},
    {JOBXCHS + 022u, xchqadd_after_upent2, NULL},
    {MARKWAKE, markwake, NULL},
    {WAKEPLAY, wakeplay, NULL},
    {WAKEPLAY + 6u, markfmsk_after_downent2, NULL},
    {WAKEPLAY + 012u, markfmsk_after_jobwake, NULL},
    {INITDSP, initdsp, NULL},
    {INITDSP + 4u, initdsp_after_supersw, NULL},
    {INITDSP + 6u, initdsp_after_priochng, NULL},
};

enum lanec_status lanec_di_sleep_dispatch(apollo_overlay_state *state, lanec_io *io,
                                          enum lanec_mission mission)
{
    return RUN(sleep);
}

/* ---- ENDIDLE, and the returns to the user ---- */

static const struct entry idle[] = {
    {CLEANEND, cleanend, NULL},
    {CLEANEND + 4u, cleanend_after_findvac, NULL},
    {ISITPRIO, isitprio, NULL},
    {REST, rest, NULL},
    {RESTSLEP, restslep, NULL},
    {RESTSLEP + 4u, restslep_after_upent2, NULL},
    {FLASHSUB, flashsub, NULL},
    {FLASHSUB + 1u, flashsub_plus1, NULL},
    {FLASHSUB + 7u, itismask_after_upent2, NULL},
    {IDLERET1 - 1u, goidle, NULL},
    {IDLERET1, idleret1, NULL},
    {IDLERET1 + 1u, idleret1_plus1, NULL},
    {IDLERET1 + 2u, idleret1_plus2, NULL},
    {OKTOENT, oktoent, NULL},
    {ENDOUT, endout, NULL},
    {TIMECHEK, timechek, NULL},
    {NORMWAKE, normwake, NULL},
    {ENDRET, endret, NULL},
    {ENDRET + 4u, endret_plus4, NULL},
    {ENDRET + 013u, pinidmsk_after_downent2, NULL},
    {ENDRET + 015u, pinidmsk_after_nvsub, NULL},
    {ENDIT, endit, NULL},
    {ENDIT + 3u, endit_after_priochng, NULL},
    {UNSETR1, unsetr1, NULL},
    {IMMEDRET - 1u, unsetr1_after_supersw, NULL},
    {IMMEDRET, immedret, NULL},
    {TERMATE, termate, NULL},
    {PROCEED, proceed, NULL},
    {MARKRET, markret, NULL},
    {MARKOVER, markover, NULL},
    {NORMRET, normret, NULL},
    {NORMRET + 4u, normret_plus4, NULL},
    {NORMRET + 010u, normret_plus010, NULL},
    {NORMRET + 014u, normret_plus014, NULL},
    {NORMRET + 024u, normret_after_novac, NULL},
    {MARSLEEP, marsleep, NULL},
    {LOADITIS, loaditis, NULL},
    {ERASER, eraser, NULL},
    {ERASER + 2u, eraser_after_nvsub, NULL},
    {ERASER + 3u, eraser_after_nvsub, NULL},
};

enum lanec_status lanec_di_idle_dispatch(apollo_overlay_state *state, lanec_io *io,
                                         enum lanec_mission mission)
{
    return RUN(idle);
}

/* ---- NVDSP: putting the display up ---- */

static const struct entry nvdsp_entries[] = {
    {PINBRNCH, pinbrnch, NULL},
    {NORMBNCH, normbnch, NULL},
    {NORMBNCH + 2u, normbnch_after_upflag, NULL},
    {NVDSP, nvdsp, NULL},
    {NVDSP + 1u, nvdsp_after_copypacs, NULL},
    {NVDSP + 022u, nvdsp_mark, NULL},
    {NVDSP + 023u, nvdsp_plus023, NULL},
    {NVDSP1, nvdsp1, NULL},
    {NV50DSP, nv50dsp, NULL},
    {NV50DSP + 1u, nv50dsp_busy, NULL},
    {NV50DSP + 2u, nv50dsp_after_nvmonopt, NULL},
    {NV50DSP + 3u, nv50dsp_after_flashoff, NULL},
    {NV50DSP + 4u, nv50dsp_after_copytogo, NULL},
    {NV50DSP + 6u, nv50dsp_after_downflag, NULL},
    {NV50DSP + 010u, nv50dsp_after_downflag2, NULL},
    {BLANKCHK, blankchk, NULL},
    {BLANKCHK + 2u, blankchk_after_blanksub, NULL},
    {PERFCHEK, perfchek, NULL},
    {GOANIDLE, goanidle, NULL},
    {GOANIDLE + 4u, goanidle_plus4, NULL},
    {GOANIDLE + 010u, goanidle_plus010, NULL},
    {ISITN00, isitn00, NULL},
    {ADDRESS_OF(1STOR2ND), first_or_second, NULL},
    {ADDRESS_OF(1STOR2ND) + 4u, first_or_second_plus4, NULL},
    {V50PASTE, v50paste, NULL},
};

enum lanec_status lanec_di_nvdsp_dispatch(apollo_overlay_state *state, lanec_io *io,
                                          enum lanec_mission mission)
{
    return RUN(nvdsp_entries);
}

/* ---- The tables ----
 *
 * The flight program enters the file through routine tables of the
 * faithful port's names, with its keys (display_interface_tables.inc,
 * which tools/lanec_keys.py writes). A key's routine is its part, above,
 * run on the program's own record: the superbank and the interrupt switch
 * of the mission's record, and channel 11, which the LM program keeps in
 * its hardware record and the CM in the mission's. A part that refuses
 * the state leaves the record as it was. */
typedef enum lanec_status part_dispatch(apollo_overlay_state *, lanec_io *, enum lanec_mission);

static enum exec_status run_part(const agc_native_program *program, apollo_overlay_state *state,
                                 void *device, part_dispatch *part)
{
    static const lanec_io none;
    static lanec_io io; /* (the machine keeps its address) */
    mission_io *record = device;
    agc_word *channel11;

    if (!program || !state || !record) return EXEC_OUT_OF_SCOPE;
#if LUMINARY099
    if (!program->hardware) return EXEC_OUT_OF_SCOPE;
    channel11 = &program->hardware->channel11;
#else
    channel11 = &record->channel11;
#endif
    io = none;
    io.superbank = record->superbank;
    io.interrupt_enabled = record->interrupt_enabled;
    io.channel[011] = *channel11;
    if (part(state, &io, THIS_MISSION) != LANEC_OK) return EXEC_OUT_OF_SCOPE;
    record->superbank = io.superbank;
    record->interrupt_enabled = io.interrupt_enabled;
    *channel11 = io.channel[011];
    return EXEC_OK;
}

#define PART(name)                                                                                 \
    static enum exec_status name##_part(const agc_native_program *program,                         \
                                        apollo_overlay_state *state, void *device)                 \
    {                                                                                              \
        return run_part(program, state, device, lanec_di_##name##_dispatch);                       \
    }
PART(dsp)
PART(mark)
PART(play)
PART(sleep)
PART(idle)
PART(nvdsp)

#if LUMINARY099
#include "display_interface_tables.inc"
#else
#include "cm_display_interface_tables.inc"
#endif
