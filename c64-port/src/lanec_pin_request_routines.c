/* SPDX-License-Identifier: MIT */
/* MPAC +3 (erasable 0157) after an operator request, Luminary099 and
 * Comanche055. VBRQEXEC (V30) and VBRQWAIT (V31) end in REQUESTC's
 * INHINT; TC MPAC, which runs the TC NOVAC, TC FINDVAC or TC WAITLIST
 * stored at MPAC (src/dsky_request_control.c lands at the service with
 * Q = MPAC +1). The service returns to MPAC +3, where REQUESTC stored
 * TC ENDOFJOB (CA ENDINST; TS MPAC +3), and that word runs as a TC.
 *
 * This routine runs only that word: TC ENDOFJOB at MPAC +3. Any other word
 * there (the TC MPAC +2 returns of other modules land on the same cell) is
 * refused, so those keep their own routes. */
#include "agc_transfer.h"
#include "agc_image_ops.h"
#include "agc_image_w.h"
#include "lanec_symbols.h"

typedef char same_mpac[(LANEC_LM_MPAC == LANEC_CM_MPAC) ? 1 : -1];
#define MPAC3 (LANEC_LM_MPAC + 3u)
/* TC ENDOFJOB (opcode 0) is ENDOFJOB's fixed-fixed address. */
#define LM_TC_ENDOFJOB LANEC_LM_ENDOFJOB
#define CM_TC_ENDOFJOB LANEC_CM_ENDOFJOB

static enum exec_status mpac3(apollo_overlay_state *s, agc_word tc_endofjob)
{
    if (s == 0 || (W[5] & 07777u) != MPAC3 || ai_word(s, MPAC3) != tc_endofjob)
        return EXEC_OUT_OF_SCOPE;
    ai_tc(s, (agc_word)(MPAC3 + 1u)); /* Q = MPAC +4 */
    W[5] = tc_endofjob;               /* Z = ENDOFJOB */
    return EXEC_OK;
}
static enum exec_status mpac3_lm(const agc_native_program *p, apollo_overlay_state *s, void *io)
{
    (void)p;
    (void)io;
    return mpac3(s, LM_TC_ENDOFJOB);
}
static enum exec_status mpac3_cm(const agc_native_program *p, apollo_overlay_state *s, void *io)
{
    (void)p;
    (void)io;
    return mpac3(s, CM_TC_ENDOFJOB);
}
static const agc_routine_entry lm_entries[1] = {{AGC_TRANSFER_ERASABLE_BASE + MPAC3, mpac3_lm}};
const agc_routine_table lanec_pin_request_lm_routines = {lm_entries, 1u};
static const agc_routine_entry cm_entries[1] = {{AGC_TRANSFER_ERASABLE_BASE + MPAC3, mpac3_cm}};
const agc_routine_table lanec_pin_request_cm_routines = {cm_entries, 1u};
