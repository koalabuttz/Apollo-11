/* SPDX-License-Identifier: MIT */
/* pw-octal-consumer: the DSPA/B/C/AB/ABC row loop (src/dsky_octal_consumer.c)
 * from the generator's verb entry, chaining BEGIN, CHANNEL, FORMAT and
 * RETURN stages (each accepted only at its own Z) until no stage accepts
 * or a stage leaves the loop (ENTRET, alarm or recovery). The CHANNEL stage
 * reads the channel NOUNCADR selects from io->channel[]. The generator
 * chooses the mission (numbers 0-2047 LM, 2048-4095 CM). */
#include "lanec_pw.h"
#include "dsky_octal_consumer.h"
#include "dsky_octal_consumer_cases.h"

#define CASES 1024u
static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    p66_bias_io p;
    enum dsky_nv_mission mission;
    /* past the spread: the module's own states with the noun in low erasable */
    uint16_t k = n < CASES ? (uint16_t)lanec_pw_spread(n, CASES, DSKY_OCTAL_CONSUMER_CASES)
                           : (uint16_t)((n - CASES) * 127u % DSKY_OCTAL_CONSUMER_CASES);
    lanec_fill(s, io, n);
    dsky_octal_consumer_input(s, &p, k, &mission);
    if (n >= CASES) lanec_pw_low_noun(s, n - CASES);
    io->superbank = p.superbank; io->interrupt_enabled = p.interrupt_enabled;
    lanec_pw_settle(s);
    m->mission = mission == DSKY_NV_CM ? LANEC_CM : LANEC_LM;
    lanec_pw_exit_stop(m, s, io, lanec_pw_octal_consumer_run);
}
const lanec_suite lanec_suite_pw_octal_consumer = { "pw-octal-consumer", CASES + LANEC_PW_LOW_NOUN, make, lanec_pw_octal_consumer_run };
