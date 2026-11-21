/* SPDX-License-Identifier: MIT */
/* pin-edges: four ways through Pinball that neither the port's cases nor
 * pin-stirred's variants of them check, each a state a crew could make.
 *
 *   - OPDEGOUT (CM p. 329, the optics' trunnion angle) with -0 in MPAC:
 *     "IF NEG ZERO SUBTRACT 1". The decimal display, from DSPDCPUT, of
 *     noun 91's second component (OCDU ANGLES, the trunnion: NNTYPTAB's
 *     00102 and RUTMXTAB's 00302), whose word, complemented in YREG, is
 *     -0; then the first component's, by DEGOUTSF.
 *   - M/SOUT (LM p. 422, CM p. 339) on a time that is the limit itself,
 *     59M58.5S (025 and 36652, with either sign): the two words' tests
 *     both come out -0 and fall into M/SLIMIT, "= 59M58.5S LIMIT". Times
 *     one count either side of it go with them.
 *   - PUTNORM (LM pp. 429-430, CM pp. 347-348), a load in octal, on noun
 *     10: "IF NNADTEM = -1, CHANNEL TO BE SPECIFIED" (CHANLOAD), with
 *     channel 7 named ("DONT LOAD CHAN 7") and with channel 011.
 *   - ENDNUM (LM p. 400, CM p. 317), the last digit of a word keyed in
 *     octal after an earlier word of the load was keyed in decimal. That
 *     word's sign left its bit in DECBRNCH (BOTHSGN: "BIT 5 FOR R1. BIT 4
 *     FOR R2."), and 5BLANK, which made the next register ready, cleared
 *     that register's own bit and the low two only ("PROTECT OTHERS"). So
 *     DECBRNCH is 020, 010 or 030 there, and CAF THREE; MASK DECBRNCH must
 *     find +0 in it and end the word as octal (ENDALL), where A's own
 *     word, -0, masked with it would go to DECEND. With them the last
 *     digit of a decimal word after another, which does go to DECEND.
 *     pin-stirred makes such states too, but a routine that refuses one
 *     only lowers its count of chains taken; here every chain from NUM
 *     must be taken.
 *
 * The states are the port's own cases from those entries (its pin-transfer
 * chains, c64-port/tests/lanec_pin_transfer_cases.c) with the few words
 * above changed, run on as chains until they leave the file. The native
 * runner runs them through the program's routine tables, with the readable
 * Pinball in tests/pinball_transfer.py's runner; the oracle runs the
 * original program in Virtual AGC to the same place. Test only. */
#include <string.h>
#include "lanec_suites.h"
#include "lanec_symbols.h"

enum { DSPDCPUT_ENTRY, M_SOUT_ENTRY, PUTNORM_ENTRY, NUM_ENTRY, ENTRIES };
#define PER_ENTRY 4u /* the port's cases from each entry, in each mission */
/* DSPDCPUT's in the CM, the other three's in both programs. */
#define BASES    (7u * PER_ENTRY)
#define VARIANTS 6u
#define CASES    (BASES * VARIANTS)

/* The words changed, the same in both programs. */
#define DECOUNT_AT  0117u
#define CHAR_AT     0117u /* the same word, by its other name */
#define MIXBR_AT    0140u
#define NOUNADD_AT  0145u
#define NNADTEM_AT  0146u
#define NNTYPTEM_AT 0147u
#define RUTMXTEM_AT 0153u
#define MPAC_AT     0154u
#define DSPCOUNT_AT 0777u
#define DECBRNCH_AT 01000u
#define XREG_AT     01003u
#define NOUNCADR_AT 01017u
/* A pair of unswitched words no routine of the chains names: M/SOUT's
 * time. */
#define TIME_AT 01300u

/* The entries by bank and address, the LM's and the CM's: DSPDCPUT (the
 * CM alone has OPDEGOUT), M/SOUT, PUTNORM and NUM. */
static const struct {
    uint16_t bank, lm, cm;
} entries[ENTRIES] = {{041u, 0u, 02537u},
                      {042u, 03303u, 03216u},
                      {041u, 03137u, 03137u},
                      {040u, LANEC_LM_NUM, LANEC_CM_NUM}};

static struct {
    uint16_t number; /* the case of pin-transfer */
    int entry;
} bases[BASES];
static uint16_t base_count;

