/* SPDX-License-Identifier: MIT */
/* The readable service routines against the faithful port's, case by case
 * (make service-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * service_routines.c) and the faithful port's under the same name with
 * faithful_ in front (the Makefile renames it). Every entry runs on random
 * machine states from the same start through both. Where the faithful
 * routine runs, the readable one must too, with the same erasable memory,
 * registers, channels and interrupt state, word for word. Where the
 * faithful routine refuses (leaves the state to the original program), the
 * readable one may run it; where the readable one refuses, it must leave
 * the state as it found it. The faithful routines are certified against
 * the original program (c64-port/tests/test_service_transfer.c), as the
 * readable ones are by make service-cases' first step.
 *
 * The program's operands are the mission's service sites (the ADRES words
 * after every TC UPFLAG, DOWNFLAG and GENTRAN), one now and then made
 * negative or taken away; its fixed data (GENTRAN's) a made-up rope that
 * now and then has no word. Q is mostly at a site or at erasable ADRES
 * words, A fits the entry most of the time, and the DELAYLOC slots are
 * free or taken.
 *
 * One kind of state no program makes, where the faithful routine departs
 * from the AGC and the readable one refuses, is not counted: UPFLAG or
 * DOWNFLAG naming a flag word that is ITEMP3 itself (an EBANK 0 address
 * past FLAGWRD0 +01300). COMFLAG's LXCH then returns through the new flag
 * word; the faithful routine returns past the ADRES. */
#include <stdio.h>
#include <string.h>
#include "agc_transfer.h"
#define AGC_SITES_CALLEES
#if LUMINARY099
#include "p66_bias.h"
#include "lm_service_sites.inc"
typedef p66_bias_io mission_io;
#define MISSION    "LM"
#define TABLE      lm_service_routines
#define FAITHFUL   faithful_lm_service_routines
#define SITES      lm_service_sites
#define CALLEES    lm_service_sites_callee
#define SITE_COUNT LM_SERVICE_SITES_COUNT
#define DELAYLOC   01326u
#define DELAYNUM   2u
#define UPFLAG     05504u
#define DOWNFLAG   05516u
#define COMFLAG    05511u
#define DEBIT      05522u
#define GENTRAN    05544u
#else
#include "cm_native_image.h"
#include "cm_service_sites.inc"
typedef cm_native_io mission_io;
#define MISSION    "CM"
#define TABLE      cm_service_routines
#define FAITHFUL   faithful_cm_service_routines
#define SITES      cm_service_sites
#define CALLEES    cm_service_sites_callee
#define SITE_COUNT CM_SERVICE_SITES_COUNT
#define DELAYLOC   01141u
#define DELAYNUM   3u
#define UPFLAG     05546u
#define DOWNFLAG   05560u
#define COMFLAG    0u
#define DEBIT      0u
#define GENTRAN    05606u
#endif

extern const agc_routine_table TABLE, FAITHFUL;

#define CASES      40000u
#define ITEMP1     061u
#define ITEMP3     063u
#define RUPTREG1   070u
#define FLAGWRD0   074u
#define LAST_FLAG  14459u
#define SITE_OTHER 0u

static uint32_t seed = 1969u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static agc_site_operand operands[SITE_COUNT];
static int rope_has_words;

/* The made-up rope: a word for every physical address, unless this case
 * has none. */
static bool fixed_data(const agc_native_program *program, uint16_t physical, agc_word *word)
{
    (void)program;
    if (!rope_has_words) return false;
    *word = (agc_word)(((physical * 2654435761u) >> 9) & 077777u);
    return true;
}

/* A register value: mostly a sign-extended word, sometimes an overflow. */
static uint16_t reg(void)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    if (rnd() % 8u == 0u) return (uint16_t)(rnd() | (rnd() << 1));
    return (uint16_t)(w | ((w & 040000u) << 1));
}

/* A site whose callee is callee (or any site, for SITE_OTHER), as Q with
 * the banks that reach it. */
static uint16_t at_site(apollo_overlay_state *s, mission_io *io, uint16_t callee)
{
    unsigned i = rnd() % SITE_COUNT, tries;
    uint16_t physical, bank;
    for (tries = 0; callee != SITE_OTHER && tries < SITE_COUNT; ++tries, i = (i + 1u) % SITE_COUNT)
        if (CALLEES[i] == callee || (callee == GENTRAN && CALLEES[i] == GENTRAN + 1u)) break;
    physical = SITES[i].physical;
    bank = (uint16_t)(physical / 1024u);
    if (bank == 2u || bank == 3u) return (uint16_t)(bank * 02000u + physical % 1024u);
    if (bank >= 040u) {
        bank = (uint16_t)(bank - 010u);
        io->superbank = 0100u;
    } else if (bank >= 030u)
        io->superbank = 0u;
    s->words[4] = (uint16_t)(bank << 10);
    return (uint16_t)(02000u + physical % 1024u);
}

