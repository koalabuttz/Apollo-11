/* SPDX-License-Identifier: MIT */
/* The CM's readable EXTENDED_VERBS as one program, for the tests of part 9
 * (make extended-verbs-cases): the file's two tables (src/binding/
 * extended_verbs_routines.c), and the parts' basic words at the keys the
 * port's tables list for them (c64-port/src/lanec_ev_routines.inc), as
 * the CM flight program enters them.
 *
 * In flight each part's word comes through the port's table, which hands
 * the part the program's channels 11-14 from its cm_native_io and 30-33
 * from its agc_hardware (flight_io.c's cm_dntm_io). Here the run's
 * cm_native_io and agc_hardware hold them, as the test harness gives them
 * (tests/interp_case.c), and parts() hands them over the same way.
 *
 * Two words outside the file are in the program too, as the compiled
 * file runs them (its spec, c64-port/tools/interp_specs.py): INTPRET,
 * which the file's TC INTPRETs go on into, the readable interpreter's
 * (src/binding/interpreter_basic.c, which reads the rope: the program
 * loads it, CM_ROPE from the Makefile); and VERB69, a TC to itself until
 * the hardware restarts the computer, which ends a run there: run once
 * where a run starts at it, and not at all where a run reaches it.
 *
 * And the parts take the compiled file's latches for its basic words (all
 * in banks 42 and 43): interrupts on or off, the superbank 0100. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "extended_verbs_file_program.h"
#include "cm_extended_verbs_file.h"
#include "cm_native_image.h"
#include "rope_image.h"
#include "lanec_ev_align.h"
#include "lanec_ev_dap.h"
#include "lanec_ev_front.h"
#include "lanec_ev_short.h"
#include "lanec_ev_time.h"
#include "lanec_ev_track.h"

typedef enum lanec_status (*dispatch)(apollo_overlay_state *, lanec_io *, enum lanec_mission);

static enum exec_status parts(const agc_native_program *p, apollo_overlay_state *s, void *v,
                              dispatch part)
{
    cm_native_io *io = v;
    lanec_io l;

    if (!p || !p->hardware || !s || !io) return EXEC_OUT_OF_SCOPE;
    if (io->interrupt_enabled > 1u || io->superbank != 0100u) return EXEC_OUT_OF_SCOPE;
    memset(&l, 0, sizeof l);
    l.superbank = io->superbank;
    l.interrupt_enabled = io->interrupt_enabled;
    l.channel[011] = io->channel11;
    l.channel[012] = io->channel12;
    l.channel[013] = io->channel13;
    l.channel[014] = io->channel14;
    l.channel[030] = p->hardware->channel30;
    l.channel[031] = p->hardware->channel31;
    l.channel[032] = p->hardware->channel32;
    l.channel[033] = p->hardware->channel33;
    if (part(s, &l, LANEC_CM) != LANEC_OK) return EXEC_OUT_OF_SCOPE;
    io->superbank = l.superbank;
    io->interrupt_enabled = l.interrupt_enabled;
    io->channel11 = l.channel[011];
    io->channel12 = l.channel[012];
    io->channel13 = l.channel[013];
    io->channel14 = l.channel[014];
    p->hardware->channel30 = l.channel[030];
    p->hardware->channel31 = l.channel[031];
    p->hardware->channel32 = l.channel[032];
    p->hardware->channel33 = l.channel[033];
    return EXEC_OK;
}

#define CM(m)                                                                                      \
    static enum exec_status m##_cm(const agc_native_program *p, apollo_overlay_state *s, void *v)  \
    {                                                                                              \
        return parts(p, s, v, lanec_ev_##m##_dispatch);                                            \
    }
CM(front)
CM(short) CM(align) CM(dap) CM(time) CM(track)
/* The LM's keys in the same list name these; the CM's program never
 * reaches them. */
#define front_lm NULL
#define short_lm NULL
#define align_lm NULL
#define radar_lm NULL
#define dap_lm   NULL
#define time_lm  NULL
#define track_lm NULL
#define lmi_lm   NULL
#include "lanec_ev_routines.inc"

    static const agc_routine_table cm_parts = {cm_entries, LANEC_EV_CM_ROUTINES};
extern const agc_routine_table cm_interpreter_basic;

/* INTPRET (06006, and +2 where it goes on after RELINT), from the
 * interpreter's table. */
#define INTPRET_KEY (2048u + 06006u - 04000u)
static agc_routine_entry intpret_entries[2];
static const agc_routine_table intpret = {intpret_entries, 2u};

static void find_intpret(void)
{
    unsigned i, k;

    for (k = 0; k < 2u; ++k) {
        intpret_entries[k].physical = (uint16_t)(INTPRET_KEY + 2u * k);
        for (i = 0; i < cm_interpreter_basic.count; ++i)
            if (cm_interpreter_basic.entries[i].physical == intpret_entries[k].physical)
                intpret_entries[k].call = cm_interpreter_basic.entries[i].call;
    }
}

/* VERB69 (43,2037), TC VERB69: the end of a run. Reached from GOEXTVB's
 * fan, Q is the fan's return (GOEXTVB +1), or from the TC itself, its own
 * (VERB69 +1); from anywhere else the run starts here, and the TC runs
 * once (the readable verb69(), part 7's). */
static enum exec_status idle(const agc_native_program *p, apollo_overlay_state *s, void *v)
{
    if (s && s->words[2] != 02002u && s->words[2] != 02040u &&
        parts(p, s, v, lanec_ev_track_dispatch) != EXEC_OK)
        return EXEC_OUT_OF_SCOPE;
    return EXEC_IDLE;
}
static const agc_routine_entry verb69_entry[1] = {{043u * 1024u + 037u, idle}};
static const agc_routine_table verb69 = {verb69_entry, 1u};

void readable_cm_file_program(agc_native_program *p, agc_hardware *hardware)
{
    /* The file's table, VERB69, the parts', INTPRET. */
    static agc_routine_table basic[4];
    static bool loaded;

    if (!loaded && !(loaded = apollo_rope_load(1, CM_ROPE))) {
        fprintf(stderr, "no %s: run c64-port/tools/setup_cloud.sh\n", CM_ROPE);
        exit(2);
    }

    (void)lm_entries;
    (void)lm_interp_entries;
    find_intpret();
    basic[0] = cm_extended_verbs_file_basic;
    basic[1] = verb69;
    basic[2] = cm_parts;
    basic[3] = intpret;
    memset(p, 0, sizeof *p);
    p->basic = basic;
    p->basic_count = 4;
    p->interpretive = &cm_extended_verbs_file_interpretive;
    p->interpretive_count = 1;
    p->interpreter_fetch = (uint16_t)(2048u + 06037u - 04000u);
    p->loc = 0164;
    p->bankset = 0165;
    p->hardware = hardware;
}
