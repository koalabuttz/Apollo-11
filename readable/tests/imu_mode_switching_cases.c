/* SPDX-License-Identifier: MIT */
/* The readable IMU mode switching routines against the faithful port's,
 * case by case (make imu-mode-switching-cases), both missions in one
 * program.
 *
 * Built with the readable tables (src/binding/imu_mode_switching_routines.c,
 * each mission's objects with only its table name left global) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them). Every key of both tables runs on CASES states:
 * the port's own entry states (c64-port/tests/imu_mode_switching_cases.c,
 * ims_input(), shaped to each routine's domain), and in half the cases
 * one or more of: A any device index or sign; Q anywhere; MODECADR,
 * RUPTREG2, LOCCTR, LGYRO, MPAC +2 to +5, ITEMP1, ATTCADR or the gyro
 * commands any sign or out of range; IMODES30's cage bit; EBANK 0;
 * channel 13's trap bits; a random word anywhere; no agc_hardware. Where
 * either port runs, the other must too, with the same memory, io record
 * and channel 11, word for word; where the readable one refuses, it must
 * leave everything as it found it. The faithful routines are certified
 * against the original programs (c64-port/tests/test_imu_mode_switching.c),
 * as the readable ones are by make imu-mode-switching-cases' oracle step.
 * argv: the LM and CM ropes. */
#include <stdio.h>
#include <string.h>
#include "imu_mode_switching_cases.h"
#include "rope_image.h"

extern const agc_routine_table faithful_lm_imu_mode_switching_routines,
    faithful_cm_imu_mode_switching_routines;

#define CASES 4096u

/* Erasable words, by mission (LM, CM). */
static const uint16_t lgyro_at[2] = {01314u, 01304u}, modecadr_at[2] = {01304u, 01322u},
                      attcadr_at[2] = {01307u, 01325u}, imodes30_at[2] = {01302u, 01320u};
#define LOCCTR_AT   064u
#define RUPTREG2_AT 071u
#define ITEMP1_AT   061u
#define MPAC_AT     0154u

static uint32_t seed = 271828u;
static unsigned rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (unsigned)(seed >> 12) & 0xFFFFu;
}

static uint16_t any_sign(void)
{
    static const uint16_t v[8] = {1u, 0u, 077776u, 077777u, 2u, 077775u, 3u, 0400u};
    return rnd() % 4u ? v[rnd() % 8u] : (uint16_t)(rnd() & 077777u);
}

static void perturb(apollo_overlay_state *s, ims_case *c, int *no_hardware)
{
    unsigned m = rnd(), cm = c->cm, i;

    if (m & 1u) s->words[0] = rnd() % 2u ? (uint16_t)(rnd() % 4u) : any_sign();
    if (m & 2u)
        s->words[2] = rnd() % 2u ? (uint16_t)(rnd() % 2u ? 7u + rnd() % 2u : 07776u + rnd() % 3u)
                                 : (uint16_t)(rnd() & 077777u);
    if (m & 4u)
        for (i = 0; i < 3u; ++i) s->words[modecadr_at[cm] + i] = any_sign();
    if (m & 010u) s->words[RUPTREG2_AT] = (uint16_t)(rnd() % 4u);
    if (m & 020u) s->words[LOCCTR_AT] = (uint16_t)(01610u + rnd() % 8u);
    if (m & 040u) s->words[lgyro_at[cm]] = rnd() % 2u ? any_sign() : (uint16_t)(rnd() & 077777u);
    if (m & 0100u) s->words[MPAC_AT + 2u + rnd() % 4u] = rnd() % 2u ? any_sign() : 0373u;
    if (m & 0200u) s->words[ITEMP1_AT] = (uint16_t)(0370u + rnd() % 010u);
    if (m & 0400u) s->words[attcadr_at[cm]] = any_sign();
    if (m & 01000u) s->words[imodes30_at[cm]] ^= 040u;
    if (m & 02000u) s->words[3] = 0u;
    if (m & 04000u) c->channel[2] |= (uint16_t)((rnd() % 8u) << 11);
    if (m & 010000u) s->words[rnd() % 2048u] = (uint16_t)(rnd() & 077777u);
    if (m & 020000u) s->words[01400u + rnd() % 0400u] = any_sign(); /* a gyro command, maybe */
    if ((m & 0140000u) == 0140000u && rnd() % 4u == 0u) *no_hardware = 1;
}

