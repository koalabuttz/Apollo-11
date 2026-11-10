/* SPDX-License-Identifier: MIT */
/* The readable IMU mode switching routines (src/imu_mode_switching/) as
 * routines of each mission's flight program, under the table names and
 * keys of the faithful port's (c64-port/src/imu_mode_switching.c):
 * lm_imu_mode_switching_routines and cm_imu_mode_switching_routines, 73
 * keys each. ZEROICDU is in fixed-fixed; the mode switching, gyro and
 * stall code in bank 07; R02 in bank 04 (the CM's in bank 07); P06 and
 * P05 in bank 37 (the CM's in bank 26).
 *
 * Plumbing only. A key's function runs until control leaves the file. The
 * io record is the faithful port's: lm_imu_io for the LM (its first member
 * the p66_bias_io), cm_native_io for the CM. Channels 12, 13 and 14 are
 * there (a write to channel 13 keeps bits 12-14 clear: they are trap
 * enables, not storage); channel 11, which ENDIMU reads, is the program's
 * agc_hardware word. The rope is read for GSELECT's packed words, GYROFRAC and the
 * word -GYROMIN.
 *
 * Refused, as the faithful port refuses them (states the program does not
 * produce, where an index or a computed jump would leave the words the
 * source addresses), with the state and the io record as they were:
 *   - ZEROICDU, SETCOARS, RNDREFDR and NOATTOFF with Q outside 010-07777
 *     (TC Q would not run a TC);
 *   - BADEND and GOODEND with A above 2 (the MODECADR index); MODESLP +1
 *     with RUPTREG2 above 2; ENDMODE -4 with LOCCTR above 01613;
 *   - GWAKE2 +3 with MPAC +5's low eight bits above 0372, or an EBANK of
 *     0 that makes 1400-1407 the central registers; GYROAGRE +011 with
 *     MPAC +4 above 0376, a further pass indexing past 01776, or EBANK 0
 *     reaching the registers or the words it tests;
 *   - STRTGYRO with LGYRO negative or 040000 and above, or a GSELECT step
 *     whose index reaches 0377 or the registers; 8192AUG with EBANK 0 and
 *     an index of 4-6;
 *   - SCALPREP +3 with MPAC +2 outside 010-07776;
 *   - PRESTAND +017 and POSTAND with channel 13 bits 12-14 set;
 *   - with no agc_hardware, the keys that can reach ENDIMU: IMUZERO3
 *     +011, NEXTCDU +012, IMUFINED, NORESET and ENDIMU. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "imu_mode_switching.h"
#include "rope_image.h"
#include "../imu_mode_switching/imu_mode_switching.h"
#include "../agc/names.h"

#if LUMINARY099
typedef lm_imu_io ims_io;
#define ROPE     0u
#define R02_BANK 04u
#define P06_BANK 037u
#else
typedef cm_native_io ims_io;
#define ROPE     1u
#define R02_BANK 07u
#define P06_BANK 026u
#endif

/* Keys: physical addresses. */
#define IN(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))
#define B07(z)      IN(07u, z)
#define R02(z)      IN(R02_BANK, z)
#define P06K(z)     IN(P06_BANK, z)

static ims_io *io_now;
static const agc_hardware *hardware_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

