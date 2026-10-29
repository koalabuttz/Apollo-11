/* SPDX-License-Identifier: MIT */
#ifndef P66_APPLICATION_H
#define P66_APPLICATION_H
#include "p66_display_job.h"
#include "dsky_display.h"
enum p66_application_service {P66_APPLICATION_SETUP=50,P66_APPLICATION_MODE=51,
 P66_APPLICATION_VERTICAL=52,P66_APPLICATION_KEY_IRQ=53,P66_APPLICATION_NV=54,
 P66_APPLICATION_NUMBER=55,P66_APPLICATION_RELAY=56,P66_APPLICATION_BIAS=83};
typedef struct {
 p66_display_job_binding display_job;
 dsky_display_state peripheral;
 uint16_t channel15,status;
 uint16_t (*invoke_application)(void *,apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
 void *application_modules;
} p66_application_binding;
/* Compiled known entries: direct STARTP66/VRTSTART, actual deferred DSPMMJB,
 * CHARIN VERB/NOUN/digits, and translated descent/display entries. Other key
 * bodies remain rejected by their source guards. No source-opcode resolver. */
exec_continuation p66_application_resolve(void *,exec_address,enum exec_context_kind);
enum exec_status p66_application_init(exec_native *,executive *,p66_application_binding *);
uint16_t p66_application_entry(apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
/* Router for Phase's direct-launch wrapper: service3 is fixed setup/bias/vertical
 * calls with normal C returns, retaining ownership; service4 is vertical only. */
uint16_t p66_application_basic(void *,apollo_overlay_state *,p66_bias_io *,uint16_t);
#endif
