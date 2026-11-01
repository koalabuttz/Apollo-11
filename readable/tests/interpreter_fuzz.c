/* SPDX-License-Identifier: MIT */
/* The readable interpreter core against the faithful one, on random
 * interpretive programs (make interpreter-cases).
 *
 * The programs and machine states are the faithful port's own fuzz's
 * (c64-port/tools/interp_core_fuzz.c, included here for its generator:
 * random interpretive programs written into bank 20 of the rope, random
 * images in the work area's frame). Each case runs pass after pass through
 * both cores from the same state, the faithful core (built with faithful_
 * in front of its names) and the readable one: each pass must end with the
 * same status, Z and all 2048 words, and the same channels.
 *
 * Unlike that fuzz's images, a case starts with the banks as the
 * interpreter leaves them at NEWOPS (FB the program's bank, which
 * BANKSET names): the faithful core reads the program through BANKSET
 * whatever FB holds, the AGC (and the readable code) through FB.
 *
 * The generator also writes operands no assembled program has, where the
 * readable code does what the AGC does and the faithful core something
 * simpler, or where an order rewrites the interpreter's own registers
 * under it. A pass that would run one is not compared ("unassembled
 * operands", see unassembled() below): an erasable order given a fixed or
 * 15-bit address, a store into fixed memory, an operand that runs past
 * the end of its bank or names the interpreter's own registers, an
 * address word of +0 or 46D, a switch bit past BIT1, a switch word past
 * the last flag word, a CGOTO list outside the program's bank. Nor is one that
 * writes NEWJOB, or that takes both images out of the work area's frame
 * ("left the frame"); a pass that takes only one of them out is a
 * difference.
 *
 * The run fails on a difference, and also when too little was compared to
 * mean anything: a crash in either core, more than a few cases abandoned
 * (a case that runs without end), or, in a run of a thousand cases or
 * more, fewer passes compared or more cases with none compared than the
 * measured rates allow (MIN_PASSES, MAX_EMPTY), or more than a quarter of
 * the cases leaving the frame. FUZZ_FIRST=n starts at case n (to rerun a
 * failing case: the counts and floors are for the cases run).
 *
 *   interpreter-cases-<m> <lm.bin> <cm.bin> [cases] [seed]
 *
 * Built once per mission. */
#include "interp_runtime.h"

extern const agc_routine faithful_lm_interp_core, faithful_cm_interp_core;

/* The fuzz's own main, and the other mission's interpreters: unused. */
static enum exec_status unused_core(const agc_native_program *p, apollo_overlay_state *s, void *io)
{
    (void)p, (void)s, (void)io;
    return EXEC_OUT_OF_SCOPE;
}
#define main              fuzz_main
#define lm_interp_runtime unused_core
#define cm_interp_runtime unused_core
#if LUMINARY099
#define cm_interp_core unused_core
#else
#define lm_interp_core unused_core
#endif
#include "../../c64-port/tools/interp_core_fuzz.c"
#undef main
#undef lm_interp_core
#undef cm_interp_core
#undef a
#undef b

#if LUMINARY099
#define MISSION      0u
#define FAITHFUL     faithful_lm_interp_core
#define READABLE     lm_interp_core
#define READABLE_MCT lm_interp_core_mct
typedef p66_bias_io mission_io;
#else
#define MISSION      1u
#define FAITHFUL     faithful_cm_interp_core
#define READABLE     cm_interp_core
#define READABLE_MCT cm_interp_core_mct
typedef cm_native_io mission_io;
#endif

/* The word off words after LOC in the program's bank. */
static unsigned code_at(const apollo_overlay_state *s, unsigned off)
{
    agc_word w = 0;
    (void)apollo_rope_read(
        MISSION,
        (uint16_t)(((s->words[0165] >> 10) & 037u) * 1024u + ((s->words[0164] + off) & 01777u)),
        &w);
    return w & 077777u;
}

/* Whether order code c names an erasable word by a 15-bit address: LXA,
 * LXC, SXA, XCHX, XAD, XSU and ITA. */
