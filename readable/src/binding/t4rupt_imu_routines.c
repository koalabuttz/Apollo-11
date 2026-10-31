/* SPDX-License-Identifier: MIT */
/* The readable IMU monitor (src/t4rupt/imu_monitor.c) as routines of the
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_t4rupt_imu.c and cm_t4rupt_imu.c):
 * lm_t4rupt_imu_routines and cm_t4rupt_imu_routines.
 *
 * Plumbing only: point the machine at the program's state, give it
 * channels 11 (DSALMOUT), 12, 14, 30 and 33 from the program's hardware
 * record and the bit table BIT15 ... BIT1 the scans index, and take the
 * states the faithful routines take: in bank 06; continuing a bit scan,
 * RUPTREG1 and RUPTREG2 as the scans leave them; after IMUCAGE's RNDREFDR,
 * the drive bits of channel 14 cleared; back from VARALARM, ITEMP6 a
 * return in fixed memory. */
#include <stddef.h>
#include "binding.h"
#include "../t4rupt/imu_monitor.h"

#define BANK06 014000u

static agc_hardware *hardware_now;

static uint16_t *channel(unsigned number)
{
    switch (number) {
    case DSALMOUT: return &hardware_now->channel11;
    case CHAN12: return &hardware_now->channel12;
    case CHAN14: return &hardware_now->channel14;
    case CHAN30: return &hardware_now->channel30;
    case CHAN33: return &hardware_now->channel33;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return *channel(number);
}

/* Channel 33's bits 11-15 are flip-flops a write sets back to 1 (C33TEST:
 * "RESETS FLIP-FLOP INPUTS"); its other bits are inputs a write leaves. */
static void channel_out(unsigned number, word value)
{
    if (number == CHAN33)
        hardware_now->channel33 |= 076000u;
    else
        *channel(number) = value;
}

/* The fixed words the routines read: the table BIT15, BIT14 ... BIT1 that
 * INDEX RUPTREG1; CAF BIT14 (or BIT13) reads. */
static word fixed_memory(unsigned physical)
{
    if (physical >= FF(BIT15_ADDRESS) && physical <= FF(BIT15_ADDRESS + 14u))
        return (word)(040000u >> (physical - FF(BIT15_ADDRESS)));
    not_modelled();
    return 0;
}

/* ---- The states the faithful routines take ---- */

/* A bit scan the routines can go on with (NXTIFAIL, NXTFL33): RUPTREG2,
 * the changes left, not negative; RUPTREG1, the position just handled, -1
 * to 6 (-0 is 0); and each change left selecting a TCF of the table:
 * IFAILJMP's words 0-3 and 5 (word 4 is 30RDMSK), C33JMP's 0-2. */
static bool scan_as_left(bool channel30)
{
    word position = fetch(ADDRESS_OF(RUPTREG1)), changes = fetch(ADDRESS_OF(RUPTREG2));
    int handled, slot;
    unsigned bit;

    if (changes & SIGN_BIT) return false;
    if (position == negate(ONE))
        handled = -1;
    else if (position == MINUS_ZERO)
        handled = 0;
    else if (position <= 6u)
        handled = (int)position;
    else
        return false;
    for (bit = 1u; bit <= 14u; ++bit) {
        if (!(changes & (1u << (bit - 1u)))) continue;
        slot = handled + (int)(15u - bit);
        if (channel30 ? slot < 0 || slot > 5 || slot == 4 : slot < 0 || slot > 2) return false;
    }
    return true;
}

static bool takes(word entry)
{
    word back = fetch(ADDRESS_OF(ITEMP6));

    if (entry == ITURNON + 022u || entry == IMUCAGE_TC_CAGESUB1 + 3u || entry == IMUOP + 7u ||
        entry == IMUOP + 020u)
        if (!scan_as_left(true)) return false;
    if (entry == PIPFAIL + 020u || entry == DNTMFAST + 4u || entry == UPTMFAST + 4u)
        if (!scan_as_left(false)) return false;
    if (entry == IMUCAGE_TC_CAGESUB1 + 3u && (hardware_now->channel14 & OCT77000)) return false;
    if (entry == ISSWON + 3u) {
        if (back < 02000u || back > 07777u) return false;
        if (back == NXTIFAIL && !scan_as_left(true)) return false;
        if (back == PIPFAIL + 6u && !scan_as_left(false)) return false;
    }
    return true;
}

/* ---- The entries ---- */

static const struct {
    word at;
    void (*routine)(void);
} entries[] = {
    {IMUMON, imumon},
    {PROCTNON + 014u, proctnon_after_alarm},
    {PROCTNON + 021u, proctnon_after_waitlist},
    {ENDTNON, endtnon},
    {UNZ2, unz2},
    {UNZ2 + 1u, unz2_after_zeroicdu},
    {ISSUP, issup},
    {ISSUP_TC_SETISSW + 010u, issup_after_waitlist},
    {ISSZERO + 2u, isszero_after_noattoff},
    {ISSZERO + 6u, isszero_after_zeroicdu},
    {ISSZERO + 012u, isszero_after_waitlist},
    {GLOCKCHK + 015u, glockchk_after_setcoars},
    {NOGIMRUN, nogimrun},
    {ITURNON + 022u, iturnon_after_alarm},
    {IMUCAGE_TC_CAGESUB1 + 3u, imucage_after_rndrefdr},
    {IMUOP + 7u, imuop_after_rndrefdr},
    {IMUOP + 020u, imuop_after_alarm},
    {PIPFAIL + 020u, pipfail_after_alarm},
    {DNTMFAST + 4u, dntmfast_after_alarm},
    {UPTMFAST + 4u, uptmfast_after_alarm},
    {ISSWON + 3u, isswon_after_varalarm},
};

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    word entry;
    unsigned i;

    if (!state || !program || !program->hardware || !bind_machine(program, state, device))
        return EXEC_OUT_OF_SCOPE;
    hardware_now = program->hardware;
    if (FBANK != BANK06) return EXEC_OUT_OF_SCOPE;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = fixed_memory;
    entry = Z;
    if (!takes(entry)) return EXEC_OUT_OF_SCOPE;
    if (entry == SETISSW) { /* TC SETISSW from another file: back by TC Q */
        (void)setissw();
        return EXEC_OK;
    }
    for (i = 0; i < COUNT(entries); ++i) {
        if (entries[i].at == entry) {
            entries[i].routine();
            return EXEC_OK;
        }
    }
    return EXEC_OUT_OF_SCOPE;
}

