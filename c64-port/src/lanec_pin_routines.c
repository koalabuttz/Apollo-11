/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_pin_routines.h"
#include "lanec_pw_runs.h"
#include "lanec_pin_gaps.h"
#include "lm_downtelemetry.h"
#include "cm_downtelemetry.h"

typedef enum lanec_status (*dispatch)(apollo_overlay_state *, lanec_io *, enum lanec_mission);
static lanec_io io;                 /* the trampoline runs one routine at a time */
#ifndef LANEC_PIN_C64
static apollo_overlay_state entry;  /* the state each claiming module starts from */
#define LANEC_PIN_SAVE(s) (entry = *(s))
#define LANEC_PIN_RESTORE(s) (*(s) = entry)
#else   /* one module, which leaves a refused state unchanged; a result the record
         * cannot carry is refused changed, and the caller makes the case again */
#define LANEC_PIN_SAVE(s) ((void)0)
#define LANEC_PIN_RESTORE(s) ((void)0)
#endif

const uint16_t lanec_pin_cells[LANEC_PIN_CELLS] = { 072u, 0114u, 0115u, 0116u, 0117u, 0124u, 0125u, 0136u, 0144u, 0154u, 0156u,
                                                     0162u, 01013u };

/* A module that ends at a register or a return cell holding a TC (not to
 * itself, and not RELINT, INHINT or EXTEND) or a TCF into fixed memory:
 * execute that instruction. */
static void cell_return(apollo_overlay_state *s)
{
    uint16_t z = s->words[5], i, cell = z < 8u, w;
    for (i = 0; i < LANEC_PIN_CELLS; ++i)
        if (lanec_pin_cells[i] == z) cell = 1;
    if (!cell) return;
    w = s->words[z];
    if (w <= 07777u && w != 3u && w != 4u && w != 6u && w != z) {   /* TC */
        s->words[5] = w;
        s->words[2] = (uint16_t)(z + 1u);
    } else if (w >= 012000u && w <= 017777u) {  /* TCF */
        s->words[5] = (uint16_t)(w & 07777u);
    }
}

/* A channel the device record does not carry was written (CHANLOAD's
 * WRITE to any channel): the routine cannot give that result. */
static int unrepresented(uint16_t last)
{
    uint16_t c;
    for (c = 0; c < LANEC_CHANNELS; ++c)
        if (io.channel[c] && c != 1u && c != 2u && c != 7u && (c < 011u || c > last)) return 1;
    return 0;
}

/* The program's fixed memory for the modules that read the rope as data
 * (lanec_fixed_read); a program without fixed_data keeps the harness's. */
static const agc_native_program *program;
static bool program_fixed(enum lanec_mission mission, uint16_t physical, agc_word *word)
{
    (void)mission;
    return program->fixed_data(program, physical, word);
}
static void use_program(const agc_native_program *p)
{
    if (p->fixed_data) { program = p; lanec_fixed_read = program_fixed; }
}

#ifdef LANEC_PIN_HAS_LM
static enum exec_status lm(const agc_native_program *p, apollo_overlay_state *s, void *v, const dispatch *f)
{
    lm_dntm_io *x = v;
    if (!LANEC_MISSION_LINKED(LANEC_LM) || !p || !s || !x || !p->hardware || !lanec_registers_valid(s)) return EXEC_OUT_OF_SCOPE;
    use_program(p);
    LANEC_PIN_SAVE(s);
    for (; *f; ++f) {
        memset(&io, 0, sizeof io);
        io.superbank = x->control.superbank; io.interrupt_enabled = x->control.interrupt_enabled;
        io.channel[011] = p->hardware->channel11;
        io.channel[012] = x->channel12; io.channel[013] = x->channel13; io.channel[014] = x->channel14;
        if ((*f)(s, &io, LANEC_LM) == LANEC_OK && !unrepresented(014u)) {
            x->control.superbank = io.superbank; x->control.interrupt_enabled = io.interrupt_enabled;
            p->hardware->channel11 = x->channel11 = io.channel[011];
            x->channel12 = io.channel[012]; x->channel13 = io.channel[013]; x->channel14 = io.channel[014];
            cell_return(s);
            return EXEC_OK;
        }
        LANEC_PIN_RESTORE(s);
    }
    return EXEC_OUT_OF_SCOPE;
}

#endif

