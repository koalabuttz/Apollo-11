/* SPDX-License-Identifier: MIT */
/* Which words of interpretive code the interpreter ran: the recorder of the
 * word measure (tests/word_coverage.py).
 *
 * Interpretive code is tables of words, which hold no line of C, so the
 * line measure (tests/line_coverage.py) cannot see which of them ran. This
 * file is compiled into src/binding/interpreter_core.c in that measure's
 * build only (WORD_COVERAGE, which tests/coverage_cc.py sets beside clang's
 * counters): every pass of the interpreter, from a case test or from a
 * scenario, goes through that file's run(), and its few calls of the
 * functions here say what the pass did:
 *
 *   words_read()   a word the interpreter's own code read from fixed
 *                  memory, by its physical address (the binding's
 *                  rope_word(), which every such read goes through);
 *   words_takes()  the next word read is the one NEWOPS takes: an order
 *                  pair, whose left order runs now, or a store code;
 *   words_right()  the pair's right order runs now (EDOP holds it);
 *   words_at()     the point the pass is at, which says how the order
 *                  that was running ended: at DANZIG or NOIBNKSW it went
 *                  on to the next order, anywhere else it went away (a
 *                  branch taken, a GOTO, a CALL, an EXIT, an alarm);
 *   words_begin(), words_end()
 *                  a pass's records count only if the pass ends well: a
 *                  pass the binding refuses is put back as it was found,
 *                  and so is what was noted of it.
 *
 * A word read just after the word NEWOPS took (at most four words on from
 * a pair, whose two orders take two each at most; two from a store code)
 * is marked as read in line: one of that word's operands, and not a word
 * some made-up address came to.
 *
 * What is kept for each word is its value as read and what was seen of it
 * (the flags below). A word read with two values (a test that writes
 * made-up programs into the rope) is marked mixed, and the measure counts
 * none of what was seen there in that program. The measure also holds
 * each value to the rope's own word, so a made-up word counts for nothing.
 *
 * A right order whose pair this recorder did not see taken (a case that
 * starts at DANZIG with an order waiting in EDOP) is an orphan. Its pair is
 * looked for behind LOC (words_right(), below) and the order noted there
 * apart from the others (ORPHAN_RAN and its two outcomes), for the measure
 * to take where the listing has a pair. One whose pair is not found is
 * noted nowhere: the measure reports how many, and where the first
 * WORDS_LOST different ones were (LOC's word and EDOP).
 *
 * At its exit a program writes what it saw to a file of its own under
 * build/linecov/words, named as its counts are (clang's
 * __llvm_profile_filename, which tests/coverage_cc.py gives each program)
 * with the mission of this build of the binding: a test of both missions
 * holds two of these recorders and writes two files. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if LUMINARY099
#define WORDS_MISSION "lm"
#else
#define WORDS_MISSION "cm"
#endif

enum {
    WORD_READ = 1,
    LEFT_RAN = 2,
    LEFT_ON = 4,
    LEFT_AWAY = 8,
    RIGHT_RAN = 16,
    RIGHT_ON = 32,
    RIGHT_AWAY = 64,
    STORE_RAN = 128,
    WORD_MIXED = 256,
    ORPHAN_RAN = 512,
    ORPHAN_ON = 1024,
    ORPHAN_AWAY = 2048,
    READ_IN_LINE = 4096
};

#define WORDS_IN_ROPE (044u * 1024u)
#define WORDS_A_PASS  4096u
#define WORDS_NO_PAIR 0xFFFFFFFFu
#define WORDS_LOST    64u

static uint16_t words_value[WORDS_IN_ROPE], words_flags[WORDS_IN_ROPE];
static unsigned words_orphans;

/* The orphans whose pair was not found, by where LOC was and what waited
 * in EDOP: the first WORDS_LOST different ones, each with how often. */
static struct words_lost {
    unsigned at, edop, times;
} words_lost[WORDS_LOST];
static unsigned words_lost_count;

/* The pass under way: what it noted, and whether it is under way at all
 * (a read outside a pass is another binding's, and is not noted). */
static struct {
    unsigned at;
    uint16_t value, flags;
} words_noted[WORDS_A_PASS];
static unsigned words_count, words_orphans_noted;
static struct words_lost words_lost_noted[WORDS_LOST];
static unsigned words_lost_noted_count;
static bool words_in_pass;

