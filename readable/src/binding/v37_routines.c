/* SPDX-License-Identifier: MIT */
/* The readable verb 37 and P00 (src/fresh_start/v37.c) as routines of the
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_v37.c, cm_v37.c): lm_v37_routines and cm_v37_routines,
 * with lm_v37_entries and cm_v37_entries.
 *
 * Plumbing only: point the machine at the program's state, give it the
 * channels of the io record (the CM's: 5, 6 and 11-13), and take the
 * states the faithful routines take. A routine that meets a step it does
 * not model (not_modelled(), a channel it has none for, or a fixed word:
 * it reads none) is not taken: the state is put back. */
#include "../agc/names.h"
#include "../fresh_start/v37.h"
#include "binding.h"
#include <setjmp.h>
#include <stddef.h>
#if LUMINARY099
#include "lm_v37.h"
typedef p66_bias_io v37_io;
#else
#include "cm_v37.h"
typedef cm_fresh_io v37_io;
#endif

#define ZERO_REGISTER 7u

/* Keys: bank 04, and the CM's bank 20 (VAC5STOR). */
#define B4(z)  (uint16_t)(4u * 1024u + ((z) - 02000u))
#define B20(z) (uint16_t)(020u * 1024u + ((z) - 02000u))

/* The places the routines come back to, after a TC out of the file: the
 * TC's word plus one, or two past a CADR or ADRES. */
/* GOFLASH's three returns (terminate, proceed, enter), past its CADR. */
#if LUMINARY099
#define AFTER_GOFLASH_1 (GOP00FIX + 010u)
#else
#define AFTER_GOFLASH_1 (GOP00FIX + 5u)
#endif
#define AFTER_GOFLASH_2 (AFTER_GOFLASH_1 + 1u)
#define AFTER_GOFLASH_3 (AFTER_GOFLASH_1 + 2u)
#define AFTER_RELDSP    (V37BAD + 1u)
#define AFTER_FALTON    (V37NONO + 1u)
#define AFTER_NEWMODEA  (V37XEQC + 3u)
#define AFTER_RELDSP_2  (V37XEQC + 4u)
#define AFTER_POOH      (POOH + 1u)
#define AFTER_IMUSE     (NOUVEAU + 6u)
#define AFTER_INTPRET   (DUMMYAD + 1u)
#if LUMINARY099
#define AFTER_XOVINFLG   (GOP00FIX + 2u)
#define AFTER_ULLAGFLG   (GOP00FIX + 4u)
#define AFTER_CLEARMRK   (GOP00FIX + 5u)
#define AFTER_AVEGFLAG   (ISSERVON + 6u)
#define AFTER_3AXISFLG   (R00 + 6u)
#define AFTER_R04FLAG    (R00 + 012u)
#define AFTER_MUNFLAG    (R00 + 014u)
#define AFTER_XOVINFLG_2 (R00 + 016u)
#define AFTER_CLRADMOD   (POOH + 4u)
#define AFTER_ENGINOF1   (SEUDOP00 + 4u)
#define AFTER_ALLCOAST   (SEUDOP00 + 6u)
#define AFTER_V37KLEAN   (SEUDOP00 + 014u)
#define AFTER_P00KLEAN   (GOMOD + 2u)
#else
#define AFTER_CLEARMRK   (GOP00FIX + 2u)
#define AFTER_SPSOFF     (R00TOP00 + 7u)
#define AFTER_MASSPROP   (R00TOP00 + 011u)
#define AFTER_RCSDAPON   (R00TOP00 + 014u)
#define AFTER_TVCZAP     (R00TOP00 + 016u)
#define AFTER_DELAYJOB   (R00TOP00 + 024u)
#define AFTER_CLEARMRK_2 (DUMMYAD + 014u)
#define AFTER_STIKFLAG   (DUMMYAD + 016u)
#define AFTER_UPACTOFF   (DUMMYAD + 020u)
#define AFTER_VHFRFLAG   (DUMMYAD + 022u)
#define AFTER_R21MARK    (DUMMYAD + 024u)
#define AFTER_V37KLEAN   (GROUPKIL + 2u)
#define AFTER_STOPRATE   (INITSUB + 6u)
#define AFTER_SETMINDB   (SETMAXER - 1u)
#define AFTER_UPFLAG     (RAKE + 015u)
#define AFTER_GENTRAN    (V5OUT1 + 6u)
#endif