static int erasable_misc(unsigned c)
{
    unsigned index = c >> 3;
    if ((c & 3u) != 2u) return 0;
    return (index >= 002u && index <= 005u) || index == 010u || index == 011u ||
           (index == 015u && (c & 4u));
}

/* Whether a is one of the interpreter's own registers that no program
 * names as an operand (NEWJOB, INTBIT15 to OVFIND, MODE to PRIORITY).
 * MPAC is not one of them: flight code names it (ORBITAL_INTEGRATION's
 * SXA,1 and LXA,1 MPAC +1 at NBRANCH, the LM's AGS_INITIALIZATION's SXA,1
 * MPAC +1, +4 and +6), so operands there are compared. */
static int interpreter_register(unsigned a)
{
    return (a >= 061u && a <= 067u) || (a >= 0107u && a <= 0121u) || (a >= 0163u && a <= 0170u);
}

/* Whether an operand (up to a matrix, 18 words) at address a runs past the
 * end of its bank: erasable is switched in 256-word banks (the interpreter
 * leaves an erasable operand in 1400-1777 under EBANK), fixed in 1K ones.
 * The AGC goes on into the next bank, the faithful core wraps; no program
 * places an operand so. Nor does one name the interpreter's own registers
 * (NEWJOB, INTBIT15 to OVFIND, MODE to PRIORITY, or a vector's length
 * before them). */
static int past_bank_end(unsigned a)
{
    if (interpreter_register(a) || a >= 040000u) return 1;
    if (a <= 055u) return 0; /* in the work area (to 45D: the address less one) */
    return a < 04000u ? (a & 0377u) > 0400u - 18u : (a & 01777u) > 02000u - 18u;
}

/* The same for address word w, direct or indexed by either index register. */
static int near_bank_end(const apollo_overlay_state *s, unsigned w, int indexed)
{
    unsigned fixloc = s->words[0120], i;

    /* (DIRADRES's CCS leaves the address less one, the order reading the
     * word after: "THIS METHOD PRECLUDES USE OF THE LAST LOCATION IN EACH
     * FBANK", and an erasable bank's first word is out of reach too.) */
    if (past_bank_end(w) || (w > 055u && w < 040000u && past_bank_end(w - 1u))) return 1;
    /* (46D, less one, is 45D: DIRADRES's AD -ENDVAC leaves +0, and GEADDR
     * makes that address 0 of the bank.) */
    if (!indexed && w == 056u) return 1;
    if (!indexed) return 0;
    if (w >= 040000u) w = ~w & 077777u; /* X2: "ADDRESS STORED COMPLEMENTED" */
    for (i = 0; i < 2u; ++i) {
        unsigned x = s->words[(fixloc + 046u + i) & 03777u];
        unsigned a = (w + (~x & 077777u)) & 077777u, j;         /* w - X, ones' complement */
        for (j = 0; j < 2u; ++j, a = (a + 077776u) % 077777u) { /* and one less */
            if (a > 077777u - 01000u) return 1;                 /* negative */
            if (a >= 0100u && past_bank_end(a)) return 1;
        }
    }
    return 0;
}

/* The last flag word, STATE + n: Luminary099 has 16 (STATE ERASE +15D),
 * Comanche055 12 (+11D). A switch word's bits 9 up number its flag word
 * ("UP TO 64 SWITCH WORDS"); past the last one, STATE + n is another
 * erasable word, from 070 the interpreter's LOC and BANKSET. */
#define LAST_FLAGWORD (MISSION ? 11u : 15u)

/* Whether order code c stores into its operand, which must be erasable:
 * SSP and TSLC. */
static int stores_operand(unsigned c)
{
    return (c & 1u) && ((c >> 2) == 011u || (c >> 2) == 017u);
}

/* The same for a store code (less one, as NEWOPS's CCS leaves it), and
 * for STODL's or STOVL's address in the word at off. */
