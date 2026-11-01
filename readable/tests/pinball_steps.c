/* SPDX-License-Identifier: MIT */
/* The readable Pinball step for step against the faithful trampoline.
 *
 * The flight program runs the routine tables one routine at a time and
 * charges the AGC clock for each by the point it starts at
 * (c64-port/platform/x86/flight_dispatch.c, cost()). So the readable
 * routines must not only end each chain in the original program's state
 * (pinball_transfer.py's suites) but take the same steps as the faithful
 * routines they stand for: start where they start, stop where they stop,
 * and leave every word, channel and the superbank as they do.
 *
 * This runs the chains of pin-transfer and pin-decdsp3 one routine at a
 * time (lanec_pin_chain_max, one step). At each step it runs the step
 * twice from the same state: through the faithful trampoline
 * (faithful_lanec_pin_run_<m>) and through the readable one (here as
 * readable_lanec_pin_run_<m>, which tests/pinball_transfer.py renames).
 * Where the faithful step is taken the readable one must be taken too,
 * with the same result; the chain goes on from it. A step the faithful
 * trampoline refuses ends the chain (the readable code may take it: the
 * chain suites check those against the original program).
 *
 * Those chains start from the states the port's own module tests made,
 * the states the keys and the routines before them leave. pin-stirred's
 * are counted but not held to it: where a stirred state is one no key
 * leaves, a faithful module may stop at a point its own cases do not
 * reach (89TEST stops at NUM unless the state is one CHARIN left, when it
 * runs on into NUM's module), or give another result than the original
 * program (CLR5's INREL, issue #1093), and the readable
 * routine goes on as the original program does.
 *
 *   pinball-steps     prints the counts; exit status 1 on any difference
 *                     in pin-transfer's or pin-decdsp3's steps
 *
 * Test only. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_pin_routines.h"
#include "dsky_fixed_display.h"

enum exec_status faithful_lanec_pin_run_lm(const agc_native_program *, apollo_overlay_state *,
                                           void *, const lanec_pin_dispatch *);
enum exec_status faithful_lanec_pin_run_cm(const agc_native_program *, apollo_overlay_state *,
                                           void *, const lanec_pin_dispatch *);
enum exec_status readable_lanec_pin_run_lm(const agc_native_program *, apollo_overlay_state *,
                                           void *, const lanec_pin_dispatch *);
enum exec_status readable_lanec_pin_run_cm(const agc_native_program *, apollo_overlay_state *,
                                           void *, const lanec_pin_dispatch *);
enum exec_status lanec_pin_chain_max(apollo_overlay_state *s, lanec_io *io,
                                     enum lanec_mission mission, uint16_t stop, uint16_t max_steps,
                                     uint16_t *steps);

/* Which trampoline the routine tables call, and whether it was called. */
static int readable, called;

enum exec_status lanec_pin_run_lm(const agc_native_program *p, apollo_overlay_state *s, void *v,
                                  const lanec_pin_dispatch *f)
{
    called = 1;
    return readable ? readable_lanec_pin_run_lm(p, s, v, f) : faithful_lanec_pin_run_lm(p, s, v, f);
}

enum exec_status lanec_pin_run_cm(const agc_native_program *p, apollo_overlay_state *s, void *v,
                                  const lanec_pin_dispatch *f)
{
    called = 1;
    return readable ? readable_lanec_pin_run_cm(p, s, v, f) : faithful_lanec_pin_run_cm(p, s, v, f);
}

/* Per suite: steps compared, steps that differ, and steps the readable
 * routine takes where the faithful one refuses. */
static unsigned long steps_compared, readable_more, differences;

static void report(const char *suite, uint32_t n, unsigned step, const char *what,
                   const apollo_overlay_state *from, const apollo_overlay_state *f,
                   const apollo_overlay_state *r)
{
    unsigned i, shown = 0;

    if (++differences > 10u && !getenv("PINBALL_STEPS_ALL")) return;
    printf("DIFF %s case %lu step %u (Z %05o, FB %05o): %s, Z %05o/%05o", suite, (unsigned long)n,
           step, from->words[LANEC_Z], from->words[4], what, f->words[LANEC_Z], r->words[LANEC_Z]);
    for (i = 0; i < 04000u && shown < 8u; ++i)
        if (f->words[i] != r->words[i]) {
            printf(" %04o %05o/%05o", i, f->words[i], r->words[i]);
            ++shown;
        }
    printf("\n");
}