/* The pair NEWOPS last took, the order of it that has not ended yet
 * (LEFT_RAN, RIGHT_RAN or 0), and what the next read is to be marked. They
 * live from pass to pass: a pass can end in the middle of a pair. So that
 * one pass's pair is not taken for another's, the next pass goes on with the
 * order only if it begins where this one stopped (Z, LOC and BANKSET as
 * words_end() found them), and a right order is the kept pair's only if LOC
 * is still at the pair or its operands (words_right()).
 *
 * taken_at is the word NEWOPS last took, a pair or a store code
 * (WORDS_NO_PAIR: none), and taken_span how far its operands can lie from
 * it: a word read that near after it is one of its operands in line
 * (READ_IN_LINE). It is kept into the next pass while LOC and BANKSET are
 * as this one left them. Z may differ: after a left order RTB the pass
 * ends at the RTB's routine, basic code, and the next begins at DANZIG
 * with the pair's right order still to read its operand. A right order
 * that runs sets it too, to the pair words_right() finds for it: a case
 * can begin at DANZIG, with a pair this recorder never saw taken. */
static struct words_state {
    unsigned pair_at, pair_word, running, next, taken_at, taken_span;
    unsigned z, loc, bankset;
} words_now = {WORDS_NO_PAIR, 0u, 0u, 0u, WORDS_NO_PAIR, 0u, 0u, 0u, 0u}, words_before;

static void words_note(unsigned at, unsigned value, unsigned flags)
{
    if (at >= WORDS_IN_ROPE) return;
    if (words_count == WORDS_A_PASS) {
        fprintf(stderr, "word_coverage: a pass noted more than %u words\n", WORDS_A_PASS);
        abort();
    }
    words_noted[words_count].at = at;
    words_noted[words_count].value = (uint16_t)value;
    words_noted[words_count].flags = (uint16_t)flags;
    ++words_count;
}

static word words_read(unsigned at, word value)
{
    unsigned flags = WORD_READ | words_now.next;

    if (!words_in_pass) return value;
    if (!words_now.next && words_now.taken_at != WORDS_NO_PAIR && at > words_now.taken_at &&
        at <= words_now.taken_at + words_now.taken_span)
        flags |= READ_IN_LINE;
    words_note(at, value, flags);
    if (words_now.next) {
        words_now.taken_at = at;
        words_now.taken_span = words_now.next == STORE_RAN ? 2u : 4u;
    }
    if (words_now.next == LEFT_RAN) {
        words_now.pair_at = at;
        words_now.pair_word = value;
        words_now.running = LEFT_RAN;
    }
    words_now.next = 0u;
    return value;
}

static void words_takes(unsigned what)
{
    words_now.next = what;
    words_now.running = 0u;
}

/* How the order that was running ended, at the point the pass has come
 * to. At a point in the middle of an order it has not ended. */
static void words_ended(word z, bool pass_over)
{
    unsigned how;

    if (!words_now.running) return;
    if (z == DANZIG || z == NOIBNKSW) {
        how = words_now.running << 1; /* on */
    } else if (z == NEWOPS ||
               (pass_over && z != DIRADRES && z != DODLOAD && z != DOVLOAD && z != DODLOAD_STAR &&
                z != DOVLOAD_STAR && z != FIFTEEN_BITADR && z != GOTO)) {
        how = words_now.running << 2; /* away */
    } else {
        return;
    }
    words_note(words_now.pair_at, words_now.pair_word, how);
    words_now.running = 0u;
}

static void words_at(word z)
{
    words_ended(z, false);
}

/* What NEWOPS leaves in EDOP of a pair: its bits 8-14, of the word's
 * complement less one (the right order's code and one). */
static unsigned words_edop_of(unsigned pair)
{
    return (((~pair & 077777u) - 1u) >> 7) & 0177u;
}

static word rope_word(unsigned physical);

/* The word `back` words before the one LOC is at, in the program's bank
 * (as the binding's code_word() finds LOC's): where it is, and its value.
 * Not a read of the interpreter's, and not noted as one. */
static bool words_behind(unsigned back, unsigned *at, word *value)
{
    unsigned address = (LOC - back) & 077777u, bank;

    if (address >= 04000u) {
        *at = 2048u + (address & 07777u) - 04000u;
    } else {
        if (address < 02000u) return false;
        bank = (BANKSET >> 10) & 037u;
        if (bank >= 030u && (*agc.superbank & 0100u)) bank += 010u;
        *at = bank * 1024u + (address & 01777u);
    }
    words_in_pass = false;
    *value = rope_word(*at);
    words_in_pass = true;
    return true;
}

/* Whether LOC is at the pair this recorder last saw taken, or at one of
 * the two words after it. */
static bool words_at_pair(void)
{
    unsigned at;
    word value;

    if (words_now.pair_at == WORDS_NO_PAIR || !words_behind(0u, &at, &value)) return false;
    return at >= words_now.pair_at && at <= words_now.pair_at + 2u;
}