static int store_unassembled(const apollo_overlay_state *s, unsigned code, unsigned off)
{
    unsigned w;

    if (code == 077777u || near_bank_end(s, code & 03777u, 1)) return 1;
    if ((code >> 11) < 3u || (code >> 11) == 7u) return 0;
    w = code_at(s, off);
    return w == 0u || (w < 040000u && near_bank_end(s, w, 1));
}

/* Whether the pass about to run gives an order an operand no program
 * assembles: an erasable address of 4000 or more for an order that takes
 * an erasable word by a 15-bit address (the AGC's INDEX would then change
 * the instruction itself; the faithful core keeps the low bits), a fixed
 * one for an order that stores into its operand (the AGC's store goes
 * nowhere; the faithful core's into erasable), one that may run past a
 * bank's end, or an address word of +0 (DIRADRES's CCS takes it as no
 * address word at all; the faithful core as address 0). */
static int unassembled(const apollo_overlay_state *s)
{
    unsigned z = s->words[5], c[2], n = 0, i, off = 1u, w;
    int indexes_changed = 0;

    if (z == (MISSION ? 06037u : 06070u)) {
        w = code_at(s, 0u);
        if (w == 0u || w == 077777u) return 1; /* "never +0", and no pair is -0 */
        if (!(w & 040000u)) return store_unassembled(s, w - 1u, 1u);
        w = (~w & 077777u) - 1u; /* the left code, and the right code + 1 */
        c[n++] = w & 0177u;
        if (w >> 7) c[n++] = (w >> 7) - 1u;
    } else if (s->words[023] != 0u) {
        c[n++] = s->words[023] - 1u;
    }
    for (i = 0; i < n; ++i) {
        if (c[i] == 0150u) { /* STADR: the store code that follows, complemented */
            w = code_at(s, off++);
            /* ITR1's CS and AD NEGONE, in ones' complement: a complemented
             * word of 1 gives -0, the code no store word gives. */
            w = (~w & 077777u) + 077776u;
            w = w > 077777u ? (w & 077777u) + 1u : w;
            if (indexes_changed || store_unassembled(s, w, off)) return 1;
            continue;
        }
        if (!(c[i] & 3u)) continue;
        w = code_at(s, off);
        if (erasable_misc(c[i]) && (w >= 04000u || interpreter_register(w))) return 1;
        if ((c[i] & 3u) == 2u && (c[i] >> 3) == 016u && (w & 017u) == 017u)
            return 1; /* switch bit 15 of a word of 15: BIT15 +15 is past BIT1 */
        if ((c[i] & 3u) == 2u && (c[i] >> 3) == 016u && (w >> 8) > LAST_FLAGWORD)
            return 1; /* a switch word past the flag words */
        /* CGOTO reads its list's CADR through FBANK ("INDEX LOC, CA 1") and
         * puts it, plus the modifier, in FBANK ("SELECT GOTO ADDRESS"); the
         * faithful core reads the list from the program's bank. Every
         * program keeps the list there and its modifier in erasable
         * (DIFEQTAB beside its CGOTO, MPAC): a fixed modifier leaves its
         * own bank in FBANK, a list elsewhere names another bank. */
        if ((c[i] & 1u) && (c[i] >> 2) == 004u &&
            (w >= 04000u || ((code_at(s, off + 1u) >> 10) & 037u) != CODE_BANK))
            return 1;
        if ((c[i] & 1u) && (c[i] & 2u ? indexes_changed || near_bank_end(s, w, 1)
                                      : w < 040000u && near_bank_end(s, w, 0)))
            return 1;
        /* (After an index order, an indexed address is past knowing.) */
        if ((c[i] & 3u) == 2u && (c[i] >> 3) <= 011u) indexes_changed = 1;
        if ((c[i] & 1u) && w == 0u) return 1; /* the AGC takes the next word of +0 as no address */
        if (stores_operand(c[i]) && w >= 02000u && w < 040000u) return 1;
        /* (CGOTO and SSP take two words, as does a switch order that
         * branches: its bits 5-6 not 11, "NOOP".) */
        if ((c[i] & 1u) && ((c[i] >> 2) == 004u || (c[i] >> 2) == 011u))
            off += 2u;
        else if ((c[i] & 3u) == 2u && (c[i] >> 3) == 016u && ((w >> 2) & 014u) != 014u)
            off += 2u;
        else
            ++off;
    }
    return 0;
}