static int entry_of(const apollo_overlay_state *s, const lanec_io *io, enum lanec_mission m)
{
    uint16_t z = s->words[LANEC_Z], bank = lanec_current_bank(s, io);
    int i;

    for (i = 0; i < ENTRIES; ++i)
        if (bank == entries[i].bank && z == (m == LANEC_LM ? entries[i].lm : entries[i].cm))
            return i;
    return -1;
}

/* The first PER_ENTRY cases of each entry in each mission. */
static void find_bases(void)
{
    static apollo_overlay_state s;
    static lanec_io io;
    static lanec_meta m;
    unsigned taken[2][ENTRIES] = {{0}};
    uint32_t k;
    int entry;

    for (k = 0; k < lanec_suite_pin_transfer.count; ++k) {
        lanec_suite_pin_transfer.make(k, &s, &io, &m);
        entry = entry_of(&s, &io, m.mission);
        if (entry >= 0 && base_count < BASES &&
            taken[m.mission == LANEC_LM ? 0 : 1][entry]++ < PER_ENTRY) {
            bases[base_count].number = (uint16_t)k;
            bases[base_count++].entry = entry;
        }
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    /* The limit's two words less one count, the limit, and one more. */
    static const uint16_t low[3] = {036651u, 036652u, 036653u};
    uint16_t *w = s->words, variant;

    if (!base_count) find_bases();
    lanec_suite_pin_transfer.make(bases[n % base_count].number, s, io, m);
    variant = (uint16_t)(n / base_count % VARIANTS);
    switch (bases[n % base_count].entry) {
    case DSPDCPUT_ENTRY:
        /* Noun 91, its second component next: the trunnion's -0 (and with
         * it +0, +1 and -1, and angles at the bias's edges), complemented
         * as DSPDCGET leaves it. */
        w[MIXBR_AT] = 2u;
        w[NNTYPTEM_AT] = 0102u;
        w[RUTMXTEM_AT] = 0302u;
        w[DECOUNT_AT] = 1u;
        w[XREG_AT + 1u] =
            (uint16_t)((uint16_t[]){077777u, 0u, 1u, 077776u, 027777u, 050000u}[variant] ^ 077777u);
        break;
    case M_SOUT_ENTRY:
        w[MIXBR_AT] = 1u; /* a normal noun: READLO takes the pair at NOUNADD */
        w[NOUNADD_AT] = TIME_AT;
        w[TIME_AT] = 025u;
        w[TIME_AT + 1u] = low[variant % 3u];
        if (variant >= 3u) { /* the same time, negative */
            w[TIME_AT] ^= 077777u;
            w[TIME_AT + 1u] ^= 077777u;
        }
        break;
    case NUM_ENTRY: {
        /* The last digit of R2 (DSPCOUNT 5, CRITCON's word for it) or of
         * R3 (0), and DECBRNCH as the words keyed before it left it. */
        static const struct {
            uint16_t dspcount, decbrnch, so_far;
        } last[VARIANTS] = {{5u, 020u, 01234u}, /* R2 octal; R1 was decimal */
                            {0u, 020u, 07654u}, /* R3 octal; R1 was decimal */
                            {0u, 010u, 0u},     /* R3 octal; R2 was decimal */
                            {0u, 030u, 07777u}, /* R3 octal; R1 and R2 were */
                            {5u, 031u, 1234u},  /* R2 decimal, +; R1 was too */
                            {0u, 036u, 9876u}}; /* R3 decimal, -; all three */
        w[DSPCOUNT_AT] = last[variant].dspcount;
        w[DECBRNCH_AT] = last[variant].decbrnch;
        w[CHAR_AT] = (uint16_t)(variant + 2u); /* the digit keyed, 2 to 7 */
        /* The word so far, four digits: YREG's or ZREG's. */
        w[XREG_AT + (last[variant].dspcount ? 1u : 2u)] = last[variant].so_far;
        break;
    }
    default:
        w[DECBRNCH_AT] = 0u;     /* octal */
        w[NNADTEM_AT] = 077776u; /* noun 10's word of NNADTAB */
        w[NNTYPTEM_AT] = 0u;     /* and of NNTYPTAB: one component, octal only */
        w[NOUNCADR_AT] = variant % 2u ? 011u : 7u;
        w[MPAC_AT] = (uint16_t)(variant / 2u ? 077777u : 052525u >> variant);
        break;
    }
}

static enum lanec_status run(apollo_overlay_state *s, lanec_io *io, enum lanec_mission mission)
{
    return lanec_suite_pin_transfer.run(s, io, mission);
}

const lanec_suite pin_edges = {"pin-edges", CASES, make, run};
