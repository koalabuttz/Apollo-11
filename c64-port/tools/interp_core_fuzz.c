/* SPDX-License-Identifier: MIT */
/* Fuzz of the compact interpreter (src/lm_interp_core.c) against the
 * verification interpreter (src/lm_interp_runtime.c, whose order bodies are
 * checked against yaAGC body by body): random interpretive programs written
 * into a copy of the mission's rope, random images, and every NEWOPS slot
 * compared, Z and all 2048 words, the status and the io record.
 *
 *   interp_core_fuzz <lm.bin> <cm.bin> [cases] [seed] [mission 0/1/2=both]
 *
 * The rope image here is this program's own (it defines apollo_rope_read), so
 * a bank of it can hold the generated program. A case stops at the first slot
 * the verification interpreter refuses, or when Z leaves the interpreter.
 *
 * FUZZ_DUMP=<file> writes each slot that compared equal as a record for
 * tools/interp_core_cycles.c (the image before it and the rope words it
 * read); FUZZ_SINGLE=1 generates one order per word (no right orders), so a
 * slot is one order. FUZZ_CASE=<n> starts at case n with a slot trace,
 * FUZZ_SHOW=<n> shows n differences, FUZZ_SELF/FUZZ_CORE2 compare an
 * interpreter with itself. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "interp_runtime.h"
#include "interp_runtime_tables.h"
#include "rope_image.h"
#include "cm_native_image.h"

static agc_word rope[2][APOLLO_ROPE_WORDS];
static uint8_t loaded[2];
/* FUZZ_DUMP: the rope words the compact interpreter's slot reads. */
static uint8_t recording;
static unsigned nrec;
static uint16_t recp[96], recw[96];
bool apollo_rope_read(unsigned mission, uint16_t physical, agc_word *word)
{
    if (mission > 1u || !loaded[mission] || physical >= APOLLO_ROPE_WORDS || !word) return false;
    *word = rope[mission][physical];
    if (recording && nrec < 96u) { unsigned i; for (i = 0; i < nrec && recp[i] != physical; ++i) {} if (i == nrec) { recp[nrec] = physical; recw[nrec++] = *word; } }
    return true;
}
static void put16(FILE *f, unsigned v) { fputc((int)(v & 0xffu), f); fputc((int)((v >> 8) & 0xffu), f); }
bool apollo_rope_load(unsigned mission, const char *path)
{
    FILE *f;
    unsigned i;
    unsigned char b[2];
    if (mission > 1u || !path || (f = fopen(path, "rb")) == 0) return false;
    for (i = 0; i < APOLLO_ROPE_WORDS; ++i) {
        unsigned bank = i / 1024u;
        if (fread(b, 1, 2, f) != 2u) { fclose(f); return false; }
        bank = bank < 2u ? bank + 2u : bank < 4u ? bank - 2u : bank;
        rope[mission][bank * 1024u + i % 1024u] = (agc_word)((((unsigned)b[0] << 8) | b[1]) >> 1);
    }
    fclose(f);
    loaded[mission] = 1;
    return true;
}

static uint64_t rs;
static unsigned rnd(unsigned n) { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return (unsigned)(rs % n); }
static agc_word rw(void) { return (agc_word)rnd(0100000u); }

#define CODE_BANK 020u
#define CODE_WORDS 0400u

/* No real program names the interpreter's own registers (INTBIT15 to
 * FIXLOC, MPAC to PUSHLOC) as an operand: both interpreters index tables by
 * MODE and PUSHLOC, past their ends for wild values. */
static agc_word safe(agc_word w)
{
    unsigned a = w & 03777u;
    if ((a >= 0115u && a <= 0121u) || (a >= 0154u && a <= 0170u) || a == 067u) return (agc_word)(w ^ 01000u);
    return w;
}
static agc_word operand0(void);
/* An address order's operand word is positive: a negative next word is
 * the following order pair, and the operand then comes from the push-down
 * list (PUSHUP), which the generator asks for with need_pair. */
static uint8_t need_pair;
static agc_word operand(void) { return (agc_word)(safe(operand0()) & 037777u); }
/* Erasable operand word: VAC-relative, general, EB-window or fixed, sometimes
 * a push-up (bit 15) or random. */