/* An erasable address of the kind given, as the switched bank maps it. */
static uint16_t physical_of(const apollo_overlay_state *s, uint16_t address)
{
    return address < 01400u ? address : (uint16_t)(s->words[3] + (address & 0377u));
}

/* A flag's name: mostly one of the flags, sometimes anything. */
static uint16_t flag_name(void)
{
    return rnd() % 8u ? (uint16_t)(rnd() % (LAST_FLAG + 1u)) : (uint16_t)(rnd() & 077777u);
}

/* Q at erasable ADRES words: name (UPFLAG, DOWNFLAG) or two addresses
 * (GENTRAN) stored there. */
static uint16_t erasable_adres(apollo_overlay_state *s, bool block)
{
    uint16_t q = (uint16_t)(030u + rnd() % 01750u);
    if (rnd() % 16u == 0u) q = (uint16_t)(rnd() % 0100u);
    if (block) {
        s->words[physical_of(s, q)] = (uint16_t)(0100u + rnd() % 01600u);
        s->words[physical_of(s, (uint16_t)(q + 1u))] = (uint16_t)(0100u + rnd() % 01600u);
        if (rnd() % 8u == 0u) s->words[physical_of(s, q)] = (uint16_t)(02000u + rnd() % 06000u);
    } else
        s->words[physical_of(s, q)] = flag_name();
    return q;
}