static v37_io *io_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The routines read no fixed words: their tables are in v37.c. */
static word fixed_memory(unsigned physical)
{
    (void)physical;
    leave();
    return 0;
}

#if COMANCHE055
/* The CM's channels: 5 and 6 (the RCS jets), 11-13. */
static agc_word *channel(unsigned number)
{
    switch (number) {
    case 05u: return &io_now->channel5;
    case 06u: return &io_now->channel6;
    case 011u: return &io_now->control.channel11;
    case 012u: return &io_now->control.channel12;
    case 013u: return &io_now->control.channel13;
    default: return NULL;
    }
}

static word channel_in(unsigned number)
{
    agc_word *word_there = channel(number);
    if (!word_there) leave();
    return (word)*word_there;
}

/* As yaAGC writes a channel: channel 13's bits 12-14 set the HANDRUPT
 * traps and are not kept. */
static void channel_out(unsigned number, word value)
{
    agc_word *word_there = channel(number);
    if (!word_there) leave();
    if (number == 013u) {
        if (value & 004000u) io_now->control.trap31a = 1u;
        if (value & 010000u) io_now->control.trap31b = 1u;
        if (value & 020000u) io_now->control.trap32 = 1u;
        value &= 043777u;
    }
    *word_there = value;
}
#define CONTROL (&io_now->control)
#else
static word channel_in(unsigned number)
{
    (void)number;
    leave();
    return 0;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    leave();
}
#define CONTROL io_now
#endif

/* V37 starts with TS MMNUMBER: A comes from pinball as a word. */
static bool takes(word entry)
{
    if (agc.memory[ZERO_REGISTER] != 0u || (CONTROL->superbank & ~0160u) != 0u ||
        CONTROL->interrupt_enabled > 1u)
        return false;
    return entry != V37 || overflow_of(A) == 0;
}

static void run_bank04(word entry);
#if COMANCHE055
static void run_bank20(word entry);
#endif

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, unsigned bank)
{
    static const agc_native_program none;
    static apollo_overlay_state before;
    static v37_io io_before;
    word entry;

    if (!state || !device || !bind_machine(program ? program : &none, state, device))
        return EXEC_OUT_OF_SCOPE;
    io_now = device;
    agc.fixed_memory = fixed_memory;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.leave = leave;
    entry = Z;
    if (!takes(entry)) return EXEC_OUT_OF_SCOPE;

    before = *state;
    io_before = *io_now;
    if (setjmp(back)) {
        *state = before;
        *io_now = io_before;
        return EXEC_OUT_OF_SCOPE;
    }
#if COMANCHE055
    if (bank == 020u) {
        run_bank20(entry);
        return EXEC_OK;
    }
#else
    (void)bank;
#endif
    run_bank04(entry);
    return EXEC_OK;
}