static agc_word operand0(void)
{
    switch (rnd(12)) {
    case 0: case 1: case 2: return (agc_word)rnd(045);
    case 3: case 4: return (agc_word)(0122u + rnd(060));          /* VBUF..MPAC..PUSHLOC neighbourhood */
    case 5: case 6: return (agc_word)(01000u + rnd(0370u));
    case 7: return (agc_word)(01400u + rnd(0400u));
    case 8: return (agc_word)(02000u + rnd(02000u));
    case 9: return (agc_word)rnd(045);
    case 10: return (agc_word)(045u + rnd(01733u));
    default: return (agc_word)(rw() & 037777u);
    }
}
/* Word starts of the generated program, and the words holding branch
 * targets, patched to starts once the program is laid out. */
static uint8_t start[CODE_WORDS];
static unsigned tpos[CODE_WORDS], ntpos;
static uint16_t *prog_at;
/* A fixed CADR inside the program (patched), or an erasable one (GOTOERS). */
static agc_word target_at(unsigned at)
{
    if (rnd(6) == 0) return (agc_word)(0100u + rnd(01300u));
    tpos[ntpos++] = at;
    return 0;
}
/* A random order code, weighted by class. */
static unsigned code(void)
{
    unsigned r = rnd(100);
    if (r < 2) return 0;                                        /* EXIT */
    if (r < 55) {   /* address (SETPD only direct, as the assembler writes it) */
        unsigned idx = rnd(32);
        return (unsigned)(1u | (idx == 037u ? 0u : rnd(2) << 1) | (idx << 2));
    }
    if (r < 75) return (unsigned)(2u | (rnd(2) << 2) | (rnd(16) << 3));   /* misc */
    if (r < 85) return (unsigned)(4u | (rnd(16) << 3));                    /* short shift */
    return (unsigned)(rnd(15) + 1u) << 3;                                  /* unary */
}
static unsigned emit_operands(uint16_t *p, unsigned at, unsigned c, unsigned last)
{
    if (c == 0) return at;
    if (c & 1u) {
        if (last && (c >> 2) != 004u && (c >> 2) != 011u && (c >> 2) != 037u && rnd(5) == 0) { need_pair = 1; return at; }
        p[at++] = (c >> 2) == 037u ? (agc_word)rnd(045) : operand();   /* SETPD: in the VAC area */
        if ((c >> 2) == 004u || (c >> 2) == 011u) { p[at] = rnd(2) ? target_at(at) : safe(rw()); ++at; }   /* CGOTO list / SSP word */
        return at;
    }
    if (c & 2u) {
        unsigned idx = c >> 3;
        if (idx >= 010u && idx != 016u) { p[at] = target_at(at); ++at; }
        else if (idx == 016u) {   /* a positive switch word, and a branch word unless it is a plain set/clear */
            agc_word sw = safe((agc_word)(rw() & 037777u));
            p[at++] = sw;
            if (((sw >> 2) & 014u) != 014u) { p[at] = target_at(at); ++at; }
        }
        else p[at++] = rnd(2) ? (agc_word)rnd(0200) : safe(rw());
        return at;
    }
    if (c & 4u) return at;   /* short shift */
    if (((c >> 3) - 1u) == 014u) p[at++] = (agc_word)(~(rnd(3) << 11 | safe((agc_word)(operand() % 02000u))) & 077777u);   /* STADR: a complemented STORE */
    return at;
}
static void program(uint16_t *p)
{
    unsigned at = 0, i, n = 0;
    static unsigned list[CODE_WORDS];
    memset(start, 0, sizeof start);
    ntpos = 0;
    prog_at = p;
    while (at < CODE_WORDS - 8u) {
        start[at] = 1;
        list[n++] = at;
        if (!need_pair && rnd(5) == 0) {
            unsigned kind = rnd(8);
            p[at++] = (uint16_t)((kind << 11) | safe((agc_word)(operand() % 02000u)));   /* stores go to erasable */
            if (kind >= 3u) { p[at] = kind == 7u ? target_at(at) : operand(); ++at; }
        } else {
            unsigned l = code(), r = rnd(6) && !getenv("FUZZ_SINGLE") ? code() : 0177u, v, w;
            need_pair = 0;
            if (r == 0177u) v = l; else v = l | ((r + 1u) << 7);
            w = (~(v + 1u)) & 077777u;
            p[at++] = (uint16_t)w;
            at = emit_operands(p, at, l, r == 0177u || !(r & 1u) ? 0u : 0u);
            if (r != 0177u) at = emit_operands(p, at, r, 1);
        }
    }
    while (at < CODE_WORDS) p[at++] = safe(rw());
    for (i = 0; i < ntpos; ++i) p[tpos[i]] = (uint16_t)((CODE_BANK << 10) | list[rnd(n)]);
}
static unsigned first_start(void)
{
    unsigned at;
    do at = rnd(CODE_WORDS - 8u); while (!start[at]);
    return at;
}