static void state(apollo_overlay_state *s, mission_io *io, uint16_t key)
{
    unsigned i;
    uint16_t z, fixed_bank = 0xffffu;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    for (i = 010; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = (uint16_t)(rnd() & 077777u);
    s->words[0] = reg();
    s->words[1] = reg();
    s->words[2] = (uint16_t)(0100u + rnd() % 07676u);
    s->words[3] = (uint16_t)((rnd() % 8u) << 8);
    s->words[4] = (uint16_t)((rnd() % 040u) << 10);
    io->superbank = (uint16_t)(rnd() & 0160u);
    io->interrupt_enabled = rnd() % 2u;
    if (key >= 2048u && key < 4096u)
        z = (uint16_t)(04000u + key - 2048u);
    else {
        fixed_bank = (uint16_t)(key / 1024u);
        z = (uint16_t)(02000u + key % 1024u);
    }
    for (i = 0; i <= DELAYNUM; ++i)
        if (rnd() % 2u) s->words[DELAYLOC + i] = 0;
    s->words[RUPTREG1] =
        rnd() % 8u ? (uint16_t)(rnd() % (DELAYNUM + 1u)) : (uint16_t)(rnd() & 077777u);
    s->words[ITEMP1] = rnd() % 2u ? (uint16_t)(rnd() % 01710u) : (uint16_t)(rnd() & 077777u);
    s->words[ITEMP3] =
        rnd() % 2u ? (uint16_t)(0100u + rnd() % 07700u) : (uint16_t)(rnd() & 077777u);

    if (z == UPFLAG || z == DOWNFLAG || (DEBIT && z == DEBIT)) {
        uint16_t q =
            rnd() % 4u ? at_site(s, io, rnd() % 2u ? UPFLAG : DOWNFLAG) : erasable_adres(s, false);
        if (z == DEBIT)
            s->words[0] = q;
        else
            s->words[2] = q;
    } else if (z == GENTRAN || z == GENTRAN + 1u) {
        s->words[2] = rnd() % 4u ? at_site(s, io, GENTRAN) : erasable_adres(s, true);
        s->words[0] = rnd() % 4u ? (uint16_t)(rnd() % 24u) : reg();
    }
    if (fixed_bank != 0xffffu) s->words[4] = (uint16_t)(fixed_bank << 10);
    s->words[6] = (uint16_t)(s->words[4] | (s->words[3] >> 8));
    s->words[5] = z;
    switch (rnd() % 32u) {
    case 0: s->words[7] = 1; break;
    case 1: io->superbank = 1; break;
    case 2: io->interrupt_enabled = 2; break;
    case 3: s->words[2] = (uint16_t)(rnd() | (rnd() << 1)); break;
    case 4: s->words[0] = (uint16_t)(rnd() | (rnd() << 1)); break;
    case 5: s->words[2] = (uint16_t)(rnd() % 0100u); break;
    default: break;
    }
}

/* An entry's place, as the listing shows it. */
static const char *where(uint16_t key)
{
    static char text[24];
    if (key >= 2048u && key < 4096u)
        sprintf(text, "%05o", (unsigned)(04000u + key - 2048u));
    else
        sprintf(text, "%02o,%04o", (unsigned)(key / 1024u), (unsigned)(02000u + key % 1024u));
    return text;
}

/* The flag word the flag at Q (UPFLAG, DOWNFLAG) names is ITEMP3. */
static bool faithful_departs(uint16_t key, const apollo_overlay_state *s,
                             const agc_native_program *program)
{
    uint16_t z = (uint16_t)(04000u + key - 2048u), q = s->words[2], flag;
    agc_word name;
    if (key < 2048u || key >= 4096u || (z != UPFLAG && z != DOWNFLAG)) return false;
    if (q >= 02000u) {
        if (!agc_transfer_operand(program, s, 0, q, &name)) return false;
    } else
        name = s->words[physical_of(s, q)];
    if (q < 02000u && physical_of(s, q) == ITEMP1) name = 15u;
    if (q < 02000u && physical_of(s, q) == ITEMP3) name = (agc_word)(q + 1u);
    if (name > LAST_FLAG) return false;
    flag = (uint16_t)(FLAGWRD0 + name / 15u);
    return flag < 02000u && physical_of(s, flag) == ITEMP3;
}

static agc_routine find(const agc_routine_table *t, uint16_t key)
{
    uint16_t i;
    for (i = 0; i < t->count; ++i)
        if (t->entries[i].physical == key) return t->entries[i].call;
    return 0;
}

int main(void)
{
    agc_native_program program;
    unsigned i, checks = 0, failures = 0, departs = 0;

    if (TABLE.count != FAITHFUL.count) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    memset(&program, 0, sizeof program);
    program.operands = operands;
    program.fixed_data = fixed_data;

    for (i = 0; i < FAITHFUL.count; ++i) {
        uint16_t key = FAITHFUL.entries[i].physical;
        agc_routine r = find(&TABLE, key), f = FAITHFUL.entries[i].call;
        unsigned n, ran = 0, refused = 0, bad = 0;
        if (!r) {
            printf("FAIL: no readable entry at key %u\n", key);
            ++failures;
            continue;
        }
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state a, b, start;
            mission_io ia, ib, istart;
            enum exec_status sa, sb;
            unsigned o;
            const char *why = 0;
            /* The operands: now and then one site's word negative or gone. */
            memcpy(operands, SITES, sizeof operands);
            program.operand_count = SITE_COUNT;
            o = rnd() % SITE_COUNT;
            if (rnd() % 16u == 0u)
                operands[o].word = (agc_word)((040000u | rnd()) & 077777u);
            else if (rnd() % 16u == 0u) {
                memmove(&operands[o], &operands[o + 1u],
                        (SITE_COUNT - o - 1u) * sizeof operands[0]);
                program.operand_count = SITE_COUNT - 1u;
            }
            rope_has_words = rnd() % 16u != 0u;
            state(&a, &ia, key);
            b = a;
            ib = ia;
            start = a;
            istart = ia;
            sa = f(&program, &a, &ia);
            sb = r(&program, &b, &ib);
            ++checks;
            if (sa == EXEC_OK) ++ran;
            if (sb != EXEC_OK) ++refused;
            if (sb != EXEC_OK) {
                if (memcmp(&b, &start, sizeof b) || memcmp(&ib, &istart, sizeof ib))
                    why = "readable refusal changed the state";
                else if (sa == EXEC_OK && faithful_departs(key, &start, &program))
                    ++departs;
                else if (sa == EXEC_OK)
                    why = "readable refused, faithful ran";
            } else if (sa == EXEC_OK && (memcmp(&a, &b, sizeof a) || memcmp(&ia, &ib, sizeof ia)))
                why = "different";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL %s case %u: %s (status %d, faithful %d)\n", where(key), n, why,
                       (int)sb, (int)sa);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (a.words[w] != b.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, b.words[w],
                               a.words[w], start.words[w]);
                if (ia.superbank != ib.superbank || ia.interrupt_enabled != ib.interrupt_enabled)
                    printf("  channels differ\n");
                printf("  start Q %06o A %06o L %06o FB %06o EB %06o SB %03o IE %o\n",
                       start.words[2], start.words[0], start.words[1], start.words[4],
                       start.words[3], (unsigned)istart.superbank,
                       (unsigned)istart.interrupt_enabled);
            }
        }
        failures += bad;
        printf("%s service entry %s: %u cases, %u run by the faithful routine, %u refused by the "
               "readable one, %u failures\n",
               MISSION, where(key), CASES, ran, refused, bad);
    }
    printf("service-cases %s: %u checks (%u where the faithful routine departs from the AGC), %u "
           "failures\n",
           MISSION, checks, departs, failures);
    return failures != 0;
}