static void run_bank04(word entry)
{
    switch (entry) {
    case GOP00FIX: gop00fix(); break;
    case AFTER_CLEARMRK: gop00fix_after_clearmrk(); break;
    case AFTER_GOFLASH_1:
    case AFTER_GOFLASH_2:
    case AFTER_GOFLASH_3: gop00fix_after_goflash(); break;
    case V37: v37(); break;
    case V37BAD: v37bad(); break;
    case AFTER_RELDSP: v37bad_after_reldsp(); break;
    case CANV37: canv37(); break;
    case AFTER_POOH: pooh_after_reldsp(); break;
    case AFTER_IMUSE: nouveau_after_downflag(); break;
    case AFTER_FALTON: v37nono_after_falton(); break;
    case V37XEQ: v37xeq(); break;
    case V37XEQC: v37xeqc(); break;
    case AFTER_NEWMODEA: v37xeqc_after_newmodea(); break;
    case AFTER_RELDSP_2: v37xeqc_after_reldsp(); break;
#if LUMINARY099
    case AFTER_XOVINFLG: gop00fix_after_downflag_xovinflg(); break;
    case AFTER_ULLAGFLG: gop00fix_after_downflag_ullagflg(); break;
    case AFTER_AVEGFLAG: isservon_after_downflag(); break;
    case V37RET: v37ret(); break;
    case R00: r00(); break;
    case AFTER_INTPRET: r00_after_intpret(); break;
    case AFTER_3AXISFLG: r00_after_downflag_3axisflg(); break;
    case AFTER_R04FLAG: r00_after_downflag_r04flag(); break;
    case AFTER_MUNFLAG: r00_after_downflag_munflag(); break;
    case AFTER_XOVINFLG_2: r00_after_downflag_xovinflg(); break;
    case AFTER_CLRADMOD: pooh_after_clradmod(); break;
    case AFTER_ENGINOF1: seudop00_after_enginof1(); break;
    case AFTER_ALLCOAST: seudop00_after_allcoast(); break;
    case AFTER_V37KLEAN: seudop00_after_v37klean(); break;
    case AFTER_P00KLEAN: gomod_after_p00klean(); break;
    case KILL2: kill2(); break;
#else
    case AFTER_SPSOFF: r00top00_after_spsoff(); break;
    case AFTER_MASSPROP: r00top00_after_massprop(); break;
    case AFTER_RCSDAPON: r00top00_after_rcsdapon(); break;
    case AFTER_TVCZAP: r00top00_after_tvczap(); break;
    case AFTER_DELAYJOB: r00top00_after_delayjob(); break;
    case ROC: roc(); break;
    case AFTER_INTPRET: roc_after_intpret(); break;
    case AFTER_CLEARMRK_2: roc_after_clearmrk(); break;
    case AFTER_STIKFLAG: roc_after_downflag_stikflag(); break;
    case AFTER_UPACTOFF: roc_after_upactoff(); break;
    case AFTER_VHFRFLAG: roc_after_downflag_vhfrflag(); break;
    case AFTER_R21MARK: roc_after_downflag_r21mark(); break;
    case AFTER_V37KLEAN: groupkil_after_v37klean(); break;
    case GOMOD: gomod(); break;
    case AFTER_STOPRATE: initsub_after_stoprate(); break;
    case AFTER_SETMINDB: initsub_after_setmindb(); break;
    case RAKE: rake(); break;
    case AFTER_UPFLAG: rake_after_upflag(); break;
#endif
    default: leave();
    }
}

#if COMANCHE055
static void run_bank20(word entry)
{
    switch (entry) {
    case VAC5STOR: vac5stor(); break;
    case AFTER_GENTRAN: vac5stor_after_gentran(); break;
    default: leave();
    }
}

static enum exec_status run20(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    return run(program, state, device, 020u);
}
#endif

static enum exec_status run04(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    return run(program, state, device, 04u);
}

/* In the faithful table's order (by key). */
#if LUMINARY099
static const agc_routine_entry routines[] = {
    {B4(GOP00FIX), run04},         {B4(AFTER_XOVINFLG), run04},
    {B4(AFTER_ULLAGFLG), run04},   {B4(AFTER_CLEARMRK), run04},
    {B4(AFTER_GOFLASH_1), run04},  {B4(AFTER_GOFLASH_2), run04},
    {B4(AFTER_GOFLASH_3), run04},  {B4(V37), run04},
    {B4(V37BAD), run04},           {B4(AFTER_RELDSP), run04},
    {B4(AFTER_AVEGFLAG), run04},   {B4(V37RET), run04},
    {B4(CANV37), run04},           {B4(R00), run04},
    {B4(AFTER_INTPRET), run04},    {B4(AFTER_3AXISFLG), run04},
    {B4(AFTER_R04FLAG), run04},    {B4(AFTER_MUNFLAG), run04},
    {B4(AFTER_XOVINFLG_2), run04}, {B4(AFTER_POOH), run04},
    {B4(AFTER_CLRADMOD), run04},   {B4(AFTER_ENGINOF1), run04},
    {B4(AFTER_ALLCOAST), run04},   {B4(AFTER_V37KLEAN), run04},
    {B4(AFTER_P00KLEAN), run04},   {B4(KILL2), run04},
    {B4(AFTER_IMUSE), run04},      {B4(AFTER_FALTON), run04},
    {B4(V37XEQ), run04},           {B4(V37XEQC), run04},
    {B4(AFTER_NEWMODEA), run04},   {B4(AFTER_RELDSP_2), run04}};
