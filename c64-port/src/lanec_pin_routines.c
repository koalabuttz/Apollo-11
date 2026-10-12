/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "lanec_pin_routines.h"
#include "lanec_pw_runs.h"
#include "lanec_pin_gaps.h"
#include "lm_downtelemetry.h"
#include "cm_downtelemetry.h"

typedef enum lanec_status (*dispatch)(apollo_overlay_state *, lanec_io *, enum lanec_mission);
static lanec_io io;                 /* the trampoline runs one routine at a time */
static apollo_overlay_state entry;  /* the state each claiming module starts from */

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

static enum exec_status lm(const agc_native_program *p, apollo_overlay_state *s, void *v, const dispatch *f)
{
    lm_dntm_io *x = v;
    if (!p || !s || !x || !p->hardware) return EXEC_OUT_OF_SCOPE;
    entry = *s;
    for (; *f; ++f) {
        memset(&io, 0, sizeof io);
        io.superbank = x->control.superbank; io.interrupt_enabled = x->control.interrupt_enabled;
        io.channel[011] = p->hardware->channel11;
        io.channel[012] = x->channel12; io.channel[013] = x->channel13; io.channel[014] = x->channel14;
        if ((*f)(s, &io, LANEC_LM) == LANEC_OK && !unrepresented(014u)) {
            x->control.superbank = io.superbank; x->control.interrupt_enabled = io.interrupt_enabled;
            p->hardware->channel11 = io.channel[011];
            x->channel12 = io.channel[012]; x->channel13 = io.channel[013]; x->channel14 = io.channel[014];
            cell_return(s);
            return EXEC_OK;
        }
        *s = entry;
    }
    return EXEC_OUT_OF_SCOPE;
}

#ifndef LANEC_PIN_C64
static enum exec_status cm(const agc_native_program *p, apollo_overlay_state *s, void *v, const dispatch *f)
{
    cm_dntm_io *x = v;
    if (!p || !s || !x) return EXEC_OUT_OF_SCOPE;
    entry = *s;
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
        *s = entry;
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
const agc_routine_table lanec_pin_lm_c64 = { lm_c64_entries, LANEC_PIN_LM_C64_ROUTINES };
