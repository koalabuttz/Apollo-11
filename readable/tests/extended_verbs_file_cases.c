/* SPDX-License-Identifier: MIT */
/* The CM's readable EXTENDED_VERBS whole: first part 9's tables (the
 * interpretive words, track.c), written in the listing's notation, must
 * be the original program's rope, word for word; then the file runs side
 * by side with the port's
 * compiled copy of the file (c64-port/src/cm_extended_verbs_file.c, built
 * with its tables renamed faithful_...), on the port's frozen cases and
 * on each of them stirred (make extended-verbs-cases).
 *
 * The compiled file is certified against the original program (its
 * frozen cases' Virtual AGC digests, which the readable file meets first,
 * in ev-file-oracle). Here every case runs again in each program from the
 * same start, and again from VARIANTS - 1 stirred starts: a word of the
 * interpreter's contract (NEWJOB, FIXLOC, EDOP, BANKSET, INTBIT15), a bank
 * register, Z or LOC moved by one, the latches, MPAC, the channels, or
 * any erasable word. (A stirred start that only the parts' tables list,
 * a word inside one of the compiled file's routines, is left out.) Each
 * pair of runs must end alike: the same outcome,
 * memory, interrupt latch, channels and traps (the harness's digest of
 * them, tests/interp_case.c), word for word. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interp_case.h"
#include "rope_image.h"
#include "cm_extended_verbs_file_vectors.h"
#include "extended_verbs_file_program.h"
#include "cm_extended_verbs_file.h"

/* Part 9's tables (src/interpreter/notation.h's struct interpretive_code). */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code TRACKTRM_CODE, SETVHFLG_CODE, RESETVHF_CODE, ATTACHIT_CODE,
    TACHEXIT_CODE, TCPIN_CODE, LMTOCM_CODE;

/* The table's words that are not the CM's rope's, each shown. */
static unsigned words_differ(const struct interpretive_code *code, unsigned *words)
{
    unsigned i, differ = 0;

    for (i = 0; i < code->count; ++i) {
        unsigned physical = code->bank * 1024u + code->first - 02000u + i;
        agc_word rope = 0;

        ++*words;
        if (!apollo_rope_read(1, (uint16_t)physical, &rope) || rope != code->words[i]) {
            printf("FAIL: %02o,%04o is %05o, the rope's %05o\n", code->bank, code->first + i,
                   code->words[i], rope);
            ++differ;
        }
    }
    return differ;
}

extern const agc_routine_table faithful_cm_extended_verbs_file_basic,
    faithful_cm_extended_verbs_file_interpretive;

#define VARIANTS 16u

