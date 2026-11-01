/* SPDX-License-Identifier: MIT */
/* pin-stirred: the port's pin-transfer chains (c64-port/tests/
 * lanec_pin_transfer_cases.c) from the entries of the readable Pinball's
 * parts, with the words those routines read stirred. pin-transfer starts
 * each routine from states the port's own module tests made, which leave
 * paths unrun (a decimal word's last digit, CLEAR on a higher pass, a sign
 * where none may go); here each such case comes in many variants: other
 * keys, DSP numbers, load modes, passes, registers and display words. The
 * native runner runs them through the program's routine tables, with the
 * readable Pinball in tests/pinball_transfer.py's runner; the oracle runs
 * the original program in Virtual AGC; both as pin-transfer's own cases
 * (its make and run). Test only. */
#include <string.h>
#include "lanec_suites.h"
#include "lanec_symbols.h"

/* The port's suite list (c64-port/tests/lanec_suites.c, built with its
 * lanec_find_suite renamed). */
const lanec_suite *port_lanec_find_suite(const char *name);

/* Part 1's code, both missions: CHARIN to 2BLANK in bank 40. */
#define PART1_WORDS 0515u
#define CASES       12000u

#define MAX_BASES 2400u
static uint16_t bases[MAX_BASES];
static uint16_t base_count;

static int in_readable_parts(const apollo_overlay_state *s, const lanec_io *io,
                             enum lanec_mission m)
{
    uint16_t z = s->words[LANEC_Z], charin = m == LANEC_LM ? LANEC_LM_CHARIN : LANEC_CM_CHARIN;
    return lanec_current_bank(s, io) == 040u && z >= charin && z <= charin + PART1_WORDS;
}

/* The pin-transfer cases from a readable entry. */
static void find_bases(void)
{
    static apollo_overlay_state s;
    static lanec_io io;
    static lanec_meta m;
    uint32_t k;

    for (k = 0; k < lanec_suite_pin_transfer.count && base_count < MAX_BASES; ++k) {
        lanec_suite_pin_transfer.make(k, &s, &io, &m);
        if (in_readable_parts(&s, &io, m.mission)) bases[base_count++] = (uint16_t)k;
    }
}

/* One of a few values, or any 15-bit word. */
static uint16_t pick(uint32_t *seed, const uint16_t *values, unsigned n)
{
    uint32_t r = lanec_rand(seed);
    return (uint16_t)(r % (n + 1u) < n ? values[r % (n + 1u)] : lanec_rand(seed) & 077777u);
}

#define PICK(seed, ...)                                                                            \
    pick(seed, (const uint16_t[]){__VA_ARGS__},                                                    \
         sizeof((const uint16_t[]){__VA_ARGS__}) / sizeof(uint16_t))

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, i;
    uint32_t seed = n * 2654435761u + 12345u;

    if (!base_count) find_bases();
    lanec_suite_pin_transfer.make(bases[n % base_count], s, io, m);
    if (n < base_count) return; /* the case itself first */
    for (i = 0; i < 16u; ++i) {
        switch (lanec_rand(&seed) % 16u) {
        case 0: /* DSPCOUNT: a DSP number, past the last, negative or -0 */
            w[LANEC_LM_DSPCOUNT] = PICK(&seed, 0, 1, 2, 3, 4, 5, 6, 9, 10, 11, 14, 15, 16, 17, 18,
                                        19, 20, 021, 022, 023, 077777, 077776, 077770);
            break;
        case 1: /* DECBRNCH: octal, + or - decimal, the registers' sign bits */
            w[LANEC_LM_DECBRNCH] =
                (uint16_t)(PICK(&seed, 0, 1, 2, 3) |
                           (lanec_rand(&seed) % 2u ? 0u : (lanec_rand(&seed) & 034u)));
            break;
        case 2: w[LANEC_LM_CLPASS] = PICK(&seed, 0, 1, 2, 5, 077776, 077777, 040000); break;
        case 3: w[LANEC_LM_CADRSTOR] = PICK(&seed, 0, 077777, 02000, 024567); break;
        case 4: w[LANEC_LM_MPAC] = (uint16_t)(lanec_rand(&seed) % 040u); break; /* a key */
        case 5: w[LANEC_LM_CHAR] = PICK(&seed, 0, 1, 2, 5, 7, 8, 9, 020); break;
        case 6: w[0137] = PICK(&seed, 0, 1, 2, 3, 4); break; /* INREL */
        case 7:
            w[01001u + lanec_rand(&seed) % 5u] =
                PICK(&seed, 0, 1, 7, 077, 0777, 07777, 012345, 077777);
            break;
        case 8:
            w[01006u + lanec_rand(&seed) % 3u] = (uint16_t)(lanec_rand(&seed) & 077777u);
            break; /* XREGLP */
        case 9:
            w[LANEC_LM_MPAC + 1u + lanec_rand(&seed) % 2u] =
                (uint16_t)(lanec_rand(&seed) & 077777u);
            break;
        case 10:
            w[LANEC_LM_DSPTAB + lanec_rand(&seed) % 12u] = (uint16_t)(lanec_rand(&seed) & 077777u);
            break;
        case 11: w[LANEC_LM_NOUT] = PICK(&seed, 0, 1, 5, 11); break;
        case 12: w[LANEC_LM_REQRET] = PICK(&seed, 0, 1, 077776, 077777); break;
        case 13: io->channel[011] = (uint16_t)(lanec_rand(&seed) & 077777u); break;
        case 14: w[LANEC_LM_DSPLOCK] = PICK(&seed, 0, 1, 2); break;
        default:
            w[0143] =
                (uint16_t)(lanec_rand(&seed) % 030u | (lanec_rand(&seed) % 2u ? 0u : 020000u));
            break; /* COUNT */
        }
    }
}

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    return lanec_suite_pin_transfer.run(s, io, mission);
}

static const lanec_suite pin_stirred = {"pin-stirred", CASES, make, run};

const lanec_suite *lanec_find_suite(const char *name)
{
    return strcmp(name, pin_stirred.name) ? port_lanec_find_suite(name) : &pin_stirred;
}
