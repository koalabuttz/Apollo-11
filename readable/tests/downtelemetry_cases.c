/* SPDX-License-Identifier: MIT */
/* The readable down-telemetry against the faithful port's, step by step
 * (make downtelemetry-cases).
 *
 * Built once per mission, with the readable table (src/binding/
 * downtelemetry_routines.c) and the faithful port's under the same name
 * with faithful_ in front (the Makefile renames it). Each case is a run of
 * DOWNRUPTs from a random erasable memory, sending a downlist (or, now and
 * then, the erasable dump) as the flight program does: each DOWNRUPT's
 * steps (DODOWNTM, DNTMGOTO, then where DNTMGOTO goes) run through both
 * from the same state. Where the faithful routine runs, the readable one
 * must too, with the same erasable memory, registers and channels, word
 * for word. Where the faithful routine refuses, the readable one may run;
 * where the readable one refuses, it must leave the state as it found it.
 * The run goes on from the faithful routine's state (or the readable
 * one's, if only it ran).
 *
 * Now and then, before a step, one thing is put out of place: a control
 * word (DNECADR, SUBLIST, CTLIST, DUMPLOC, DNTMGOTO, DNLSTCOD) set to a
 * random word or to a list word from elsewhere, A overflowed, the bank or
 * the interrupt state wrong. The faithful routines are certified against
 * the original program (c64-port/tests/test_lm_downtelemetry.c and
 * test_cm_downtelemetry.c), as the readable ones are by make
 * downtelemetry-cases' first step.
 *
 * First, the readable downlists (src/downtelemetry/downlink_lists.c)
 * against the assembled listing's words, which the faithful port's data
 * holds: one unbroken run of words from LOWIDCOD, each the listing's, with
 * no listing word in the run left out, ending with DNTABLE, whose words
 * point at control lists in the run. */
#include <stdio.h>
#include <string.h>
#include "agc_transfer.h"
#if LUMINARY099
#include "lm_downtelemetry.h"
#include "lm_downtelemetry_data.inc"
typedef lm_dntm_io dntm_io;
#define MISSION     "LM"
#define TABLE       lm_downtelemetry_routines
#define FAITHFUL    faithful_lm_downtelemetry_routines
#define DATA        lm_dntm_data
#define DATA_COUNT  LM_DNTM_DATA_COUNT
#define LISTS       6u
#define DODOWNTM    LM_DNTM_DODOWNTM
#define DNPHASE1    LM_DNTM_DNPHASE1
#define DNDUMPI     LM_DNTM_DNDUMPI
#define RESUME      LM_DNTM_RESUME
#define CONTROL(io) ((io)->control)
#define CH(io, n)   ((io)->channel##n)
#else
#include "cm_downtelemetry.h"
#include "cm_downtelemetry_data.inc"
typedef cm_dntm_io dntm_io;
#define MISSION    "CM"
#define TABLE      cm_downtelemetry_routines
#define FAITHFUL   faithful_cm_downtelemetry_routines
#define DATA       cm_dntm_data
#define DATA_COUNT CM_DNTM_DATA_COUNT
#define LISTS      5u
#define DODOWNTM   CM_DNTM_DODOWNTM
#define DNPHASE1   CM_DNTM_DNPHASE1
#define DNDUMPI    CM_DNTM_DNDUMPI
#define RESUME     CM_DNTM_RESUME
#define CONTROL(x) ((x)->io)
#define CH(x, n)   ((x)->io.channel##n)
#endif

extern const agc_routine_table TABLE, FAITHFUL;

#define SEQUENCES 2000u
#define DOWNRUPTS 1200u
#define DNLSTCOD  0332u
#define DUMPCNT   0333u
#define CTLIST    0334u
#define DNTMGOTO  0335u
#define DNECADR   0336u
#define SUBLIST   0337u
#define BANK05    012000u

static uint32_t seed = 1969u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static uint16_t word(void)
{
    return (uint16_t)((rnd() ^ (rnd() << 3)) & 077777u);
}

/* A word that means something here: a list word, or the address of one
 * (as a list pointer holds it), or a random word. */
static uint16_t list_word(void)
{
    const agc_site_operand *d = &DATA[rnd() % DATA_COUNT];
    switch (rnd() % 4u) {
    case 0: return (uint16_t)d->word;
    case 1: return (uint16_t)(02000u + d->physical % 1024u);
    case 2: return (uint16_t)((02000u + d->physical % 1024u) ^ 077777u);
    default: return word();
    }
}

static uint16_t sign_extended(uint16_t w)
{
    return (uint16_t)(w | ((w & 040000u) << 1));
}