/* A crash in either interpreter: counted apart from a hang, and fatal to
 * the run. */
static unsigned crashed;
static void on_crash(int sig)
{
    ++crashed;
    on_alarm(sig);
}

/* Timeouts allowed per run: a case that loops without end is abandoned
 * (interp_core_fuzz.c), which a slow machine may also do to a long one. */
#define MAX_ABANDONED 5u
/* Measured at 20,000 cases (seeds 1 to 64, both missions): 1.21 to 1.32
 * passes compared per case, 42 to 45 cases in a hundred with none. A run
 * that compares fewer than MIN_PASSES per hundred cases, or has more than
 * MAX_EMPTY in a hundred with none, stopped short somewhere. Runs under
 * FLOOR_CASES (a FUZZ_FIRST rerun of a few cases) are too small to judge. */
#define MIN_PASSES  115u
#define MAX_EMPTY   47u
#define FLOOR_CASES 10000u

int main(int argc, char **argv)
{
    static struct {
        apollo_overlay_state s;
        agc_word pad[0200000];
    } fa, ra;
    unsigned cases = argc > 3 ? (unsigned)strtoul(argv[3], 0, 0) : 20000u;
    unsigned seed = argc > 4 ? (unsigned)strtoul(argv[4], 0, 0) : 1u;
    static bool judged;
    static unsigned c, first, ran, slots, refused, hung, unreal, unasm, bad, shown, empty;

    (void)fuzz_main;
    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        fprintf(stderr, "usage: interpreter-cases <lm.bin> <cm.bin> [cases] [seed]\n");
        return 2;
    }
    first = getenv("FUZZ_FIRST") ? (unsigned)atoi(getenv("FUZZ_FIRST")) : 0u;
    if (first > cases) first = cases;
    for (c = first; c < cases; ++c) {
        unsigned slot, compared = slots;
        apollo_overlay_state *f = &fa.s, *r = &ra.s, pre;

        rs = 0x9e3779b97f4a7c15ull ^ ((uint64_t)(seed * 1000003u + c) << 1);
        rnd(2);
        program(&rope[MISSION][CODE_BANK * 1024u]);
        if (sigsetjmp(hang, 1)) {
            ++hung;
            continue;
        }
        signal(SIGALRM, on_alarm);
        signal(SIGSEGV, on_crash);
        arm(300);
        image(f, MISSION);
        /* The banks as at NEWOPS: FB is the program's, BBANK agrees. */
        f->words[4] = (agc_word)(f->words[0165] & 076000u);
        f->words[6] = (agc_word)(f->words[4] | (f->words[3] >> 8));
        *r = *f;
        memset(fa.pad, 0, sizeof fa.pad);
        memset(ra.pad, 0, sizeof ra.pad);
        for (slot = 0; slot < 40u; ++slot) {
            enum exec_status sf, sr;
            mission_io fio, rio;
            unsigned i, diff = 0;
            bool fw, rw;

            /* The fuzz's frame: a program's own words, pointers and MODE. */
            if (!interpreter_point(f, MISSION)) break;
            if (f->words[0166] < f->words[0120] + 4u || f->words[0166] > f->words[0120] + 052u)
                break;
            if (((f->words[0165] >> 10) & 037u) != CODE_BANK) break;
            if (f->words[5] == (MISSION ? 06037u : 06070u) &&
                ((f->words[0164] & 01777u) >= CODE_WORDS || !start[f->words[0164] & 01777u]))
                break;
            if (f->words[0163] > 1u && f->words[0163] != 077776u) break;
            if (unassembled(f)) {
                ++unasm;
                break;
            }
            memset(&fio, 0, sizeof fio);
            fio.interrupt_enabled = 1;
            rio = fio;
            pre = *f;
            sf = FAITHFUL(0, f, &fio);
            READABLE_MCT = 0;
            sr = READABLE(0, r, &rio);
            if (sf == EXEC_OUT_OF_SCOPE) {
                ++refused;
                break;
            }
            /* (A program never writes NEWJOB.) A pass only one side takes
             * out of the frame is a difference. */
            fw = wild(f, &pre) || f->words[067] != pre.words[067];
            rw = wild(r, &pre) || r->words[067] != pre.words[067];
            if (fw && rw) {
                ++unreal;
                break;
            }
            if (fw || rw) {
                ++bad;
                if (shown++ < 12u)
                    fprintf(stderr, "case %u slot %u: only the %s pass left the frame\n", c, slot,
                            fw ? "faithful" : "readable");
                break;
            }
            ++slots;
            for (i = 0; i < 2048u; ++i) diff += f->words[i] != r->words[i];
            if (memcmp(fa.pad, ra.pad, sizeof fa.pad)) ++diff;
            if (sf != sr || diff || memcmp(&fio, &rio, sizeof fio)) {
                ++bad;
                if (getenv("FUZZ_DUMP_PRE") && bad == 1u) {
                    FILE *out = fopen(getenv("FUZZ_DUMP_PRE"), "wb");
                    if (out) {
                        fwrite(&pre, sizeof pre, 1, out);
                        fclose(out);
                    }
                }
                if (shown++ < (getenv("FUZZ_SHOW") ? (unsigned)atoi(getenv("FUZZ_SHOW")) : 12u)) {
                    agc_word w[3] = {0, 0, 0};
                    for (i = 0; i < 3u; ++i)
                        (void)apollo_rope_read(MISSION,
                                               (uint16_t)(((pre.words[0165] >> 10) & 037u) * 1024u +
                                                          ((pre.words[0164] + i) & 01777u)),
                                               &w[i]);
                    fprintf(stderr,
                            "case %u slot %u (Z %05o LOC %05o words %05o %05o %05o EDOP %03o MODE "
                            "%05o): status %d/%d, %u words differ (faithful/readable):",
                            c, slot, pre.words[5], pre.words[0164], w[0], w[1], w[2],
                            pre.words[023], pre.words[0163], sf, sr, diff);
                    fprintf(stderr, " [ADDRWD %05o EB %05o FB %05o]", f->words[0116], f->words[3],
                            f->words[4]);
                    for (i = 0; i < 2048u; ++i)
                        if (f->words[i] != r->words[i])
                            fprintf(stderr, " %04o=%06o/%06o", i, f->words[i], r->words[i]);
                    fprintf(stderr, "\n");
                }
                break;
            }
            if (sf != EXEC_OK) break;
        }
        if (slots == compared) ++empty;
    }
    arm(0);
    ran = cases - first;
    printf("%s: %u cases, %u passes compared, %u cases with none, %u refused, %u abandoned (%u "
           "crashed), %u left the frame, %u unassembled operands, %u differ\n",
           MISSION ? "CM" : "LM", ran, slots, empty, refused, hung, crashed, unreal, unasm, bad);
    /* Cases the comparison never sees must not hide a failure: no crash, a
     * few timeouts at most (a slow machine), and, in a run large enough to
     * judge, enough passes compared, few cases with none, and most cases
     * staying in the frame. */
    judged = ran >= FLOOR_CASES;
    if (crashed || hung > MAX_ABANDONED ||
        (judged && (slots < ran / 100u * MIN_PASSES || empty > ran / 100u * MAX_EMPTY ||
                    unreal > ran / 4u))) {
        printf("FAIL: %u crashed, %u abandoned (at most %u), %u passes compared (at least %u), "
               "%u cases with none (at most %u), %u left the frame (at most %u)\n",
               crashed, hung, MAX_ABANDONED, slots, ran / 100u * MIN_PASSES, empty,
               ran / 100u * MAX_EMPTY, unreal, ran / 4u);
        return 1;
    }
    return bad ? 1 : 0;
}