static void chain(const char *suite, uint32_t n, apollo_overlay_state *s, lanec_io *io,
                  enum lanec_mission mission)
{
    static apollo_overlay_state from, f, r;
    static lanec_io f_io, r_io;
    enum exec_status fs, rs;
    uint16_t fsteps, rsteps;
    unsigned step;

    for (step = 0; step < 64u; ++step) {
        from = f = r = *s;
        f_io = r_io = *io;
        readable = 0;
        called = 0;
        fs = lanec_pin_chain_max(&f, &f_io, mission, (uint16_t)(AGC_TRANSFER_NONE - 1u), 1u,
                                 &fsteps);
        if (!fsteps) return;
        if (called) {
            readable = 1;
            called = 0;
            rs = lanec_pin_chain_max(&r, &r_io, mission, (uint16_t)(AGC_TRANSFER_NONE - 1u), 1u,
                                     &rsteps);
            if (fs != EXEC_OK) {
                readable_more += rs == EXEC_OK;
                return;
            }
            ++steps_compared;
            if (rs != EXEC_OK)
                report(suite, n, step, "refused", &from, &f, &r);
            else if (memcmp(f.words, r.words, sizeof f.words))
                report(suite, n, step, "words", &from, &f, &r);
            else if (memcmp(&f_io, &r_io, sizeof f_io))
                report(suite, n, step, "channels", &from, &f, &r);
            if (rs != EXEC_OK || memcmp(f.words, r.words, sizeof f.words) ||
                memcmp(&f_io, &r_io, sizeof f_io))
                return;
        } else if (fs != EXEC_OK)
            return;
        *s = f;
        *io = f_io;
    }
}

/* The rope for the routines that read it as data, as tools/lanec/native.c
 * gives it. */
static bool fixed(enum lanec_mission mission, uint16_t physical, agc_word *word)
{
    return dsky_fixed_display_data(mission == LANEC_LM ? DSKY_NV_LM : DSKY_NV_CM, physical, word,
                                   0);
}

int main(void)
{
    static const struct {
        const char *name;
        int held; /* the steps must be the faithful port's */
    } suites[] = {{"pin-transfer", 1}, {"pin-decdsp3", 1}, {"pin-stirred", 0}};
    static apollo_overlay_state s;
    static lanec_io io;
    static lanec_meta m;
    const char *only = getenv("PINBALL_STEPS_CASE"); /* SUITE:CASE, for a debugger */
    unsigned i;
    uint32_t n;
    int bad = 0;

    lanec_fixed_read = fixed;
    for (i = 0; i < sizeof suites / sizeof suites[0]; ++i) {
        const lanec_suite *suite = lanec_find_suite(suites[i].name);

        if (!suite) {
            printf("FAIL: no suite %s\n", suites[i].name);
            return 1;
        }
        steps_compared = readable_more = differences = 0;
        for (n = 0; n < suite->count; ++n) {
            char name[64];

            snprintf(name, sizeof name, "%s:%lu", suite->name, (unsigned long)n);
            if (only && strcmp(name, only)) continue;
            memset(&s, 0, sizeof s);
            memset(&io, 0, sizeof io);
            memset(&m, 0, sizeof m);
            suite->make(n, &s, &io, &m);
            chain(suite->name, n, &s, &io, m.mission);
        }
        printf("%s: pinball-steps %s: %lu Pinball steps the same through the readable routines as "
               "through the "
               "faithful port's, %lu differ%s; %lu more taken where the faithful port refuses\n",
               !suites[i].held ? "INFO"
               : differences   ? "FAIL"
                               : "PASS",
               suite->name, steps_compared - differences, differences,
               suites[i].held ? "" : " (not held)", readable_more);
        bad |= suites[i].held && differences;
    }
    return bad;
}