static void image(apollo_overlay_state *s, unsigned m)
{
    unsigned i, eb = 3u + rnd(5), vac = 0401u + 054u * rnd(5);
    for (i = 0; i < 2048u; ++i) s->words[i] = rw();
    s->words[0] = (agc_word)((s->words[0] & 077777u) | ((s->words[0] << 1) & 0100000u));
    s->words[1] = (agc_word)((s->words[1] & 077777u) | ((s->words[1] << 1) & 0100000u));
    s->words[2] &= 077777u;
    s->words[3] = (agc_word)(eb << 8);
    s->words[4] = (agc_word)(rnd(040) << 10);
    s->words[6] = (agc_word)(s->words[4] | eb);
    s->words[7] = 0;
    s->words[067] = rnd(4) ? 0 : (agc_word)rnd(040000);
    s->words[0120] = (agc_word)vac;
    s->words[0166] = (agc_word)(vac + rnd(040));
    s->words[0163] = rnd(3) == 0 ? 077776u : rnd(2) ? 0u : 1u;
    s->words[0164] = (agc_word)(02000u + first_start());
    s->words[0165] = (agc_word)((CODE_BANK << 10) | eb);
    s->words[0115] = 0;
    s->words[023] = 0;
    if (rnd(3) == 0) s->words[0121] = 0;
    for (i = 0; i < 4; ++i) s->words[vac + 046u + i] = (agc_word)(rnd(2) ? rnd(010) : 077777u - rnd(010));   /* small X1 X2 S1 S2 */
    if (rnd(2)) for (i = 0; i < 7; ++i) s->words[0154u + i] = (agc_word)(rnd(2) ? rnd(010000) : (077777u - rnd(010000)));
    s->words[5] = m ? 06037u : 06070u;
}

static int small(agc_word x) { return x < 01000u || x > 076777u; }
static int wild(const apollo_overlay_state *s, const apollo_overlay_state *pre)
{
    unsigned v = s->words[0120];
    if ((s->words[3] >> 8) < 3u || v != pre->words[0120] || (s->words[0164] & 076000u) != 02000u
        || ((s->words[0165] >> 10) & 037u) != CODE_BANK) return 1;
    if (!small(s->words[v + 046u]) || !small(s->words[v + 047u]) || !small(pre->words[v + 046u]) || !small(pre->words[v + 047u])) return 1;
    return 0;
}
static int interpreter_point(const apollo_overlay_state *s, unsigned m)
{
    agc_word z = s->words[5];
    if (m == 0)
        return z == 06070u || z == 06061u || z == 06063u || z == 04631u || z == 06037u;
    return z == 06037u || z == 06030u || z == 06032u || z == 04675u || z == 06006u;
}

/* Random code can run a body loop without end (DDV normalizing a zero
 * divisor word, as the AGC would too): such a case is abandoned. */
static sigjmp_buf hang;
static void on_alarm(int sig) { (void)sig; siglongjmp(hang, 1); }
static void arm(unsigned ms)
{
    struct itimerval t;
    memset(&t, 0, sizeof t);
    t.it_value.tv_sec = ms / 1000u; t.it_value.tv_usec = (ms % 1000u) * 1000u;
    setitimer(ITIMER_REAL, &t, 0);
}

