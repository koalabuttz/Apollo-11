/* SPDX-License-Identifier: MIT */
#ifndef INTERP_MISSION_H
#define INTERP_MISSION_H
/* The compiled interpretive bodies (src/lm_interp_orders.c, lm_interp_steps.c)
 * serve both programs. Luminary099 and Comanche055 share INTERPRETER.agc's
 * text and erasable assignments, but its fixed code sits at other addresses.
 * IF_(lm,cm) names one fixed address (a TC's return, a Z) in Luminary099 and
 * the same instruction in Comanche055; src/cm_interp_*.c compile the bodies
 * with INTERP_COMANCHE. tools/cm_interp_generate.py --check verifies every
 * pair against both assembly listings. */
#ifdef INTERP_COMANCHE
#define IF_(lm,cm) (cm)
#else
#define IF_(lm,cm) (lm)
#endif
#endif