/* Keys: bank 06, in the faithful tables' order. */
#define B6(z) (uint16_t)(06u * 1024u + ((z) - 02000u))

static const agc_routine_entry routines[] = {{B6(K_IMUMON), run},
                                             {B6(K_PROCTNON + 014u), run},
                                             {B6(K_PROCTNON + 021u), run},
                                             {B6(K_ENDTNON), run},
                                             {B6(K_UNZ2), run},
                                             {B6(K_UNZ2 + 1u), run},
                                             {B6(K_ISSUP), run},
                                             {B6(ISSUP_TC_SETISSW + 010u), run},
                                             {B6(K_ISSZERO + 2u), run},
                                             {B6(K_ISSZERO + 6u), run},
                                             {B6(K_ISSZERO + 012u), run},
                                             {B6(K_GLOCKCHK + 015u), run},
                                             {B6(K_NOGIMRUN), run},
                                             {B6(K_ITURNON + 022u), run},
                                             {B6(IMUCAGE_TC_CAGESUB1 + 3u), run},
                                             {B6(K_IMUOP + 7u), run},
                                             {B6(K_IMUOP + 020u), run},
                                             {B6(K_PIPFAIL + 020u), run},
                                             {B6(K_DNTMFAST + 4u), run},
                                             {B6(K_UPTMFAST + 4u), run},
                                             {B6(K_SETISSW), run},
                                             {B6(K_ISSWON + 3u), run}};
#if LUMINARY099
const agc_routine_table lm_t4rupt_imu_routines = {routines, COUNT(routines)};
#else
const agc_routine_table cm_t4rupt_imu_routines = {routines, COUNT(routines)};
#endif
