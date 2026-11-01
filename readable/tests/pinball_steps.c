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
 *   pinball-steps --stops FILE
 *                     writes where the faithful steps of those chains end,
 *                     and the faithful routine tables' points
 *                     (src/binding/pinball_stops.inc, which the readable
 *                     binding steps by)
 *
 * Test only. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_pin_routines.h"
#include "dsky_fixed_display.h"
#include "lanec_symbols.h"

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

/* --stops: the faithful steps' starts and ends (src/binding/pinball_stops.inc). */
#define MAX_STOPS 4096u
static struct {
    uint16_t from, to;
} stops[2][MAX_STOPS];
static unsigned stop_count[2];
static int collect;

/* A fixed address's physical word number, as the readable machine counts
 * them (fixed_physical); an erasable one as 0176000 + the address. */
static uint16_t physical(const apollo_overlay_state *s, const lanec_io *io)
{
    uint16_t z = s->words[LANEC_Z], bank = (uint16_t)(s->words[4] >> 10);
    if (z < 02000u) return (uint16_t)(0176000u + z);
    if (z >= 04000u) return z; /* banks 2 and 3 */
    if (bank >= 030u && (io->superbank & 0100u)) bank = (uint16_t)(bank + 010u);
    return (uint16_t)(bank * 02000u + (z - 02000u));
}

static void add_stop(enum lanec_mission mission, uint16_t from, uint16_t to)
{
    unsigned m = mission == LANEC_LM ? 0u : 1u, i;
    for (i = 0; i < stop_count[m]; ++i)
        if (stops[m][i].from == from && stops[m][i].to == to) return;
    if (stop_count[m] == MAX_STOPS) {
        printf("FAIL: more than %u faithful steps\n", MAX_STOPS);
        exit(1);
    }
    stops[m][stop_count[m]].from = from;
    stops[m][stop_count[m]++].to = to;
}

static int by_stop(const void *a, const void *b)
{
    const uint16_t *x = a, *y = b;
    return x[0] != y[0] ? x[0] - y[0] : x[1] - y[1];
}

/* The points of the faithful port's Pinball routine tables: the file's
 * code, as far as the port enters it. */
static const agc_routine_table *const pinball_tables[2][6] = {
    {&lanec_pin_lm_routines, &lanec_pin_keys_num_lm_routines, &lanec_pin_keys_cmd_lm_routines,
     &lanec_pin_keys_release_lm_routines, &lanec_pin_keys_clear_lm_routines,
     &lanec_pin_keys_sign_lm_routines},
    {&lanec_pin_cm_routines, &lanec_pin_keys_num_cm_routines, &lanec_pin_keys_cmd_cm_routines,
     &lanec_pin_keys_release_cm_routines, &lanec_pin_keys_clear_cm_routines,
     &lanec_pin_keys_sign_cm_routines}};

static int by_point(const void *a, const void *b)
{
    return *(const uint16_t *)a - *(const uint16_t *)b;
}

static void write_points(FILE *out, unsigned m)
{
    static uint16_t points[MAX_STOPS];
    unsigned t, i, count = 0, kept = 0;

    for (t = 0; t < 6u; ++t)
        for (i = 0; i < pinball_tables[m][t]->count; ++i) {
            if (count == MAX_STOPS) {
                printf("FAIL: more than %u Pinball points\n", MAX_STOPS);
                exit(1);
            }
            points[count++] = pinball_tables[m][t]->entries[i].physical;
        }
    qsort(points, count, sizeof points[0], by_point);
    for (i = 0; i < count; ++i)
        if (!kept || points[i] != points[kept - 1u]) points[kept++] = points[i];
    fprintf(out, "static const uint16_t pinball_points[%u] = {\n", kept);
    for (i = 0; i < kept; ++i) fprintf(out, "    0%06o,\n", points[i]);
    fprintf(out, "};\n");
}

static bool fixed(enum lanec_mission mission, uint16_t physical, agc_word *word);

/* CHARIN's steps end at the key's routine: CHARIN2 +3 TC +1, indexed by the
 * key, runs its word of the table after it, TC <label>
 * (lanec_pin_keys_charin). The
 * chains here start most keys at their routine, so their stops are taken
 * from the table itself, all 32 keys, in bank 40 like CHARIN. */
static void add_charin_stops(enum lanec_mission mission)
{
    const uint16_t charin = mission == LANEC_LM ? LANEC_LM_CHARIN : LANEC_CM_CHARIN;
    uint16_t key;

    for (key = 0; key < 32u; ++key) {
        agc_word word = 0;
        uint16_t at = (uint16_t)(040u * 02000u + (charin + 15u + key - 02000u)), label;

        if (!fixed(mission, at, &word) || (word & 070000u)) {
            printf("FAIL: CHARIN's key table word %u is not a TC\n", key);
            exit(1);
        }
        label = (uint16_t)(word & 07777u);
        add_stop(mission, (uint16_t)(040u * 02000u + (charin - 02000u)),
                 label >= 04000u ? label : (uint16_t)(040u * 02000u + (label - 02000u)));
    }
}

