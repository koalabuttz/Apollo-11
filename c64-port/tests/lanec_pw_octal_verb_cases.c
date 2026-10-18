/* SPDX-License-Identifier: MIT */
/* pw-octal-verb: the octal display verbs 1-5 (src/dsky_octal_verb.c): the
 * verb bodies from their entries, the row return at 02422 and the channel
 * read continuation at 02504 (chosen, as the generator does, by Z).
 * Generator numbers 0-8191 are LM, 8192-16383 CM.
 *
 * The channel continuation reads channel (CHANNEL word 01017) & 0777; the
 * generator passes the value it reads (word 01020) separately. Lane C holds
 * channels 0-077 only, so make() folds the channel number to its low six
 * bits (still a channel the module serves) and puts the value in that
 * channel of the lanec_io. */
#include "lanec_pw_e.h"
#include "dsky_octal_verb.h"
#include "dsky_octal_verb_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    uint8_t entry;
    /* past the spread: the module's own states with the noun in low erasable
     * (V01-V05 and V11-V15 on nouns 20, 21 and 36), then at the top of
     * erasable (01776, 01777: V04 and V05 read on into fixed bank 41, as
     * on the LM's noun 13, TCDH) */
    uint16_t k = n < CASES ? (uint16_t)lanec_pw_spread(n, CASES, DSKY_OCTAL_VERB_CASES)
                           : (uint16_t)((n - CASES) * 127u % DSKY_OCTAL_VERB_CASES), ch;
    uint32_t i = n - CASES;
    lanec_fill(s, io, n);
    dsky_octal_verb_input(s, &p, k, &mission, &entry);
    if (n >= CASES + LANEC_PW_LOW_NOUN && entry != 2u) {
        uint16_t *w = s->words, f = w[0140] == 1u ? 0147u : 0146u;
        w[0145] = (uint16_t)(01776u + (i & 1u));
        w[f] = (uint16_t)((w[f] & ~06000u) | ((1u + (i / 2u) % 2u) << 10));
        p.superbank = 0100u;
    } else if (n >= CASES && entry != 2u) lanec_pw_low_noun(s, i);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    if (entry == 2u) {
        s->words[01017] = (uint16_t)(s->words[01017] & ~0700u);
        ch = (uint16_t)(s->words[01017] & 077u);
        if (ch != 1u && ch != 2u && ch != 7u) io->channel[ch] = s->words[01020];
    }
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    LANEC_PW_E_STOP(m, s, io, lanec_pw_octal_verb_run);
}
const lanec_suite lanec_suite_pw_octal_verb = { "pw-octal-verb", CASES + LANEC_PW_LOW_NOUN + 16u, make, lanec_pw_octal_verb_run };
