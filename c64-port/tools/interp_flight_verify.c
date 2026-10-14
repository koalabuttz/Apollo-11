/* SPDX-License-Identifier: MIT */
/* Host pre-check for the flowed interp-flight cases
 * (tools/interp_flight_test.py --flowed): the verification interpreter
 * (src/lm_interp_runtime.c, its order bodies checked against yaAGC) runs
 * each case's slots while Z stays at an interpreter entry point, as
 * platform/c64/interp_flight_check.c runs resident steps, and writes the
 * result in that program's format (digest, exit Z, exit FB, slots, status,
 * still at an entry). tools/interp_flight_capture.c check then says which
 * of these states yaAGC reaches: a case it does not, or that the
 * verification interpreter refuses, runs outside the interpreter's
 * contract (docs/INTERPRETER-RUNTIME.md: an indexed or push-down address
 * past erasable, LOC not at an order, an operand bank past the rope) and
 * is left out of the VICE run.
 *
 *   interp-flight-verify ROPE.bin CASES.bin RESULTS.bin
 *
 * Built with -Dexit=interp_flight_verify_exit: the verification
 * interpreter's host build stops an erasable-CADR GOTO cycle with exit(),
 * which here refuses the case. */
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "interp_runtime.h"
#include "rope_image.h"
#include "p66_bias.h"

static agc_word rope[APOLLO_ROPE_WORDS];
bool apollo_rope_read(unsigned mission, uint16_t physical, agc_word *word)
{
    if (mission || physical >= APOLLO_ROPE_WORDS || !word) return false;
    *word = rope[physical];
    return true;
}
static int load(const char *path)
{
    FILE *f = fopen(path, "rb");
    unsigned i;
    unsigned char b[2];
    if (!f) return 0;
    for (i = 0; i < APOLLO_ROPE_WORDS; ++i) {
        unsigned bank = i / 1024u;
        if (fread(b, 1, 2, f) != 2u) { fclose(f); return 0; }
        bank = bank < 2u ? bank + 2u : bank < 4u ? bank - 2u : bank;
        rope[bank * 1024u + i % 1024u] = (agc_word)((((unsigned)b[0] << 8) | b[1]) >> 1);
    }
    fclose(f);
    return 1;
}
static jmp_buf cycle;
void interp_flight_verify_exit(int status)
{
    (void)status;
    longjmp(cycle, 1);
}
/* tools/interp_flight_capture.c points[] (LM). */
static const unsigned points[] = {06070, 06061, 06063, 06106, 06460, 06144, 06463, 06466, 06252, 06646, 06037};
static int at_point(const apollo_overlay_state *s)
{
    unsigned i;
    for (i = 0; i < sizeof points / sizeof points[0]; ++i) if ((s->words[5] & 07777u) == points[i]) return 1;
    return 0;
}

/* The slots from s while Z stays at an entry point (their count in *slots). */
static enum exec_status run(apollo_overlay_state *s, p66_bias_io *io, unsigned *slots)
{
    volatile enum exec_status st = EXEC_OK;
    volatile unsigned k = 0;
    if (setjmp(cycle)) st = EXEC_OUT_OF_SCOPE;
    else while (k < 4000u && at_point(s) && (st = lm_interp_runtime(0, s, io)) == EXEC_OK) ++k;
    *slots = k;
    return st;
}

int main(int argc, char **argv)
{
    static apollo_overlay_state s;
    unsigned char c[4104], r[12];
    FILE *in, *out;
    unsigned i, n = 0;
    if (argc != 4 || !load(argv[1]) || !(in = fopen(argv[2], "rb")) || !(out = fopen(argv[3], "wb"))) {
        fprintf(stderr, "usage: interp-flight-verify ROPE.bin CASES.bin RESULTS.bin\n");
        return 2;
    }
    while (fread(c, 1, sizeof c, in) == sizeof c) {
        p66_bias_io io = {(uint16_t)(c[4096] | c[4097] << 8), 1u};
        enum exec_status st;
        unsigned slots;
        uint16_t a = 0, b = 0;
        for (i = 0; i < 2048u; ++i) s.words[i] = (agc_word)(c[2 * i] | c[2 * i + 1] << 8);
        st = run(&s, &io, &slots);
        /* (the digest tools/interp_flight_capture.c check compares) */
        for (i = 0; i < 2048u; ++i) {
            if (i >= 024u && i <= 031u) continue;
            a = (uint16_t)(a + (s.words[i] & (i < 2u ? 0177777u : 077777u)));
            b = (uint16_t)(b + a);
        }
        a = (uint16_t)(a + io.superbank); b = (uint16_t)(b + a);
        r[0] = (unsigned char)a; r[1] = (unsigned char)(a >> 8); r[2] = (unsigned char)b; r[3] = (unsigned char)(b >> 8);
        r[4] = (unsigned char)s.words[5]; r[5] = (unsigned char)(s.words[5] >> 8);
        r[6] = (unsigned char)s.words[4]; r[7] = (unsigned char)(s.words[4] >> 8);
        /* (steps: nonzero, as check fails a case that took none) */
        r[8] = (unsigned char)(slots + 1u); r[9] = (unsigned char)((slots + 1u) >> 8);
        r[10] = (unsigned char)st; r[11] = (unsigned char)at_point(&s);
        fwrite(r, 1, sizeof r, out);
        printf("case %u: %u slots, status %d, exit Z %05o FB %05o\n", n++, slots, (int)st, s.words[5], s.words[4]);
    }
    fclose(in);
    fclose(out);
    return 0;
}