/* DSPDCPUT's step runs through DECDSP3's BANKJUMP (INDEX A; CAF SFOUTABR) to
 * the scale factor routine's output routine and stops there, in flight for
 * routines the chains do not reach as well: the 14 CADRs of SFOUTABR, read
 * from the rope (CAF SFOUTABR is the word at DSPDCPUT +037). */
static void add_sfout_stops(enum lanec_mission mission)
{
    const uint16_t dspdcput = mission == LANEC_LM ? LANEC_LM_DSPDCPUT : LANEC_CM_DSPDCPUT;
    const uint16_t from = (uint16_t)(041u * 02000u + (dspdcput - 02000u));
    agc_word word = 0;
    uint16_t table, i;

    if (!fixed(mission, (uint16_t)(from + 037u), &word) || (word & 070000u) != 030000u) {
        printf("FAIL: DECDSP3's CAF SFOUTABR not found\n");
        exit(1);
    }
    table = (uint16_t)(word & 07777u);
    for (i = 0; i < 14u; ++i) {
        uint16_t bank;

        if (!fixed(mission, (uint16_t)(041u * 02000u + (table + i - 02000u)), &word)) {
            printf("FAIL: SFOUTABR word %u unreadable\n", i);
            exit(1);
        }
        bank = (uint16_t)((word >> 10) & 037u);
        if (bank >= 030u) bank = (uint16_t)(bank + 010u); /* the superbank of banks 40-47 */
        add_stop(mission, from, (uint16_t)(bank * 02000u + (word & 01777u)));
    }
}

/* PUTDECSF's step, like DSPDCPUT's, runs through PUTDCSF2's BANKJUMP
 * (INDEX A; CAF SFINTABR) to the scale factor routine's input routine and
 * stops there; so do PUTSFNOR's and PUTDCSF2's. The 14 CADRs of SFINTABR,
 * read from the rope (PUTDECSF is CHANLOAD +11, PUTSFNOR +21, PUTDCSF2 +22
 * and CAF SFINTABR +23). */
static void add_sfin_stops(enum lanec_mission mission)
{
    const uint16_t chanload = mission == LANEC_LM ? LANEC_LM_CHANLOAD : LANEC_CM_CHANLOAD;
    const uint16_t at = (uint16_t)(041u * 02000u + (chanload - 02000u));
    static const uint16_t starts[] = {11u, 21u, 22u};
    agc_word word = 0;
    uint16_t table, i, s;

    if (!fixed(mission, (uint16_t)(at + 23u), &word) || (word & 070000u) != 030000u) {
        printf("FAIL: PUTDCSF2's CAF SFINTABR not found\n");
        exit(1);
    }
    table = (uint16_t)(word & 07777u);
    for (i = 0; i < 14u; ++i) {
        uint16_t bank;

        if (!fixed(mission, (uint16_t)(041u * 02000u + (table + i - 02000u)), &word)) {
            printf("FAIL: SFINTABR word %u unreadable\n", i);
            exit(1);
        }
        bank = (uint16_t)((word >> 10) & 037u);
        if (bank >= 030u) bank = (uint16_t)(bank + 010u); /* the superbank of banks 40-47 */
        for (s = 0; s < sizeof starts / sizeof starts[0]; ++s)
            add_stop(mission, (uint16_t)(at + starts[s]),
                     (uint16_t)(bank * 02000u + (word & 01777u)));
    }
}

/* The load verbs' steps run to the next call and stop at the callee
 * (lanec_pin_head_load); COMPTEST's, NOUNTEST's and UPDATVB -1's steps
 * stop where they come back. The chains reach ABLOAD, BLOAD and CLOAD but
 * not verb 25's ABCLOAD, so its stops are read from the rope: from each
 * word the port enters, to the target of the first TC after it; and from
 * that target back to the word after the TC, for the TCs that come
 * straight back (all but REQDATX and REQDATY, whose steps end at
 * ENDOFJOB). */
static void add_abcload_stops(enum lanec_mission mission)
{
    static const uint16_t entered[] = {0u, 2u, 3u, 5u, 6u, 8u, 9u, 11u};
    static const bool back[] = {false, true, true, true, false, true, false, true};
    const uint16_t abcload = mission == LANEC_LM ? LANEC_LM_ABCLOAD : LANEC_CM_ABCLOAD;
    uint16_t previous = 0;
    unsigned i;

    for (i = 0; i < sizeof entered / sizeof entered[0]; ++i) {
        const uint16_t from = (uint16_t)(041u * 02000u + (abcload + entered[i] - 02000u));
        uint16_t at = from, target;
        agc_word word = 0;

        for (;; ++at) {
            if (at > from + 2u || !fixed(mission, at, &word)) {
                printf("FAIL: no TC after ABCLOAD +%u\n", entered[i]);
                exit(1);
            }
            if (!(word & 070000u) && (word & 07777u) >= 020u) break; /* a TC, not EXTEND */
        }
        target = (uint16_t)(word & 07777u);
        target = target >= 04000u ? target : (uint16_t)(041u * 02000u + (target - 02000u));
        add_stop(mission, from, target);
        if (i > 0u && back[i]) add_stop(mission, previous, from);
        previous = target;
    }
}