static unsigned compare(const agc_routine_table *readable, const agc_routine_table *faithful,
                        unsigned first, const char *name)
{
    unsigned i, n, failures = 0;

    if (readable->count != faithful->count) {
        printf("FAIL %s: %u keys, faithful %u\n", name, readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        agc_routine f = faithful->entries[i].call, r = readable->entries[i].call;
        unsigned ran = 0, bad = 0;

        if (readable->entries[i].physical != key) {
            printf("FAIL %s key %u at %u, faithful %u\n", name, i, readable->entries[i].physical,
                   key);
            ++failures;
            continue;
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            ims_case c;
            lm_imu_io lio, flio, rlio;
            cm_native_io cio, fcio, rcio;
            agc_hardware hw, fhw, rhw;
            agc_native_program fp, rp;
            enum exec_status sf, sr;
            int no_hardware = 0;
            const char *why = 0;
            void *fio, *rio;

            ims_input(&start, &c, (uint16_t)(first + i + IMS_ROUTINES * (n % IMS_ENTRY_STATES)));
            if (n % 2u) perturb(&start, &c, &no_hardware);
            memset(&lio, 0, sizeof lio);
            memset(&cio, 0, sizeof cio);
            memset(&hw, 0, sizeof hw);
            lio.control.superbank = cio.superbank = c.superbank;
            lio.control.interrupt_enabled = cio.interrupt_enabled = c.interrupt_enabled;
            lio.channel12 = cio.channel12 = c.channel[1];
            lio.channel13 = cio.channel13 = c.channel[2];
            lio.channel14 = cio.channel14 = c.channel[3];
            hw.channel11 = c.channel[0];
            fa = ra = start;
            flio = rlio = lio;
            fcio = rcio = cio;
            fhw = rhw = hw;
            memset(&fp, 0, sizeof fp);
            memset(&rp, 0, sizeof rp);
            fp.hardware = no_hardware ? NULL : &fhw;
            rp.hardware = no_hardware ? NULL : &rhw;
            fio = c.cm ? (void *)&fcio : (void *)&flio;
            rio = c.cm ? (void *)&rcio : (void *)&rlio;
            sf = f(&fp, &fa, fio);
            sr = r(&rp, &ra, rio);
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK &&
                (memcmp(&ra, &start, sizeof ra) || memcmp(&rlio, &lio, sizeof lio) ||
                 memcmp(&rcio, &cio, sizeof cio)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK &&
                     (memcmp(&flio, &rlio, sizeof flio) || memcmp(&fcio, &rcio, sizeof fcio) ||
                      fhw.channel11 != rhw.channel11))
                why = "different io record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %s %05o case %u: %s\n", name, key, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran == 0u && bad == 0u) {
            printf("FAIL %s %05o: the faithful routine never ran\n", name, key);
            ++bad;
        }
        failures += bad;
        printf("%s %05o: %u cases, %u run by the faithful routine, %u failures\n", name, key, CASES,
               ran, bad);
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned failures = 0;

    if (argc != 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        fprintf(stderr, "usage: ims-cases LM-rope CM-rope\n");
        return 2;
    }
    failures += compare(&lm_imu_mode_switching_routines, &faithful_lm_imu_mode_switching_routines,
                        0u, "LM IMU mode switching");
    failures += compare(&cm_imu_mode_switching_routines, &faithful_cm_imu_mode_switching_routines,
                        IMS_LM_ROUTINES, "CM IMU mode switching");
    printf("imu-mode-switching-cases: %u failures\n", failures);
    return failures != 0;
}
