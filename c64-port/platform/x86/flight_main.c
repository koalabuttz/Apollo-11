/* SPDX-License-Identifier: MIT */
/* The native x86_64 flight program (docs/X86-FLIGHT.md): Luminary099
 * (RK_LM) or Comanche055 (RK_CM) from GOJAM, every lane's native routines
 * through platform/x86/flight_dispatch.c, interpretive code compiled
 * (X86_INTERP_MODE FD_INTERP_CORE: through the interpreter core), on the
 * virtual AGC clock.
 *
 *   x86-flight-<m> ROPE.bin [CS [KEYS]]
 *
 * Runs CS centiseconds of AGC time (default 1000). KEYS (e.g. V37E12E) are
 * DSKY keys, @CS timing them. Scenario inputs FLIGHT_CH30-33, FLIGHT_POKE,
 * FLIGHT_LOAD, FLIGHT_GOJAM and FLIGHT_DOWNRUPT: platform/x86/
 * flight_scenario.h (with the radars and X86_START). Environment:
 *   X86_SPEED     AGC time per wall time: 1 real time (default), 0 flat out
 *   X86_REPORT    a JSON report: points the interpreter core ran (and why),
 *                 basic points no routine holds, steps, clock
 *   X86_SAMPLES   each centisecond's erasable memory at the first idle
 *                 ADVAN at or after it, one line of 2048 octal words
 *   X86_SAMPLES_FROM  the first centisecond X86_SAMPLES writes (default 1)
 *   X86_DSKY=1    a live DSKY in the terminal (platform/x86/terminal_dsky.h);
 *                 with no CS argument the run goes on until q
 *   X86_TYPED     characters sent through the live DSKY's typed-key path
 *                 from the start, as if typed (the lockstep check takes the
 *                 same) */
#include <string.h>
#include <time.h>
#include "flight_scenario.h"
#include "terminal_dsky.h"
#include "rope_image.h"
#if RK_LM
#define MISSION 0u
#else
#define MISSION 1u
#endif
#ifndef X86_INTERP_MODE
#define X86_INTERP_MODE FD_INTERP_COMPILED
#endif

static double now_s(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (double)t.tv_sec+(double)t.tv_nsec*1e-9;}

static void report(FILE *f,const fd_state *st,unsigned long cs,double wall){unsigned i,n=0;
 fprintf(f,"{\n \"mission\": \"%s\",\n \"interp\": \"%s\",\n \"centiseconds\": %lu,\n \"steps\": %lu,\n \"core_slots\": %lu,\n"
  " \"periods\": %lu,\n \"wall_seconds\": %.3f,\n \"major_mode\": \"%02o\",\n \"failreg\": \"%05o\",\n \"unmapped\": %lu,\n \"gaps\": [",
  MISSION?"cm":"lm",st->mode==FD_INTERP_CORE?"core":"compiled",cs,st->steps,st->slots,st->periods,wall,
  (unsigned)st->s.words[01011],(unsigned)st->s.words[0375],st->unmapped);
 for(i=0;i<st->ngaps;++i){const fd_gap *g=&st->gaps[i];
  fprintf(f,"%s\n  {\"kind\": \"%s\", \"point\": \"%02o,%04o\", \"arrivals\": %lu, \"slots\": %lu, \"refused_by\": %s%s%s}",i?",":"",
   g->interp?"interp":"basic",g->key/1024u,g->key%1024u+02000u,g->arrivals,g->slots,g->by?"\"":"",g->by?g->by:"null",g->by?"\"":"");}
 fprintf(f,"\n ],\n \"tables\": [");
 for(i=0;i<fd_table_count&&i<256u;++i)if(st->uses[i])fprintf(f,"%s\n  {\"table\": \"%s\", \"status\": \"%s\", \"routines\": %lu}",n++?",":"",fd_tables[i].name,
  fd_table_status(i)=='c'?"complete":fd_table_status(i)=='u'?"uncertified":"shared",st->uses[i]);
 fprintf(f,"\n ]\n}\n");}