/* The pair's right order runs now. LOC is at the last word the left order
 * took, the pair itself or one of its operands (two at most). The pair is
 * the one this recorder saw taken if that pair's right order is the one
 * waiting and LOC is there; if not (an orphan), it is the nearest word at
 * or behind LOC that reads as a pair with this right order. */
static void words_right(void)
{
    unsigned back, at;
    word value;

    if (words_at_pair() && words_edop_of(words_now.pair_word) == EDOP) {
        words_note(words_now.pair_at, words_now.pair_word, RIGHT_RAN);
        words_now.running = RIGHT_RAN;
        words_now.taken_at = words_now.pair_at;
        words_now.taken_span = 4u;
        return;
    }
    for (back = 0u; back < 3u; ++back) {
        if (words_behind(back, &at, &value) && (value & 040000u) && words_edop_of(value) == EDOP) {
            words_now.pair_at = at;
            words_now.pair_word = value;
            words_note(at, value, ORPHAN_RAN);
            words_now.running = ORPHAN_RAN;
            words_now.taken_at = at;
            words_now.taken_span = 4u;
            return;
        }
    }
    ++words_orphans_noted;
    if (words_lost_noted_count < WORDS_LOST) {
        words_lost_noted[words_lost_noted_count].at = words_behind(0u, &at, &value) ? at : 0u;
        words_lost_noted[words_lost_noted_count].edop = EDOP;
        ++words_lost_noted_count;
    }
    words_now.running = 0u;
}

static void words_write(void)
{
    extern const char __llvm_profile_filename[] __attribute__((weak));
    static const char profiles[] = "/profiles/";
    char name[1024];
    const char *base = __llvm_profile_filename, *cut, *end;
    unsigned at, tries, salt = (unsigned)time(NULL) ^ (unsigned)clock() ^ (unsigned)(size_t)&at;
    FILE *out = NULL;

    if (!base || !(cut = strstr(base, profiles)) || !(end = strchr(cut, '%'))) return;
    for (tries = 0u; !out && tries < 1000u; ++tries) {
        salt = salt * 1103515245u + 12345u;
        if (snprintf(name, sizeof name, "%.*s/words/%.*s%s.%08x.words", (int)(cut - base), base,
                     (int)(end - cut - (int)sizeof profiles + 1), cut + sizeof profiles - 1,
                     WORDS_MISSION, salt) >= (int)sizeof name)
            return;
        out = fopen(name, "wx");
    }
    if (!out) {
        fprintf(stderr, "word_coverage: cannot write %s\n", name);
        return;
    }
    fprintf(out, "mission %s\norphans %u\n", WORDS_MISSION, words_orphans);
    for (at = 0u; at < words_lost_count; ++at)
        fprintf(out, "lost %05o %03o %u\n", words_lost[at].at, words_lost[at].edop,
                words_lost[at].times);
    for (at = 0u; at < WORDS_IN_ROPE; ++at)
        if (words_flags[at]) fprintf(out, "%05o %05o %03x\n", at, words_value[at], words_flags[at]);
    fclose(out);
}

static void words_begin(void)
{
    words_before = words_now;
    if (Z != words_now.z || LOC != words_now.loc || BANKSET != words_now.bankset)
        words_now.running = 0u; /* not the pass that stopped in that order */
    if (LOC != words_now.loc || BANKSET != words_now.bankset) words_now.taken_at = WORDS_NO_PAIR;
    words_count = 0u;
    words_orphans_noted = 0u;
    words_lost_noted_count = 0u;
    words_in_pass = true;
}

static void words_end(enum exec_status status)
{
    static bool writes;
    unsigned i, k;

    words_in_pass = false;
    if (status != EXEC_OK) {
        words_now = words_before;
        return;
    }
    words_now.next = 0u;
    words_ended(Z, true);
    words_now.z = Z;
    words_now.loc = LOC;
    words_now.bankset = BANKSET;
    if (!writes) {
        writes = true;
        atexit(words_write);
    }
    words_orphans += words_orphans_noted;
    for (i = 0u; i < words_lost_noted_count; ++i) {
        for (k = 0u; k < words_lost_count; ++k)
            if (words_lost[k].at == words_lost_noted[i].at &&
                words_lost[k].edop == words_lost_noted[i].edop)
                break;
        if (k == WORDS_LOST) continue;
        if (k == words_lost_count) {
            words_lost[k] = words_lost_noted[i];
            words_lost[k].times = 0u;
            ++words_lost_count;
        }
        ++words_lost[k].times;
    }
    for (i = 0u; i < words_count; ++i) {
        unsigned at = words_noted[i].at;

        if (words_flags[at] && words_value[at] != words_noted[i].value)
            words_flags[at] |= WORD_MIXED;
        if (!words_flags[at]) words_value[at] = words_noted[i].value;
        words_flags[at] |= words_noted[i].flags;
    }
}