/* The compact interpreter's slot again on a copy of the image followed by
 * unmapped pages: 1 when it touched no word past the image and ended as
 * in the comparison. */
static sigjmp_buf fault;
static void on_fault(int sig) { (void)sig; siglongjmp(fault, 1); }
static int guarded_ok(const apollo_overlay_state *pre, unsigned m, const apollo_overlay_state *want)
{
    static apollo_overlay_state *g;
    p66_bias_io l = { 0, 1 };
    cm_native_io c;
    int ok;
    if (!g) {
        char *p = mmap(0, 4096u + 0x40000u, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (p == MAP_FAILED || mprotect(p + 4096, 0x40000u, PROT_NONE)) return 0;
        g = (apollo_overlay_state *)(void *)p;
    }
    *g = *pre;
    memset(&c, 0, sizeof c); c.interrupt_enabled = 1;
    signal(SIGSEGV, on_fault);
    if (sigsetjmp(fault, 1)) ok = 0;
    else { if (m) (void)cm_interp_core(0, g, &c); else (void)lm_interp_core(0, g, &l); ok = !memcmp(g, want, sizeof *g); }
    signal(SIGSEGV, on_alarm);
    return ok;
}

int main(int argc, char **argv)
{
    static unsigned hung, c, m, slots, refused, bad, shown, unreal;
    static apollo_overlay_state pre;
    static FILE *dump;
    unsigned cases = argc > 3 ? (unsigned)strtoul(argv[3], 0, 0) : 2000u, seed = argc > 4 ? (unsigned)strtoul(argv[4], 0, 0) : 1u;
    unsigned which = argc > 5 ? (unsigned)strtoul(argv[5], 0, 0) : 2u;
    /* Random words can send an index past the image (both interpreters read
     * W[a] for any 16-bit a, as no real program asks them to): equal padding
     * keeps such reads alike, and the padding is compared too. */
    static struct { apollo_overlay_state s; agc_word pad[0200000]; } pa, pb;
#define a pa.s
#define b pb.s
    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        fprintf(stderr, "usage: interp_core_fuzz <lm.bin> <cm.bin> [cases] [seed] [mission]\n");
        return 2;
    }
    if (getenv("FUZZ_DUMP") && (dump = fopen(getenv("FUZZ_DUMP"), "wb")) == 0) return 2;
    for (c = getenv("FUZZ_CASE") ? (unsigned)atoi(getenv("FUZZ_CASE")) : 0; c < cases; ++c) {
        unsigned slot;
        rs = 0x9e3779b97f4a7c15ull ^ ((uint64_t)(seed * 1000003u + c) << 1);
        rnd(2);
        m = which < 2u ? which : c & 1u;
        program(&rope[m][CODE_BANK * 1024u]);
        if (sigsetjmp(hang, 1)) { ++hung; continue; }
        signal(SIGALRM, on_alarm);
        signal(SIGSEGV, on_alarm);   /* (a wild index far past the padding) */
        arm(300);
        image(&a, m);
        b = a;
        memset(pa.pad, 0, sizeof pa.pad); memset(pb.pad, 0, sizeof pb.pad);
        for (slot = 0; slot < 40u; ++slot) {
            enum exec_status sa, sb;
            unsigned i, diff = 0;
            p66_bias_io la = { 0, 1 }, lb = { 0, 1 };
            cm_native_io ca, cb;
            if (!interpreter_point(&a, m)) break;
            /* A real program keeps its push-down pointer in its VAC area. */
            if (a.words[0166] < a.words[0120] + 4u || a.words[0166] > a.words[0120] + 052u) break;
            /* Only the generated code: the rest of the rope read as interpretive
             * code is not interpretive code. */
            if (((a.words[0165] >> 10) & 037u) != CODE_BANK) break;
            /* (at a word the generator laid out as one: no operand read as code) */
            if (a.words[5] == (m ? 06037u : 06070u) && ((a.words[0164] & 01777u) >= CODE_WORDS || !start[a.words[0164] & 01777u])) break;
            /* and MODE -1, +0 or +1 (both interpreters index tables by it). */
            if (a.words[0163] > 1u && a.words[0163] != 077776u) break;
            if (getenv("FUZZ_PROGRESS")) fprintf(stderr, "case %u slot %u\n", c, slot);
            if (getenv("FUZZ_CASE")) {
                agc_word w = 0;
                (void)apollo_rope_read(m, (uint16_t)(((a.words[0165] >> 10) & 037u) * 1024u + (a.words[0164] & 01777u)), &w);
                fprintf(stderr, "slot %u Z %05o LOC %05o word %05o EDOP %03o MODE %05o\n", slot, a.words[5], a.words[0164], w, a.words[023], a.words[0163]);
            }
            memset(&ca, 0, sizeof ca); ca.interrupt_enabled = 1; cb = ca;
            pre = a;
            if (m) { sa = cm_interp_runtime(0, &a, &ca); recording = 1; nrec = 0; sb = cm_interp_core(0, &b, &cb); recording = 0; }
            else {
                sa = getenv("FUZZ_CORE2") ? lm_interp_core(0, &a, &la) : lm_interp_runtime(0, &a, &la);
                recording = 1; nrec = 0;
                sb = getenv("FUZZ_SELF") ? lm_interp_runtime(0, &b, &lb) : lm_interp_core(0, &b, &lb);
                recording = 0;
            }
            if (sa == EXEC_OUT_OF_SCOPE) { ++refused; break; }
            /* A slot that leaves the program's frame (an index register or
             * a store run wild: EBANK 0-2, which lays the EB window over the
             * registers, a store into EDOP or FIXLOC, X1/X2 far from their
             * small indexing range) is outside what any program does; there
             * the two interpreters' register-write orders may differ. */
            if (wild(&a, &pre) || wild(&b, &pre)) { ++unreal; break; }
            ++slots;
            for (i = 0; i < 2048u; ++i) if (a.words[i] != b.words[i]) ++diff;
            if (memcmp(pa.pad, pb.pad, sizeof pa.pad)) { ++diff; memset(pa.pad, 0, sizeof pa.pad); memset(pb.pad, 0, sizeof pb.pad); }
            if (sa != sb || diff || memcmp(&la, &lb, sizeof la) || memcmp(&ca, &cb, sizeof ca)) {
                ++bad;
                if (shown++ < (getenv("FUZZ_SHOW") ? (unsigned)atoi(getenv("FUZZ_SHOW")) : 12u)) {
                    fprintf(stderr, "case %u (%s) slot %u: status %d/%d, %u words differ:", c, m ? "cm" : "lm", slot, sa, sb, diff);
                    for (i = 0; i < 2048u && diff; ++i) if (a.words[i] != b.words[i]) { fprintf(stderr, " %04o=%06o/%06o", i, a.words[i], b.words[i]); --diff; }
                    fprintf(stderr, "\n");
                }
                break;
            }
            /* (not a slot that touches a word past the image: on the 6502
             * that is the program itself; rerun with the image before
             * unmapped pages) */
            if (dump && nrec < 96u && guarded_ok(&pre, m, &b)) {
                agc_word w = 0;
                (void)apollo_rope_read(m, (uint16_t)(((pre.words[0165] >> 10) & 037u) * 1024u + (pre.words[0164] & 01777u)), &w);
                put16(dump, 0xa6c1u);
                for (i = 0; i < 2048u; ++i) put16(dump, pre.words[i]);
                put16(dump, 1u); put16(dump, 0u); put16(dump, nrec);
                for (i = 0; i < nrec; ++i) { put16(dump, recp[i]); put16(dump, recw[i]); }
                put16(dump, w);
                /* the slot's result */
                for (i = 0; i < 2048u; ++i) put16(dump, b.words[i]);
            }
            if (sa != EXEC_OK) break;
            if (getenv("FUZZ_CASE") && slot == 0 && c == (unsigned)atoi(getenv("FUZZ_CASE")) + 1u) break;
        }
    }
    arm(0);
    if (dump) fclose(dump);
    printf("%u cases, %u slots compared, %u refused, %u abandoned, %u left the frame, %u differ\n", cases, slots, refused, hung, unreal, bad);
    return bad ? 1 : 0;
}
