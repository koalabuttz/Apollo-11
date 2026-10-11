/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_ROUTINES_H
#define STANDARD_LEAD_ROUTINES_H
/* Luminary099 and Comanche055 SYSTEM_TEST_STANDARD_LEAD_INS as native routines of the
 * control-transfer ABI (docs/NATIVE_TRANSFER_ABI.md). LM io is a p66_bias_io.
 *
 * LM basic (bank 04): E/BKCALL 2576, its SWRETURN landing points 2607/2610/2611,
 * E/CALL 2615, the post-EXIT resume 2627 and E/JOBWAK 2631. Each runs its
 * standard_lead stage and leaves Z at SWCALL, INTPRET, INTPRET+2 or the
 * caller, where the trampoline continues natively. The fixed CADR a caller
 * places after its call is that caller's compiled operand, read from the
 * program's site table; the flight program itself has no such caller.
 *
 * lm_interpreter_routines holds the INTERPRETER entries these paths reach so
 * far (INTPRET 6037 for the E/CALL form, INTPRET+2 6041); the interpretive
 * table holds E/CALL's EXIT at LOC 04,2626. The fetch point is 6070. A native
 * interpretive callee ends with lm_interpreter_rvq. */
#include "agc_transfer.h"
#include "standard_lead.h"
#include "standard_lead_wake.h"
#define LM_INTERPRETER_FETCH ((uint16_t)(2048u + (06070u - 04000u)))
extern const agc_routine_table standard_lead_routines;
extern const agc_routine_table lm_interpreter_routines;
extern const agc_routine_table standard_lead_interpretive_routines;
/* The interpreter's RVQ, as the last action of a native interpretive callee. */
enum exec_status lm_interpreter_rvq(apollo_overlay_state *, p66_bias_io *);

/* Comanche055: the same with io a cm_native_io. E/BKCALL 13,2036, landing
 * points 13,2047/2050/2051, E/CALL 13,2055, resume 13,2067, E/JOBWAK 04,2606;
 * INTPRET 6006 and INTPRET+2 6010; EXIT at LOC 13,2066; fetch point 6037. */
#define CM_INTERPRETER_FETCH ((uint16_t)(2048u + (06037u - 04000u)))
extern const agc_routine_table cm_standard_lead_routines;
extern const agc_routine_table cm_interpreter_routines;
extern const agc_routine_table cm_standard_lead_interpretive_routines;
enum exec_status cm_interpreter_rvq(apollo_overlay_state *, cm_native_io *);
#endif