int main(int argc, char **argv)
{
    static fd_state st;
    static fs_inputs in;
    static terminal_dsky dsky;
    const int live = getenv("X86_DSKY") && *getenv("X86_DSKY") == '1';
    const char *typed = getenv("X86_TYPED");
    fd_ran ran;
    unsigned long cs = argc > 2 ? strtoul(argv[2], 0, 0) : 1000ul, next = 1, periods = 0,
                  from =
                      getenv("X86_SAMPLES_FROM") ? strtoul(getenv("X86_SAMPLES_FROM"), 0, 10) : 1ul;
    double speed = getenv("X86_SPEED") ? atof(getenv("X86_SPEED")) : 1.0, t0, wall;
    FILE *samples = getenv("X86_SAMPLES") ? fopen(getenv("X86_SAMPLES"), "w") : 0;
    if (argc < 2 || !apollo_rope_load(MISSION, argv[1])) {
        fprintf(stderr, "usage: x86-flight ROPE.bin [CS [KEYS]]\n");
        return 2;
    }
    if (!fs_parse(&in, argc > 3 ? argv[3] : "")) return 2;
    if (live) {
        if (argc <= 2) cs = ~0ul;
        td_start(&dsky);
        dsky.quit_at_eof = argc <= 2;
    } else
        td_init(&dsky);
    for (; typed && *typed; ++typed) td_type(&dsky, (unsigned char)*typed);
    if (!fs_start(&st, X86_INTERP_MODE)) return 2;
    if (getenv("X86_START")) fs_inputs_from(&in, fd_centiseconds(&st));
    t0 = now_s();
    while (fd_centiseconds(&st) < cs) {
        fd_boundary(&st);
        /* Real time: AGC time runs no faster than wall time times X86_SPEED. */
        if (speed > 0.0 && st.periods != periods) {
            double ahead;
            periods = st.periods;
            if ((ahead = (double)periods / 100.0 / speed - (now_s() - t0)) > 0.0) {
                struct timespec d;
                d.tv_sec = (time_t)ahead;
                d.tv_nsec = (long)((ahead - (double)d.tv_sec) * 1e9);
                nanosleep(&d, 0);
            }
        }
        if (fs_sample_point(&in, &st)) {
            unsigned long t = fd_centiseconds(&st);
            for (; next <= cs && t >= next; ++next)
                if (samples && next >= from) {
                    unsigned i;
                    fprintf(samples, "%lu", next);
                    for (i = 0; i < 2048u; ++i) fprintf(samples, " %05o", (unsigned)st.s.words[i]);
                    fputc('\n', samples);
                }
            if (fs_inputs_due(&in, &st)) continue;
            if (live) td_read_keys(&dsky);
            if (dsky.quit) break;
            if (td_inputs_due(&dsky, &st)) continue;
        }
        if (fd_run(&st, &ran) == EXEC_UNMAPPED) {
            if (live) td_restore(); /* the stop message stays on screen */
            fprintf(stderr, "no routine at basic %02o,%04o (Z %05o FB %05o) at %lu cs\n",
                    ran.key / 1024u, ran.key % 1024u + 02000u, (unsigned)st.s.words[5],
                    (unsigned)st.s.words[4], fd_centiseconds(&st));
            break;
        }
        if (ran.status != EXEC_OK) {
            if (live) td_restore(); /* the stop message stays on screen */
            fprintf(stderr, "%s refused at %02o,%04o (status %d) at %lu cs\n",
                    ran.table ? ran.table->name : "interpreter core", ran.key / 1024u,
                    ran.key % 1024u + 02000u, (int)ran.status, fd_centiseconds(&st));
            break;
        }
        fs_after_routine(&in, &st);
        if (live) {
            td_after_routine(&dsky, &st);
            td_draw(&dsky, &st);
        }
    }
    if (live) td_restore();
    wall = now_s() - t0;
    if (samples) fclose(samples);
    printf("%s: %lu cs of AGC time in %.2f s; %lu routines, %lu interpreter core slots, %lu clock "
           "periods; major mode %02o, FAILREG %05o\n",
           MISSION ? "CM" : "LM", fd_centiseconds(&st), wall, st.steps - st.slots, st.slots,
           st.periods, (unsigned)st.s.words[01011], (unsigned)st.s.words[0375]);
    {
        unsigned i, n = 0;
        unsigned long r = 0;
        for (i = 0; i < fd_table_count && i < 256u; ++i)
            if (st.uses[i] && fd_table_status(i) == 'u') {
                ++n;
                r += st.uses[i];
            }
        if (n)
            printf(
                "%lu routines from %u tables of files not yet certified (X86_REPORT lists them)\n",
                r, n);
    }
    if (st.ngaps) {
        unsigned i;
        printf("points without a compiled routine (%u):\n", st.ngaps);
        for (i = 0; i < st.ngaps; ++i)
            printf("  %s %02o,%04o: %lu arrivals, %lu slots%s%s\n",
                   st.gaps[i].interp ? "interp" : "basic ", st.gaps[i].key / 1024u,
                   st.gaps[i].key % 1024u + 02000u, st.gaps[i].arrivals, st.gaps[i].slots,
                   st.gaps[i].by ? ", refused by " : "", st.gaps[i].by ? st.gaps[i].by : "");
    }
    if (getenv("X86_REPORT")) {
        FILE *f = fopen(getenv("X86_REPORT"), "w");
        if (f) {
            report(f, &st, live ? fd_centiseconds(&st) : cs, wall);
            fclose(f);
        }
    }
    /* A live run the user quit ran as long as they wanted. */
    return st.unmapped || (fd_centiseconds(&st) < cs && !dsky.quit) ? 1 : 0;
}
