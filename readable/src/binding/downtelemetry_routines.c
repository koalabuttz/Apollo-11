/* SPDX-License-Identifier: MIT */
/* The readable down-telemetry (src/downtelemetry/) as routines of the
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_downtelemetry.c, cm_downtelemetry.c):
 * lm_downtelemetry_routines and cm_downtelemetry_routines.
 *
 * Plumbing only: point the machine at the program's state, give it the
 * fixed words the program reads (the downlists and constants, from the
 * faithful port's compiled listing data) and the channels of the DNTM io
 * record, and take the states the faithful routines take. A routine that
 * meets a step it does not model (not_modelled(), or a fixed word or a
 * channel it has none for) is not taken: the state is put back. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "../downtelemetry/downtelemetry.h"
#include "../agc/names.h"
#if LUMINARY099
#include "lm_downtelemetry.h"
#include "lm_downtelemetry_data.inc"
typedef lm_dntm_io dntm_io;
#define DATA       lm_dntm_data
#define DATA_COUNT LM_DNTM_DATA_COUNT
#else
#include "cm_downtelemetry.h"
#include "cm_downtelemetry_data.inc"
typedef cm_dntm_io dntm_io;
#define DATA       cm_dntm_data
#define DATA_COUNT CM_DNTM_DATA_COUNT
#endif

#define ZERO_REGISTER 7u
#define BANK05        012000u /* FBANK for bank 05 */
#define DNDUMPI_PLUS3 (DNDUMPI + 3u)

/* Keys: bank 05, and DNTMGOTO's erasable key. */
#define B5(z)        (uint16_t)(5u * 1024u + ((z) - 02000u))
#define ERASABLE_KEY (uint16_t)(AGC_TRANSFER_ERASABLE_BASE + ADDRESS_OF(DNTMGOTO))

static dntm_io *io_now;
static jmp_buf back;

/* For the tests: set when a routine reads bank 05's first word, 02000 (the
 * second word of a double word at 01777, which the listing rules out). */
bool downtelemetry_read_02000;

static void leave(void)
{
    longjmp(back, 1);
}

/* The fixed words the program reads: the downlists, DNTABLE and the
 * constants CCS runs as instructions. */
static word fixed_memory(unsigned physical)
{
    agc_word value = 0;

    if (physical == B5(02000u)) downtelemetry_read_02000 = true;
    if (!agc_site_lookup(DATA, DATA_COUNT, (uint16_t)physical, &value)) leave();
    return (word)(value & MINUS_ZERO);
}

/* ---- The channels of the DNTM io record ---- */

#if LUMINARY099
#define CHANNEL(n) (io_now->channel##n)
#define TRAPS      io_now
#else
#define CHANNEL(n) (io_now->io.channel##n)
#define TRAPS      (&io_now->io)
#endif

/* The channels a READ, RAND or WAND reads: 11-14 and the inputs 30-33.
 * The LM reads bits 12-14 of channel 13 (the HANDRUPT traps) as 0. */
static word channel_in(unsigned channel)
{
    switch (channel) {
    case 011u: return CHANNEL(11);
    case 012u: return CHANNEL(12);
#if LUMINARY099
    case 013u: return (word)(CHANNEL(13) & 043777u);
#else
    case 013u: return CHANNEL(13);
#endif
    case 014u: return CHANNEL(14);
    case 030u: return io_now->channel30;
    case 031u: return io_now->channel31;
    case 032u: return io_now->channel32;
    case 033u: return io_now->channel33;
    default: leave(); return 0;
    }
}

/* The channels the program writes: 13, whose bits 12-14 set the HANDRUPT
 * traps (as yaAGC's channel write does) and are not kept, and DNTM1 and
 * DNTM2. */
static void channel_out(unsigned channel, word value)
{
    switch (channel) {
    case 013u:
        if (value & 004000u) TRAPS->trap31a = 1;
        if (value & 010000u) TRAPS->trap31b = 1;
        if (value & 020000u) TRAPS->trap32 = 1;
        CHANNEL(13) = (agc_word)(value & 043777u);
        return;
    case 034u: io_now->channel34 = value; return;
    case 035u: io_now->channel35 = value; return;
    default: leave();
    }
}

/* ---- The states the faithful routines take ---- */

/* The words DNTMGOTO can hold: the places this program and the fresh
 * start store there. */
static bool goto_as_expected(void)
{
    word to = DNTMGOTO;
    return to == DNPHASE1 || to == DNPHASE2 || to == DNDUMPI || to == DNDUMPI_PLUS3 ||
           to == DNDUMP || to == DNDUMP1;
}

static bool takes(word entry)
{
    if (FBANK != BANK05 || agc.memory[ZERO_REGISTER] != 0u || (*agc.superbank & ~0160u) != 0u ||
        *agc.interrupts > 1u)
        return false;
    switch (entry) {
    case DODOWNTM: return overflow_of(A) == 0; /* TS BANKRUPT stores the BBANK */
    case ADDRESS_OF(DNTMGOTO): return goto_as_expected();
    default: return true;
    }
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static const agc_native_program none;
    static apollo_overlay_state before;
    static dntm_io io_before;
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
    switch (entry) {
    case DODOWNTM: dodowntm(); break;
    case ADDRESS_OF(DNTMGOTO): dntmgoto(); break;
    case DNPHASE1: dnphase1(); break;
    case DNPHASE2: dnphase2(); break;
    case DNDUMPI: dndumpi(); break;
    case DNDUMPI_PLUS3: dndumpi_plus3(); break;
    case DNDUMP: dndump(); break;
    case DNDUMP1: dndump1(); break;
    default: return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

/* In the faithful table's order (by key). */
static const agc_routine_entry routines[] = {
    {B5(DODOWNTM), run},      {B5(DNPHASE1), run}, {B5(DNPHASE2), run}, {B5(DNDUMPI), run},
    {B5(DNDUMPI_PLUS3), run}, {B5(DNDUMP), run},   {B5(DNDUMP1), run},  {ERASABLE_KEY, run}};
#if LUMINARY099
const agc_routine_table lm_downtelemetry_routines = {routines, COUNT(routines)};
#else
const agc_routine_table cm_downtelemetry_routines = {routines, COUNT(routines)};
#endif