/* A fresh erasable memory, sending a downlist or the dump. */
static void begin(apollo_overlay_state *s, dntm_io *io)
{
    static const dntm_io none;
    unsigned i;
    for (i = 0; i < APOLLO_SHARED_WORDS; ++i) s->words[i] = word();
    s->words[0] = sign_extended(s->words[0]);
    s->words[1] = sign_extended(s->words[1]);
    s->words[7] = 0;
    *io = none;
    s->words[DNLSTCOD] = (uint16_t)(rnd() % LISTS);
    if (rnd() % 6u == 0u) {
        static const uint16_t counts[] = {004000u, 010000u, 020000u};
        s->words[DNTMGOTO] = DNDUMPI;
        s->words[DUMPCNT] = counts[rnd() % 3u];
    } else
        s->words[DNTMGOTO] = DNPHASE1;
}

/* A DOWNRUPT: the interrupted BBANK in A, the lead-in's bank 05. */
static void downrupt(apollo_overlay_state *s, dntm_io *io)
{
    uint16_t bb = (uint16_t)(((rnd() % 040u) << 10) | (rnd() & 7u));
    s->words[0] = rnd() % 64u ? sign_extended(bb) : (uint16_t)(rnd() | (rnd() << 1));
    s->words[2] = word();
    s->words[3] = 0;
    s->words[4] = s->words[6] = rnd() % 64u ? BANK05 : (uint16_t)(rnd() & 076000u);
    s->words[5] = DODOWNTM;
    CONTROL(io).interrupt_enabled = rnd() % 128u ? 0u : (uint16_t)(rnd() % 3u);
    CONTROL(io).superbank = rnd() % 128u ? (uint16_t)(rnd() & 0160u) : rnd();
    CH(io, 11) = word();
    CH(io, 12) = word();
    CH(io, 13) = (uint16_t)(word() & 043777u);
    CH(io, 14) = (uint16_t)(word() & 037u);
    io->channel30 = word();
    io->channel31 = word();
    io->channel32 = word();
    io->channel33 = word();
}

/* One thing out of place. */
static void disturb(apollo_overlay_state *s, dntm_io *io)
{
    static const uint16_t places[] = {DNLSTCOD, DUMPCNT, CTLIST, DNTMGOTO, DNECADR, SUBLIST};
    switch (rnd() % 8u) {
    case 0: s->words[0] = (uint16_t)(rnd() | (rnd() << 1)); break;
    case 1: s->words[1] = sign_extended(word()); break;
    case 2: CH(io, 13) = word(); break;
    case 3: s->words[DNLSTCOD] = (uint16_t)(rnd() % 8u); break;
    default: s->words[places[rnd() % 6u]] = list_word(); break;
    }
}

static agc_routine find(const agc_routine_table *t, uint16_t key)
{
    uint16_t i;
    for (i = 0; i < t->count; ++i)
        if (t->entries[i].physical == key) return t->entries[i].call;
    return 0;
}

/* A list word or DUMPLOC at the last word of an E-bank (which the listing
 * rules out, and only a disturbed one is): the routine reads the double
 * word at 01777, whose second word is fixed address 02000. The faithful
 * routine reads erasable 0 there (A); the readable one reads the rope:
 * in the CM the word there (LOWIDCOD), in the LM none it knows, so it
 * refuses. */
extern bool downtelemetry_read_02000;

bool downlink_list_word(uint16_t address, uint16_t *value);

static bool listing_word(uint16_t address, uint16_t *value)
{
    agc_word w;
    if (!agc_site_lookup(DATA, DATA_COUNT, (uint16_t)(5u * 1024u + address - 02000u), &w))
        return false;
    *value = (uint16_t)w;
    return true;
}

/* The readable downlists word by word against the listing's. */
static unsigned check_lists(void)
{
    uint16_t address, first = 0, last = 0, value, listed = 0;
    unsigned failures = 0, words = 0, i;

    for (address = 02000u; address < 04000u; ++address) {
        if (!downlink_list_word(address, &value)) continue;
        if (!first) first = address;
        if (last && address != last + 1u) {
            printf("FAIL: the lists break at 05,%04o\n", address);
            ++failures;
        }
        last = address;
        ++words;
        if (!listing_word(address, &listed) || listed != value) {
            printf("FAIL: list word 05,%04o is %05o, the listing's %05o\n", address, value, listed);
            ++failures;
        }
    }
    if (!first || !listing_word(first, &value) || value != 077340u) {
        printf("FAIL: the lists do not start at LOWIDCOD\n");
        ++failures;
    }
    for (i = 0; first && i < LISTS; ++i) {
        uint16_t list = 0;
        downlink_list_word((uint16_t)(last - i), &list);
        if (list < first || list > last - LISTS) {
            printf("FAIL: DNTABLE word %u points at %04o\n", LISTS - 1u - i, list);
            ++failures;
        }
    }
    printf("downtelemetry-cases %s: %u downlist words 05,%04o-%04o, %u failures\n", MISSION, words,
           first, last, failures);
    return failures;
}