#ifdef LANEC_PIN_HAS_CM
static enum exec_status cm(const agc_native_program *p, apollo_overlay_state *s, void *v, const dispatch *f)
{
    cm_dntm_io *x = v;
    if (!LANEC_MISSION_LINKED(LANEC_CM) || !p || !s || !x || !lanec_registers_valid(s)) return EXEC_OUT_OF_SCOPE;
    use_program(p);
    LANEC_PIN_SAVE(s);
    for (; *f; ++f) {
        memset(&io, 0, sizeof io);
        io.superbank = x->io.superbank; io.interrupt_enabled = x->io.interrupt_enabled;
        io.channel[011] = x->io.channel11; io.channel[012] = x->io.channel12;
        io.channel[013] = x->io.channel13; io.channel[014] = x->io.channel14;
        io.channel[015] = x->io.main_keys; io.channel[016] = x->io.navigation_keys;
        if ((*f)(s, &io, LANEC_CM) == LANEC_OK && !unrepresented(016u)) {
            x->io.superbank = io.superbank; x->io.interrupt_enabled = io.interrupt_enabled;
            x->io.channel11 = io.channel[011]; x->io.channel12 = io.channel[012];
            x->io.channel13 = io.channel[013]; x->io.channel14 = io.channel[014];
            x->io.main_keys = io.channel[015]; x->io.navigation_keys = io.channel[016];
            cell_return(s);
            return EXEC_OK;
        }
        LANEC_PIN_RESTORE(s);
    }
    return EXEC_OUT_OF_SCOPE;
}
#endif

#ifndef LANEC_PIN_C64
/* The return point after one of the file's BANKCALLs, as INTER-BANK's
 * SWRETURN leaves it (Q = BUF2 + 1): the earlier modules enter there only at
 * SWRETURN, so give the state back to SWRETURN (BUF2 = this point, BUF2 + 1
 * = its bank, FB = the bank SWRETURN put back in BUF2 + 1), which then
 * produces exactly this state again, and run them from there. */
static enum exec_status swreturn(const agc_native_program *p, apollo_overlay_state *s, void *v, const dispatch *f,
                                 enum exec_status (*run)(const agc_native_program *, apollo_overlay_state *, void *,
                                                         const dispatch *),
                                 uint16_t at)
{
    static apollo_overlay_state back;
    uint16_t *w = s->words;
    enum exec_status status;
    if (w[2] != 0134u || (w[0134] & ~076000u) || (w[4] & ~076000u)) return EXEC_OUT_OF_SCOPE;
    back = *s;
    w[0133] = w[5]; w[0134] = back.words[4];
    w[4] = back.words[0134]; w[6] = (uint16_t)(w[4] | (w[3] >> 8));
    w[5] = at;
    status = run(p, s, v, f);
    if (status != EXEC_OK) *s = back;
    return status;
}
#endif

#include "lanec_pin_routines.inc"
#ifndef LANEC_PIN_C64
const agc_routine_table lanec_pin_lm_routines = { lm_entries, LANEC_PIN_LM_ROUTINES };
const agc_routine_table lanec_pin_cm_routines = { cm_entries, LANEC_PIN_CM_ROUTINES };
#endif

/* The C64 round trips: one module's points, running it alone. */
static dispatch c64_run[2];
#ifdef LANEC_PIN_HAS_LM
static enum exec_status c64_lm(const agc_native_program *p, apollo_overlay_state *s, void *v) { return lm(p, s, v, c64_run); }
#endif
#ifdef LANEC_PIN_HAS_CM
static enum exec_status c64_cm(const agc_native_program *p, apollo_overlay_state *s, void *v) { return cm(p, s, v, c64_run); }
#endif
static agc_routine_entry c64_entries[LANEC_PIN_C64_MAX];   /* the most points one module claims first */
agc_routine_table lanec_pin_c64 = { c64_entries, 0 };

const lanec_pin_point *lanec_pin_c64_points(int cm, uint16_t *count)
{
    *count = cm ? LANEC_PIN_CM_C64_POINTS : LANEC_PIN_LM_C64_POINTS;
    return cm ? c64_cm_points : c64_lm_points;
}

void lanec_pin_c64_select(uint8_t module, int cm)
{
    uint16_t i, n, k = 0;
    const lanec_pin_point *p = lanec_pin_c64_points(cm, &n);
#ifdef LANEC_PIN_C64
    c64_run[0] = LANEC_PIN_C64_RUN;
#else
    c64_run[0] = c64_runs[module];
#endif
    for (i = 0; i < n; ++i)
        if (p[i].module == module) {
            c64_entries[k].physical = p[i].physical;
#if defined(LANEC_PIN_HAS_LM) && defined(LANEC_PIN_HAS_CM)
            c64_entries[k++].call = cm ? c64_cm : c64_lm;
#elif defined(LANEC_PIN_HAS_CM)
            c64_entries[k++].call = c64_cm;
#else
            c64_entries[k++].call = c64_lm;
#endif
        }
    lanec_pin_c64.count = k;
}
