/* SPDX-License-Identifier: MIT */
#ifndef LM_DAP_SHARED_H
#define LM_DAP_SHARED_H
/* Helpers the LM DAP files (DAPIDLER, P-AXIS, Q,R-AXIS) define with the
 * same text. In flight builds (RK_FIXED_IMAGE, one canonical image) each is
 * a weak lm_dap_* definition, so a link of several of the files keeps one
 * copy; elsewhere each file keeps its own static copy, so host tests and
 * C64 diagnostics build exactly as before. Declare a helper with
 * LM_DAP_SHARED after defining its name below. */
#if RK_FIXED_IMAGE
#define LM_DAP_SHARED __attribute__((weak,noinline))
#define msu lm_dap_msu
#define rand lm_dap_rand
#define wand lm_dap_wand
#define sv lm_dap_sv
#define tcq lm_dap_tcq
#else
#define LM_DAP_SHARED static
#endif
#endif