static agc_word *channel_at(unsigned number)
{
    switch (number) {
    case 012u: return &io_now->channel12;
    case 013u: return &io_now->channel13;
    case 014u: return &io_now->channel14;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    if (number == DSALMOUT) {
        if (!hardware_now) not_modelled();
        return (word)(hardware_now->channel11 & MINUS_ZERO);
    }
    return (word)(*channel_at(number) & MINUS_ZERO);
}

static void channel_out(unsigned number, word value)
{
    *channel_at(number) = number == CHAN13 ? (agc_word)(value & 043777u) : value;
}

static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* ---- The states the faithful routines take ---- */

/* EBANK 0 makes 1400-1407 the central registers: a DCA or DXCH there at
 * an index of 0-7 reaches A, L, Q, the banks, Z or ZERO. */
static bool e0_registers(word ebank, word index)
{
    return ((ebank >> 8) & 7u) == 0u && index <= 7u;
}

static bool any(void)
{
    return true;
}

static bool q_returns(void)
{
    return Q >= 010u && Q <= 07777u;
}

/* The keys that can reach ENDIMU's READ DSALMOUT. */
static bool has_hardware(void)
{
    return hardware_now != NULL;
}

static bool device_index(void)
{
    return A <= 2u;
}

static bool sleeping_device(void)
{
    return RUPTREG2 <= 2u;
}

static bool woken_job(void)
{
    return LOCCTR <= 01613u;
}

static bool arriving_ecadr(void)
{
    word ecadr = MPAC(5);

    return (ecadr & LOW8) <= 0372u && !e0_registers(ecadr, (word)(ecadr & LOW8));
}

static bool agreed_command(void)
{
    word n = MPAC(3), at = MPAC(4);
    bool again = n >= 1u && n <= 037777u;

    if (at > 0376u) return false;
    if (again && (ITEMP1 > 0376u || 2ul * (n - 1u) + ITEMP1 > 0376u)) return false;
    if (e0_registers(EBANK, at) || (again && e0_registers(EBANK, ITEMP1))) return false;
    /* Under EBANK 0 the DXCH can land on MPAC +3 or ITEMP1, which this
     * reads first. */
    return ((EBANK >> 8) & 7u) != 0u ||
           (at + 1u != ADDRESS_OF(MPAC) + 3u && at != ADDRESS_OF(MPAC) + 3u &&
            at + 1u != ADDRESS_OF(ITEMP1) && at != ADDRESS_OF(ITEMP1));
}

/* LGYRO's GSELECT index stays inside the gyro commands from here: no step
 * count past 7 and no index of 0377. Gyro X adds 2, Y adds 2, Z
 * subtracts 4 and adds 0, each with BIT13. */
static bool gyro_steps(void)
{
    word lgyro = (word)fetch(ADDRESS_OF(LGYRO));
    unsigned count;

    if (lgyro > 037777u) return false;
    for (;;) {
        count = lgyro >> 11;
        if ((count & 1u) || count >= 6u) return true;
        if (count == 4u) lgyro = (word)(lgyro - 4u);
        lgyro = (word)(lgyro + (count == 4u ? 0u : 2u) + BIT13);
        if ((lgyro & LOW8) == LOW8 || e0_registers(lgyro, (word)(lgyro & LOW8))) return false;
    }
}

static bool augment(void)
{
    word lgyro = (word)fetch(ADDRESS_OF(LGYRO));

    return ((lgyro >> 8) & 7u) != 0u || (lgyro & LOW8) < 4u || (lgyro & LOW8) > 6u;
}

static bool scaler_return(void)
{
    return MPAC(2) >= 010u && MPAC(2) <= 07776u;
}

static bool standby_channel(void)
{
    return (io_now->channel13 & 034000u) == 0u;
}

/* ---- Running a key ---- */

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, bool (*in_domain)(void), void (*routine)(void))
{
    ims_io *io = device, kept;
    apollo_overlay_state before;

#if LUMINARY099
    if (!io || !bind_machine(program, state, &io->control)) return EXEC_OUT_OF_SCOPE;
#else
    if (!bind_machine(program, state, io)) return EXEC_OUT_OF_SCOPE;
#endif
    io_now = io;
    hardware_now = program->hardware;
    agc.fixed_memory = rope_word;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.leave = leave;
    if (!in_domain()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

#define KEY(name, domain)                                                                          \
    static enum exec_status key_##name(const agc_native_program *program,                          \
                                       apollo_overlay_state *state, void *device)                  \
    {                                                                                              \
        return run(program, state, device, domain, name);                                          \
    }
KEY(zeroicdu, q_returns)
KEY(imuzero, any)
KEY(imuzero_after_alarm, any)
KEY(imuzeroa_after_waitlist, any)
KEY(modeexit, any)
KEY(imuzero2, any)
KEY(imuzero3, any)
KEY(imuzero3_after_setissw, has_hardware)
KEY(imucoars, any)
KEY(imucoars_after_waitlist, any)
KEY(coars, any)
KEY(coars2, any)
KEY(nextcdu_after_fixdelay, has_hardware)
KEY(coarserr_after_alarm, any)
KEY(ca_plus_ece, any)
KEY(setcoars, q_returns)
KEY(rndrefdr, q_returns)
KEY(imufine, any)
KEY(imufine_after_waitlist, any)
KEY(imufine_after_waitlist2, any)
KEY(imufined, has_hardware)
KEY(ifailok, any)
KEY(pfailok2_after_setissw, any)
KEY(pfailok, any)
KEY(noattoff, q_returns)
KEY(pipuse, any)
KEY(pipuse1, any)
KEY(pipfree2_after_setissw, any)
KEY(pipfree, any)
KEY(pipfree_after_alarm, any)
KEY(imupulse, any)
KEY(gwake2_after_waitlist, arriving_ecadr)
KEY(gyroagre_after_tpagree, agreed_command)
KEY(gwake, any)
KEY(strtgyro, gyro_steps)
KEY(noreset, has_hardware)
KEY(gyroexit, any)
KEY(aug3_after_waitlist, any)
KEY(eighty_one_ninety_two_aug, augment)
KEY(endimu, has_hardware)
KEY(imubad, any)
KEY(badend, device_index)
KEY(goodend, device_index)
KEY(goodend_after_jobwake, woken_job)
KEY(aotstall, any)
KEY(radstall, any)
KEY(imustall, any)
KEY(modeslp_after_makecadr, sleeping_device)
KEY(gomanur, any)
KEY(r02both, any)
KEY(r02both_after_varalarm, any)
KEY(r02zero, any)
KEY(r02zero_after_upflag, any)
KEY(p06, any)
KEY(prestand, any)
KEY(prestand_retry, any)
KEY(prestand_compatible, any)
KEY(prestand_after_rndrefdr, any)
KEY(prestand_after_downflag, any)
KEY(prestand_after_downflag2, standby_channel)
KEY(prestand_after_phaschng, any)
KEY(prestand_after_goperf1, any)
KEY(scalprep, any)
KEY(scalprep_after_finetime, scaler_return)
KEY(postand, standby_channel)
KEY(postand_retry, any)
KEY(postand_compatible, any)
KEY(postand_after_shortmp, any)
KEY(postand_after_tpagree, any)
KEY(postcom_after_tpagree, any)
KEY(postcom_after_downflag, any)

/* By physical address: the LM's R02 (bank 04) comes before bank 07, the
 * CM's (bank 07) after the stalls. */
#define R02_ENTRIES                                                                                \
    {R02(R02BOTH), key_r02both}, {R02(R02BOTH + 013u), key_r02both_after_varalarm},                \
        {R02(R02ZERO), key_r02zero}, {R02(R02ZERO + 2u), key_r02zero_after_upflag},

static const agc_routine_entry entries[] = {
    {FF(ZEROICDU), key_zeroicdu},
#if LUMINARY099
    R02_ENTRIES
#endif
    {B07(IMUZERO), key_imuzero},
    {B07(IMUZERO + 7u), key_imuzero_after_alarm},
    {B07(IMUZEROA + 023u), key_imuzeroa_after_waitlist},
    {B07(MODEEXIT), key_modeexit},
    {B07(IMUZERO2), key_imuzero2},
    {B07(IMUZERO3), key_imuzero3},
    {B07(IMUZERO3 + 011u), key_imuzero3_after_setissw},
    {B07(IMUCOARS), key_imucoars},
    {B07(IMUCOARS + 7u), key_imucoars_after_waitlist},
    {B07(COARS), key_coars},
    {B07(COARS2), key_coars2},
    {B07(NEXTCDU + 012u), key_nextcdu_after_fixdelay},
    {B07(COARSERR + 5u), key_coarserr_after_alarm},
    {B07(CA_PLUS_ECE), key_ca_plus_ece},
    {B07(SETCOARS), key_setcoars},
    {B07(RNDREFDR), key_rndrefdr},
    {B07(IMUFINE), key_imufine},
    {B07(IMUFINE + 015u), key_imufine_after_waitlist},
    {B07(IMUFINE + 021u), key_imufine_after_waitlist2},
    {B07(IMUFINED), key_imufined},
    {B07(IFAILOK), key_ifailok},
    {B07(PFAILOK2 + 4u), key_pfailok2_after_setissw},
    {B07(PFAILOK), key_pfailok},
    {B07(NOATTOFF), key_noattoff},
    {B07(PIPUSE), key_pipuse},
    {B07(PIPUSE1), key_pipuse1},
    {B07(PIPFREE2 + 2u), key_pipfree2_after_setissw},
    {B07(PIPFREE), key_pipfree},
    {B07(PIPFREE + 011u), key_pipfree_after_alarm},
    {B07(IMUPULSE), key_imupulse},
    {B07(GWAKE2 + 3u), key_gwake2_after_waitlist},
    {B07(GYROAGRE + 011u), key_gyroagre_after_tpagree},
    {B07(GWAKE), key_gwake},
    {B07(STRTGYRO), key_strtgyro},
    {B07(NORESET), key_noreset},
    {B07(GYROEXIT), key_gyroexit},
    {B07(AUG3 + 6u), key_aug3_after_waitlist},
    {B07(EIGHT_ONE_NINETY_TWO_AUG), key_eighty_one_ninety_two_aug},
    {B07(ENDIMU), key_endimu},
    {B07(IMUBAD), key_imubad},
    {B07(BADEND), key_badend},
    {B07(GOODEND), key_goodend},
    {B07(ENDMODE - 4u), key_goodend_after_jobwake},
    {B07(AOTSTALL), key_aotstall},
    {B07(RADSTALL), key_radstall},
    {B07(IMUSTALL), key_imustall},
    {B07(MODESLP + 1u), key_modeslp_after_makecadr},
    {B07(GOMANUR), key_gomanur},
#if COMANCHE055
    R02_ENTRIES
#endif
    {P06K(P06), key_p06},
    {P06K(PRESTAND), key_prestand},
    {P06K(PRESTAND + 5u), key_prestand_retry},
    {P06K(PRESTAND + 6u), key_prestand_compatible},
    {P06K(PRESTAND + 013u), key_prestand_after_rndrefdr},
    {P06K(PRESTAND + 015u), key_prestand_after_downflag},
    {P06K(PRESTAND + 017u), key_prestand_after_downflag2},
    {P06K(PRESTAND + 027u), key_prestand_after_phaschng},
    {P06K(PRESTAND + 032u), key_prestand_after_goperf1},
    {P06K(PRESTAND + 033u), key_prestand_after_goperf1},
    {P06K(PRESTAND + 034u), key_prestand_after_goperf1},
    {P06K(SCALPREP), key_scalprep},
    {P06K(SCALPREP + 3u), key_scalprep_after_finetime},
    {P06K(POSTAND), key_postand},
    {P06K(POSTAND + 010u), key_postand_retry},
    {P06K(POSTAND + 011u), key_postand_compatible},
    {P06K(POSTAND + 016u), key_postand_after_shortmp},
    {P06K(POSTAND + 021u), key_postand_after_tpagree},
    {P06K(POSTCOM + 4u), key_postcom_after_tpagree},
    {P06K(POSTCOM + 010u), key_postcom_after_downflag},
};

#if LUMINARY099
const agc_routine_table lm_imu_mode_switching_routines = {entries, COUNT(entries)};
#else
const agc_routine_table cm_imu_mode_switching_routines = {entries, COUNT(entries)};
#endif
