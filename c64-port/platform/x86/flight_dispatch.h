/* SPDX-License-Identifier: MIT */
#ifndef FLIGHT_DISPATCH_H
#define FLIGHT_DISPATCH_H
/* The native x86_64 flight program's step (docs/X86-FLIGHT.md): every
 * lane's routine tables (build/flight-store/<m>-host.inc, the order
 * tools/flight_store_build.py resolves them in) behind two direct-mapped
 * point indexes, one basic and one interpretive. A step is a boundary
 * (the clock periods due, then rk_rupt_take) and one routine. An
 * interpretive point no compiled table holds, or whose routine refuses, runs
 * one slot of the native interpreter core (lm_interp_core/cm_interp_core)
 * instead; each such point is counted for the report. Built once per mission
 * (RK_LM or RK_CM). */
#include "flight_io.h"
#include "resident_rupt.h"

/* X86_INTERP_COMPILED (default): compiled interpretive tables first, the
 * interpreter core where none holds the point. X86_INTERP_CORE: no
 * interpretive table; the core runs every interpretive point. */
enum { FD_INTERP_COMPILED, FD_INTERP_CORE };

typedef struct {
    const agc_routine_table *t; const char *name; uint8_t interp; enum flight_io_kind io;
    const agc_site_operand *sites; uint16_t nsites; uint8_t fixed;
} fd_table;
/* Each table's certification (build/x86-flight/<m>-status.inc): 'c' a
 * complete file's, 'u' a file's not yet complete, 'x' shared code. With
 * X86_CERTIFIED_ONLY the 'u' tables are not linked into the indexes. */
char fd_table_status(unsigned table);

/* A point the core ran, or no routine held: key, kind, arrivals, slots. */
typedef struct { uint16_t key; uint8_t interp, refused; unsigned long arrivals, slots; const char *by; } fd_gap;
enum { FD_GAPS = 1024 };

typedef struct {
    apollo_overlay_state s, undo;
    flight_io io;
    agc_hardware hw;
    agc_native_program p;
    uint16_t pending;
    uint8_t mode;
    /* FLIGHT_DOWNRUPT: a DOWNRUPT request each time channel 4 passes a
     * multiple of 64 counts (20 ms), as tools/flight_downlink.h does. */
    uint8_t downrupt;
    /* The virtual AGC clock, in machine cycles (85,333 1/3 a second): each
     * routine adds its cost (fd_cost); every 853 1/3 is one clock period,
     * in three steps (rk_timer_phase: 2.5, 2.5 and 5 ms), run at the next
     * boundary. */
    double mct, next_period; /* next_period: the end of the next clock step */
    uint8_t phase;           /* that step (rk_timer_phase) */
    unsigned long periods, steps, slots;
    /* AGC time counts from here (TIME2/TIME1 in centiseconds): 0 from
     * GOJAM, a start state's own time from X86_START. */
    unsigned long t0;
    fd_gap gaps[FD_GAPS];
    unsigned ngaps;
    unsigned long unmapped;   /* basic points with no routine: fatal */
    unsigned long uses[256];  /* routines run, by table */
} fd_state;

/* What the last fd_run ran: the table (NULL for the core) and the point. */
typedef struct { const fd_table *table; uint16_t key; uint8_t interp; enum exec_status status; double mct; } fd_ran;

extern const fd_table fd_tables[];
extern const unsigned fd_table_count;

/* Cold start at GOJAM (Z 04000, interrupts enabled, input channels 30-33 as
 * given, everything else zero). The rope must be loaded (apollo_rope_load). */
void fd_init(fd_state *st, uint8_t mode, agc_word ch30, agc_word ch31);
/* The clock steps due (rk_timer_phase each), with rk_rupt_take before
 * each, as the flight resident's rk_rupts: a request that can be taken is
 * taken before the later steps raise theirs. Returns the interrupt it took
 * (0 for none). */
uint8_t fd_boundary(fd_state *st);
/* One routine (or one interpreter slot) from the point the state is at.
 * EXEC_UNMAPPED: a basic point with no routine (the program stops). */
enum exec_status fd_run(fd_state *st, fd_ran *ran);
/* The point at the state: physical key, interpretive or not. */
uint16_t fd_point(const fd_state *st, int *interp);
/* MODREG, the time in centiseconds (TIME2:TIME1). */
/* AGC time in centiseconds since the start (st->t0). */
unsigned long fd_centiseconds(const fd_state *st);
#endif