/* CHANLOAD's step stops at LOADLV (lanec_pin_chanload), which no chain
 * reaches. */
static void add_chanload_stop(enum lanec_mission mission)
{
    const uint16_t chanload = mission == LANEC_LM ? LANEC_LM_CHANLOAD : LANEC_CM_CHANLOAD;
    const uint16_t loadlv = mission == LANEC_LM ? LANEC_LM_LOADLV : LANEC_CM_LOADLV;

    add_stop(mission, (uint16_t)(041u * 02000u + (chanload - 02000u)),
             (uint16_t)(041u * 02000u + (loadlv - 02000u)));
}

/* VERBFAN's step for verbs 00-39 runs through VBFANDIR's BANKJUMP (INDEX
 * VERBREG; CAF VERBTAB) to the verb's routine and stops there. The chains
 * reach only some verbs, so the 40 CADRs of VERBTAB are read from the rope:
 * VBFANDIR is found in bank 41 by its three words, and VERBFAN is 11 words
 * before it (its TC VBFANDIR at VERBFAN +5, LST2CON just before VBFANDIR). */
static void add_verbtab_stops(enum lanec_mission mission)
{
    const uint16_t bankjump = mission == LANEC_LM ? LANEC_LM_BANKJUMP : LANEC_CM_BANKJUMP;
    const uint16_t verbreg = 01001u;
    uint16_t at, i;

    for (at = 041u * 02000u + 11u; at < 041u * 02000u + 01775u; ++at) {
        agc_word index = 0, caf = 0, tc = 0, back = 0;
        const uint16_t here = (uint16_t)(02000u + (at & 01777u));

        if (!fixed(mission, at, &index) || index != 050000u + verbreg) continue;
        if (!fixed(mission, (uint16_t)(at + 1u), &caf) || caf != 030000u + here + 3u) continue;
        if (!fixed(mission, (uint16_t)(at + 2u), &tc) || tc != bankjump) continue;
        if (!fixed(mission, (uint16_t)(at - 6u), &back) || back != here) {
            printf("FAIL: VERBFAN's TC VBFANDIR not found\n");
            exit(1);
        }
        for (i = 0; i < 40u; ++i) {
            agc_word word = 0;
            uint16_t bank;

            if (!fixed(mission, (uint16_t)(at + 3u + i), &word)) {
                printf("FAIL: VERBTAB word %u unreadable\n", i);
                exit(1);
            }
            bank = (uint16_t)((word >> 10) & 037u);
            if (bank >= 030u) bank = (uint16_t)(bank + 010u); /* the superbank of banks 40-47 */
            add_stop(mission, (uint16_t)(at - 11u), (uint16_t)(bank * 02000u + (word & 01777u)));
        }
        return;
    }
    printf("FAIL: VBFANDIR not found\n");
    exit(1);
}

/* ENTERJMP's step for a pass 0 ENTER (POSTJUMP to ENTER, then CCS REQRET
 * and TC ENTPAS0) stops at ENTPAS0, which the chains, starting ENTER keys
 * at ENTER, do not show: ENTPAS0 is read from ENTER +5. */
static void add_enter_stop(enum lanec_mission mission)
{
    const uint16_t enterjmp = mission == LANEC_LM ? LANEC_LM_ENTERJMP : LANEC_CM_ENTERJMP;
    const uint16_t enter = mission == LANEC_LM ? LANEC_LM_ENTER : LANEC_CM_ENTER;
    agc_word word = 0;

    if (!fixed(mission, (uint16_t)(041u * 02000u + (enter + 5u - 02000u)), &word) ||
        (word & 070000u)) {
        printf("FAIL: ENTER's TC ENTPAS0 not found\n");
        exit(1);
    }
    add_stop(mission, (uint16_t)(040u * 02000u + (enterjmp - 02000u)),
             (uint16_t)(041u * 02000u + ((word & 07777u) - 02000u)));
}