const uint16_t lm_v37_entries[LM_V37_ROUTINES] = {GOP00FIX,         AFTER_XOVINFLG,
                                                  AFTER_ULLAGFLG,   AFTER_CLEARMRK,
                                                  AFTER_GOFLASH_1,  AFTER_GOFLASH_2,
                                                  AFTER_GOFLASH_3,  V37,
                                                  V37BAD,           AFTER_RELDSP,
                                                  AFTER_AVEGFLAG,   V37RET,
                                                  CANV37,           R00,
                                                  AFTER_INTPRET,    AFTER_3AXISFLG,
                                                  AFTER_R04FLAG,    AFTER_MUNFLAG,
                                                  AFTER_XOVINFLG_2, AFTER_POOH,
                                                  AFTER_CLRADMOD,   AFTER_ENGINOF1,
                                                  AFTER_ALLCOAST,   AFTER_V37KLEAN,
                                                  AFTER_P00KLEAN,   KILL2,
                                                  AFTER_IMUSE,      AFTER_FALTON,
                                                  V37XEQ,           V37XEQC,
                                                  AFTER_NEWMODEA,   AFTER_RELDSP_2};
const agc_routine_table lm_v37_routines = {routines, COUNT(routines)};
#else
static const agc_routine_entry routines[] = {
    {B4(GOP00FIX), run04},        {B4(AFTER_CLEARMRK), run04},   {B4(AFTER_GOFLASH_1), run04},
    {B4(AFTER_GOFLASH_2), run04}, {B4(AFTER_GOFLASH_3), run04},  {B4(V37), run04},
    {B4(AFTER_SPSOFF), run04},    {B4(AFTER_MASSPROP), run04},   {B4(AFTER_RCSDAPON), run04},
    {B4(AFTER_TVCZAP), run04},    {B4(AFTER_DELAYJOB), run04},   {B4(V37BAD), run04},
    {B4(AFTER_RELDSP), run04},    {B4(CANV37), run04},           {B4(ROC), run04},
    {B4(AFTER_INTPRET), run04},   {B4(AFTER_CLEARMRK_2), run04}, {B4(AFTER_STIKFLAG), run04},
    {B4(AFTER_UPACTOFF), run04},  {B4(AFTER_VHFRFLAG), run04},   {B4(AFTER_R21MARK), run04},
    {B4(AFTER_POOH), run04},      {B4(AFTER_V37KLEAN), run04},   {B4(GOMOD), run04},
    {B4(AFTER_IMUSE), run04},     {B4(AFTER_FALTON), run04},     {B4(V37XEQ), run04},
    {B4(V37XEQC), run04},         {B4(AFTER_NEWMODEA), run04},   {B4(AFTER_RELDSP_2), run04},
    {B4(AFTER_STOPRATE), run04},  {B4(AFTER_SETMINDB), run04},   {B4(RAKE), run04},
    {B4(AFTER_UPFLAG), run04},    {B20(VAC5STOR), run20},        {B20(AFTER_GENTRAN), run20}};
const cm_v37_entry cm_v37_entries[CM_V37_ROUTINES] = {
    {04, GOP00FIX},        {04, AFTER_CLEARMRK},   {04, AFTER_GOFLASH_1},
    {04, AFTER_GOFLASH_2}, {04, AFTER_GOFLASH_3},  {04, V37},
    {04, AFTER_SPSOFF},    {04, AFTER_MASSPROP},   {04, AFTER_RCSDAPON},
    {04, AFTER_TVCZAP},    {04, AFTER_DELAYJOB},   {04, V37BAD},
    {04, AFTER_RELDSP},    {04, CANV37},           {04, ROC},
    {04, AFTER_INTPRET},   {04, AFTER_CLEARMRK_2}, {04, AFTER_STIKFLAG},
    {04, AFTER_UPACTOFF},  {04, AFTER_VHFRFLAG},   {04, AFTER_R21MARK},
    {04, AFTER_POOH},      {04, AFTER_V37KLEAN},   {04, GOMOD},
    {04, AFTER_IMUSE},     {04, AFTER_FALTON},     {04, V37XEQ},
    {04, V37XEQC},         {04, AFTER_NEWMODEA},   {04, AFTER_RELDSP_2},
    {04, AFTER_STOPRATE},  {04, AFTER_SETMINDB},   {04, RAKE},
    {04, AFTER_UPFLAG},    {020, VAC5STOR},        {020, AFTER_GENTRAN}};
const agc_routine_table cm_v37_routines = {routines, COUNT(routines)};
#endif