static uint32_t seed = 271828u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* Stir start (and its case's latches) by variant v; 0 leaves it. */
static void stir(apollo_overlay_state *s, irc_case *c, unsigned v)
{
    static const agc_word superbanks[] = {0u, 0100u, 0120u, 0140u, 0160u, 0101u, 0100u, 0100u};
    unsigned k;

    switch (v) {
    case 0: break;
    case 1: s->words[067] = rnd() % 2u ? 0u : (agc_word)(rnd() & 077777u); break; /* NEWJOB */
    case 2: /* FIXLOC: a VAC area, NOVAC's, or any word */
        k = rnd() % 8u;
        s->words[0120] = k < 5u    ? (agc_word)(0401u + 054u * k)
                         : k == 5u ? 0110u
                                   : (agc_word)(rnd() & 01777u);
        break;
    case 3:
        s->words[023] = (agc_word)(rnd() % 4u ? rnd() & 077u : rnd() & 077777u);
        break;                                                 /* EDOP */
    case 4: s->words[0165] ^= rnd() % 2u ? 02000u : 1u; break; /* BANKSET */
    case 5: s->words[0115] ^= 040000u; break;                  /* INTBIT15 */
    case 6: c->superbank = superbanks[rnd() % 8u]; break;
    case 7: c->interrupt_enabled = (uint8_t)(rnd() % 3u); break;
    case 8: /* FB, and BB with it */
        s->words[4] = (agc_word)((rnd() % 040u) << 10);
        s->words[6] = (agc_word)((s->words[4] & 076000u) | (s->words[3] >> 8));
        break;
    case 9: /* EB, and BB with it */
        s->words[3] = (agc_word)((rnd() % 8u) << 8);
        s->words[6] = (agc_word)((s->words[4] & 076000u) | (s->words[3] >> 8));
        break;
    case 10: s->words[6] ^= rnd() % 2u ? 1u : 02000u; break; /* BB alone */
    case 11: s->words[5] = (agc_word)(s->words[5] + (rnd() % 2u ? 1u : 077777u)) & 07777u; break;
    case 12:
        s->words[0164] = (agc_word)(s->words[0164] + (rnd() % 2u ? 1u : 077777u)) & 077777u;
        break;
    case 13: /* MPAC: the verb less 40 for GOEXTVB's fan (PINBALL passes no other) */
        s->words[0154] = (agc_word)(rnd() % 2u ? rnd() % 60u : rnd() & 077777u);
        if (s->words[5] == 02000u) s->words[0154] = (agc_word)(rnd() % 60u);
        break;
    case 14: /* the channels */
        irc_ch_on = 1;
        for (k = 0; k < IRC_NCH; ++k) irc_ch[k] = (agc_word)(rnd() & 077777u);
        break;
    default: /* any erasable word but the registers and counters */
        k = 062u + rnd() % (04000u - 062u);
        s->words[k] = (agc_word)(rnd() & 077777u);
        break;
    }
}

int main(void)
{
    static agc_native_program faithful, readable;
    static agc_hardware faithful_hw, readable_hw;
    unsigned i, v, runs = 0, moved = 0, inside = 0, failures = 0, by_variant[VARIANTS] = {0};
    unsigned show = getenv("SHOW") ? (unsigned)atoi(getenv("SHOW")) : 8u, only = VARIANTS;

    if (getenv("VARIANT")) only = (unsigned)atoi(getenv("VARIANT"));

    faithful.basic = &faithful_cm_extended_verbs_file_basic;
    faithful.basic_count = 1;
    faithful.interpretive = &faithful_cm_extended_verbs_file_interpretive;
    faithful.interpretive_count = 1;
    faithful.interpreter_fetch = (uint16_t)(2048u + 06037u - 04000u);
    faithful.loc = 0164;
    faithful.bankset = 0165;
    faithful.hardware = &faithful_hw;
    readable_cm_file_program(&readable, &readable_hw); /* (it loads the CM's rope) */
    {
        static const struct interpretive_code *const tables[] = {
            &TRACKTRM_CODE, &SETVHFLG_CODE, &RESETVHF_CODE, &ATTACHIT_CODE,
            &TACHEXIT_CODE, &TCPIN_CODE,    &LMTOCM_CODE,
        };
        unsigned t, words = 0, differ = 0;

        for (t = 0; t < sizeof tables / sizeof tables[0]; ++t)
            differ += words_differ(tables[t], &words);
        printf("ev-file-cases: %u words, %u not the rope's\n", words, differ);
        failures += differ;
    }
    /* The four basic words called straight, on a coherent state at each:
     * with the program's agc_hardware both ports run them (how far is the
     * runs' business, below: the compiled file goes on into DNEDUMP, the
     * readable word stops at its table's next entry); without it, which the
     * harness never gives (it lends its own), both refuse, the readable one
     * leaving the state as it was. */
    {
        const agc_routine_table *ft = &faithful_cm_extended_verbs_file_basic,
                                *rt = &cm_extended_verbs_file_basic;
        unsigned e, h, bad = 0;

        for (e = 0; e < rt->count; ++e)
            for (h = 0; h < 2u; ++h) {
                static apollo_overlay_state fs, rs;
                agc_native_program fp = faithful, rp = readable;
                cm_native_io fio, rio;
                agc_hardware fhw, rhw;
                enum exec_status fst, rst;
                unsigned j = 0;

                while (j < ft->count && ft->entries[j].physical != rt->entries[e].physical) ++j;
                if (j == ft->count) {
                    printf("FAIL basic word %u: not the compiled file's\n",
                           rt->entries[e].physical);
                    ++bad;
                    continue;
                }

                memset(&fs, 0, sizeof fs);
                memset(&fio, 0, sizeof fio);
                memset(&fhw, 0, sizeof fhw);
                fs.words[4] = 066000u; /* FB 33, with the superbank bit 43 */
                fs.words[5] = (agc_word)(02000u + rt->entries[e].physical % 1024u);
                fs.words[6] = 066000u;
                fio.superbank = 0100u;
                rs = fs;
                rio = fio;
                rhw = fhw;
                fp.hardware = h ? &fhw : NULL;
                rp.hardware = h ? &rhw : NULL;
                fst = ft->entries[j].call(&fp, &fs, &fio);
                rst = rt->entries[e].call(&rp, &rs, &rio);
                if (fst != rst || (h && fst != EXEC_OK) ||
                    (!h && (rst != EXEC_OUT_OF_SCOPE || memcmp(&fs, &rs, sizeof fs)))) {
                    printf("FAIL basic word %05o %s hardware: %u, faithful %u\n",
                           (unsigned)fs.words[5], h ? "with" : "without", rst, fst);
                    ++bad;
                }
            }
        printf("ev-file-cases: %u basic words with and without hardware, %u failures\n", rt->count,
               bad);
        failures += bad;
    }

    for (i = 0; i < CM_EXTENDED_VERBS_FILE_VECTORS; ++i)
        for (v = 0; v < VARIANTS; ++v) {
            static apollo_overlay_state start, f, r;
            agc_word ch[IRC_NCH], trap[3];
            uint8_t ch_on, isr_on;
            uint16_t isr, fie = 0, rie = 0;
            irc_case c = cm_extended_verbs_file_cases[i];
            unsigned fk, rk, w;
            uint32_t fd, rd;

            if (only < VARIANTS && v != only) continue;
            irc_build(&start, &c);
            stir(&start, &c, v);
            /* A start the parts' tables list and the compiled file does
             * not (a word inside one of its routines) is not compared. */
            {
                agc_routine fcall, rcall;

                (void)agc_transfer_next(&faithful, &start, c.superbank, 0xfffeu, &fcall);
                (void)agc_transfer_next(&readable, &start, c.superbank, 0xfffeu, &rcall);
                if (!fcall && rcall) {
                    ++inside;
                    continue;
                }
            }
            memcpy(ch, irc_ch, sizeof ch);
            memcpy(trap, irc_trap, sizeof trap);
            ch_on = irc_ch_on;
            isr_on = irc_isr_on;
            isr = irc_isr;

            f = start;
            fk = irc_run(&faithful, &f, &c, 200, &fie);
            fd = irc_digest(fk, &f, fie);

            memcpy(irc_ch, ch, sizeof ch);
            memcpy(irc_trap, trap, sizeof trap);
            irc_ch_on = ch_on;
            irc_isr_on = isr_on;
            irc_isr = isr;
            r = start;
            rk = irc_run(&readable, &r, &c, 200, &rie);
            rd = irc_digest(rk, &r, rie);

            ++runs;
            moved += fk == IRC_STOPPED && memcmp(&f, &start, sizeof f) != 0;
            if (fk != rk || fd != rd) {
                ++by_variant[v];
                if (++failures <= show) {
                    printf("FAIL case %u variant %u (Z %05o FB %05o LOC %05o BANKSET %05o, "
                           "superbank %03o, interrupts %u): outcome %u, faithful %u; digest "
                           "%08lx, faithful %08lx\n",
                           i, v, start.words[5], start.words[4], start.words[0164],
                           start.words[0165], c.superbank, c.interrupt_enabled, rk, fk,
                           (unsigned long)rd, (unsigned long)fd);
                    for (w = 0; w < 04000u; ++w)
                        if (f.words[w] != r.words[w])
                            printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, r.words[w],
                                   f.words[w], start.words[w]);
                }
            }
        }
    for (v = 0; v < VARIANTS; ++v)
        if (by_variant[v]) printf("variant %u: %u failures\n", v, by_variant[v]);
    printf("ev-file-cases: %u runs (%u cases, %u starts each, %u starts inside a compiled "
           "routine left out), %u moved the machine, %u failures\n",
           runs, CM_EXTENDED_VERBS_FILE_VECTORS, VARIANTS, inside, moved, failures);
    return failures != 0;
}