static void write_stops(const char *path)
{
    FILE *out = fopen(path, "w");
    unsigned m, i;
    if (!out) {
        printf("FAIL: cannot write %s\n", path);
        exit(1);
    }
    fprintf(out,
            "/* Generated by readable/tests/pinball_steps.c --stops from the faithful port's\n"
            " * Pinball chains (pin-transfer, pin-decdsp3, the load and input scale suites), "
            "CHARIN's key table, SFOUTABR, SFINTABR, ABCLOAD, CHANLOAD, VERBTAB and ENTER: where "
            "each\n"
            " * faithful step that starts at a Pinball point ends, as {start, end} physical\n"
            " * words (fixed_physical; an erasable end 0176000 + its address); and the points\n"
            " * of the faithful port's Pinball routine tables, the file's code as far as the\n"
            " * port enters it. Do not edit. */\n");
    add_charin_stops(LANEC_LM);
    add_charin_stops(LANEC_CM);
    add_sfout_stops(LANEC_LM);
    add_sfout_stops(LANEC_CM);
    add_abcload_stops(LANEC_LM);
    add_abcload_stops(LANEC_CM);
    add_chanload_stop(LANEC_LM);
    add_chanload_stop(LANEC_CM);
    add_sfin_stops(LANEC_LM);
    add_sfin_stops(LANEC_CM);
    add_verbtab_stops(LANEC_LM);
    add_verbtab_stops(LANEC_CM);
    add_enter_stop(LANEC_LM);
    add_enter_stop(LANEC_CM);
    for (m = 0; m < 2u; ++m) {
        qsort(stops[m], stop_count[m], sizeof stops[m][0], by_stop);
        fprintf(out, "%s\nstatic const struct pinball_stop pinball_stops[%u] = {\n",
                m ? "#else" : "#if LUMINARY099", stop_count[m]);
        for (i = 0; i < stop_count[m]; ++i)
            fprintf(out, "    {0%06o, 0%06o},\n", stops[m][i].from, stops[m][i].to);
        fprintf(out, "};\n");
        write_points(out, m);
    }
    fprintf(out, "#endif\n");
    fclose(out);
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
        if (called && collect) {
            if (fs != EXEC_OK) return;
            add_stop(mission, physical(&from, io), physical(&f, &f_io));
        } else if (called) {
            readable = 1;
            called = 0;
            rs = lanec_pin_chain_max(&r, &r_io, mission, (uint16_t)(AGC_TRANSFER_NONE - 1u), 1u,
                                     &rsteps);
            if (fs != EXEC_OK) {
                readable_more += rs == EXEC_OK;
                return;
            }
            ++steps_compared;
            if (getenv("PINBALL_STEPS_PAIRS")) /* each faithful step's start and end */
                printf("PAIR %s %s %05o %05o %05o %05o\n", suite, mission == LANEC_LM ? "LM" : "CM",
                       from.words[4], from.words[LANEC_Z], f.words[4], f.words[LANEC_Z]);
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

int main(int argc, char **argv)
{
    static const struct {
        const char *name;
        int held; /* the steps must be the faithful port's */
    } suites[] = {{"pin-transfer", 1},
                  {"pin-decdsp3", 1},
                  {"pin-stirred", 0},
                  /* The load verbs and the input scale routines (part 4), which the
                   * chains above hardly reach: the faithful modules' own suites. */
                  {"pin-head-load", 1},
                  {"pw-load-request", 1},
                  {"pw-load-complete", 1},
                  {"pw-load-control", 1},
                  {"pw-load-store", 1},
                  {"pw-decimal-load", 1},
                  {"pw-input-mode", 1},
                  {"pw-input-select", 1},
                  {"pw-input-store", 1},
                  {"pw-input-factor", 1},
                  {"pw-input-angle", 1},
                  {"pw-input-arithmetic", 1},
                  {"pw-input-scale", 1},
                  {"pw-input-time", 1},
                  /* Monitor, DSPFMEM and the alarms (part 5). */
                  {"pw-monitor", 1},
                  {"pw-monitor-consumer", 1},
                  {"pw-monitor-paste", 1},
                  {"pw-nv-consumer", 1},
                  {"pw-fixed-display", 1},
                  {"pw-source-alarm", 1},
                  {"pw-alarm-cycle", 1},
                  {"pw-alarm-recycle", 1}};
    static apollo_overlay_state s;
    static lanec_io io;
    static lanec_meta m;
    const char *only = getenv("PINBALL_STEPS_CASE"); /* SUITE:CASE, for a debugger */
    unsigned i;
    uint32_t n;
    int bad = 0;

    lanec_fixed_read = fixed;
    collect = argc == 3 && !strcmp(argv[1], "--stops");
    for (i = 0; i < sizeof suites / sizeof suites[0]; ++i) {
        const lanec_suite *suite = lanec_find_suite(suites[i].name);

        if (!suite) {
            printf("FAIL: no suite %s\n", suites[i].name);
            return 1;
        }
        if (collect && !suites[i].held) continue;
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
        if (collect) continue;
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
    if (collect) write_stops(argv[2]);
    return bad;
}
