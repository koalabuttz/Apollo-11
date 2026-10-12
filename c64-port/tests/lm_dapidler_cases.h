/* SPDX-License-Identifier: MIT */
#ifndef LM_DAPIDLER_CASES_H
#define LM_DAPIDLER_CASES_H
#include "lm_dapidler.h"
/* lm-dapidler cases: case n enters entry n % 9 (CHEKBITS, CHEKMORE,
 * DAPIDLER, CHECKUP, 16,2042, 16,2050, 16,2161, SHUTDOWN, ALTDSPLY) from a
 * random image with FB bank 16 (bank 20 for ALTDSPLY) and EB 6, random
 * channels 5, 6, 12, 30, 31 and channel 14 bits 1-5, the CDU counters at
 * any angle, a canonical Executive (all cores busy in an eighth of the
 * cases), and a caller return in Q, and runs through the trampoline (with
 * the native Executive and inter-bank routines) until the
 * original program would leave these files for good: TCF RESUME, the
 * IBNKCALL to QERRCALC or CALCPERR (not yet native), STARTDAP's IBNKCALL to ZATTEROR (DAP_INTERFACE_SUBROUTINES), the
 * 1202 alarm, or CHEKBITS's return to its caller. tools/lm_dapidler_reference.c runs the
 * same entry in the original program. */
#define LM_DI_ENTRIES 9u
#define LM_DI_CASES 9216u
enum {LM_DI_RETURNED=0,LM_DI_ALARM=1,LM_DI_RESUMED=2,LM_DI_QERRCALC=3,LM_DI_CALCPERR=4,LM_DI_ZATTEROR=5,LM_DI_NONE=6};
#define LM_DI_ZATTEROR_Z 02153u /* bank 20 */
#define LM_DI_QERRCALC_Z 02626u /* bank 17 */
#define LM_DI_CALCPERR_Z 03446u /* bank 16 */
typedef struct {agc_word stop_z,stop_fb,superbank;} lm_di_case;
extern const agc_word lm_di_entry_z[LM_DI_ENTRIES];
#define LM_DI_SITES 5u
extern const agc_site_operand lm_di_sites[LM_DI_SITES];
void lm_di_input(apollo_overlay_state *,lm_flight_io *,lm_di_case *,uint16_t n);
/* Runs case n natively; returns the outcome kind (LM_DI_NONE: none). */
unsigned lm_di_run(apollo_overlay_state *,lm_flight_io *,uint16_t n,uint16_t *steps);
uint32_t lm_di_digest(apollo_overlay_state *,uint16_t n);
#endif
