/* SPDX-License-Identifier: MIT */
#include "lanec_pin_routines.h"
#include "lanec_pw_runs.h"
#include "lanec_pin_gaps.h"
#include "lanec_pin_heads.h"
#include "lanec_pin_keys.h"
#include "lm_downtelemetry.h"
#include "cm_downtelemetry.h"

/* The trampoline (src/lanec_pin_trampoline.c) runs these routines on the lm_dntm_io or cm_dntm_io record. */
#define lm lanec_pin_run_lm
#define cm lanec_pin_run_cm
typedef lanec_pin_dispatch dispatch;

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