static const char *where(uint16_t key)
{
    static char text[24];
    if (key >= AGC_TRANSFER_ERASABLE_BASE)
        sprintf(text, "E %04o", (unsigned)(key - AGC_TRANSFER_ERASABLE_BASE));
    else
        sprintf(text, "%02o,%04o", (unsigned)(key / 1024u), (unsigned)(02000u + key % 1024u));
    return text;
}

static unsigned ran[16], refused[16], tried[16];

int main(void)
{
    agc_native_program program = {0, 1, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};
    unsigned n, k, i, checks = 0, failures = 0, departs = 0;
    apollo_overlay_state s;
    dntm_io io;

    if (TABLE.count != FAITHFUL.count || TABLE.count > 16u) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    for (i = 0; i < TABLE.count; ++i)
        if (TABLE.entries[i].physical != FAITHFUL.entries[i].physical) {
            printf("FAIL: entry %u has key %u, faithful %u\n", i, TABLE.entries[i].physical,
                   FAITHFUL.entries[i].physical);
            return 1;
        }

    failures += check_lists();
    for (n = 0; n < SEQUENCES; ++n) {
        begin(&s, &io);
        for (k = 0; k < DOWNRUPTS; ++k) {
            unsigned step;
            downrupt(&s, &io);
            for (step = 0; step < 8u; ++step) {
                apollo_overlay_state a, b, start;
                dntm_io ia, ib, istart;
                enum exec_status sa, sb;
                const char *why = 0;
                uint16_t key;
                agc_routine f, r;

                if (rnd() % 24u == 0u) disturb(&s, &io);
                if (s.words[5] == RESUME) break;
                key = agc_transfer_point(&program, &s, CONTROL(&io).superbank);
                f = find(&FAITHFUL, key);
                r = find(&TABLE, key);
                if (!f || !r) break;
                for (i = 0; FAITHFUL.entries[i].physical != key; ++i) {
                }
                a = b = start = s;
                ia = ib = istart = io;
                sa = f(&program, &a, &ia);
                downtelemetry_read_02000 = false;
                sb = r(&program, &b, &ib);
                ++checks;
                ++tried[i];
                if (sa == EXEC_OK) ++ran[i];
                if (sb != EXEC_OK) {
                    ++refused[i];
                    if (memcmp(&b, &start, sizeof b) || memcmp(&ib, &istart, sizeof ib))
                        why = "readable refusal changed the state";
                    else if (sa == EXEC_OK && downtelemetry_read_02000)
                        ++departs;
                    else if (sa == EXEC_OK)
                        why = "readable refused, faithful ran";
                } else if (sa == EXEC_OK &&
                           (memcmp(&a, &b, sizeof a) || memcmp(&ia, &ib, sizeof ia))) {
                    if (downtelemetry_read_02000)
                        ++departs;
                    else
                        why = "different";
                }
                if (why && ++failures <= 8u) {
                    unsigned w;
                    printf("FAIL %s, sequence %u downrupt %u: %s (status %d, faithful %d)\n",
                           where(key), n, k, why, (int)sb, (int)sa);
                    for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                        if (a.words[w] != b.words[w])
                            printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, b.words[w],
                                   a.words[w], start.words[w]);
                    if (memcmp(&ia, &ib, sizeof ia)) printf("  channels differ\n");
                }
                if (sa == EXEC_OK) {
                    s = a;
                    io = ia;
                } else if (sb == EXEC_OK) {
                    s = b;
                    io = ib;
                } else {
                    /* As a fresh start would: the downlist from its start. */
                    s.words[DNTMGOTO] = DNPHASE1;
                    s.words[DNLSTCOD] = (uint16_t)(rnd() % LISTS);
                    break;
                }
            }
        }
    }
    for (i = 0; i < TABLE.count; ++i) {
        printf("%s down-telemetry entry %s: %u steps, %u run by the faithful routine, %u refused "
               "by the readable one\n",
               MISSION, where(FAITHFUL.entries[i].physical), tried[i], ran[i], refused[i]);
        if (ran[i] == 0u) {
            printf("FAIL: entry %s never run\n", where(FAITHFUL.entries[i].physical));
            ++failures;
        }
    }
    printf(
        "downtelemetry-cases %s: %u checks (%u where the faithful routine departs from the AGC), "
        "%u failures\n",
        MISSION, checks, departs, failures);
    return failures != 0;
}
