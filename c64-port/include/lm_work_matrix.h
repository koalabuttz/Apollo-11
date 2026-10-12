/* SPDX-License-Identifier: MIT */
#ifndef LM_WORK_MATRIX_H
#define LM_WORK_MATRIX_H
/* Luminary099 MXV (column 0) and VXM (column 1) of an 18-word matrix in the
 * work area (the address formation of `word` names erasable below 01000,
 * e.g. LOMAT's matrix at 0D), through the compiled matrix phases
 * (src/matrix_native.c). Address formation, then the four phases, ending at
 * DANZIG as lmi_mxv/lmi_vxm do. matrix_native_phase's domain starts at
 * ADDRWD 01000, so each phase runs on ADDRWD+01000 and the bias comes back
 * out after it (phases 1 and 2 also leave it in A), as lm-midgim's GET.LVC
 * does. Returns 0 (image partly advanced) when the matrix would run past
 * erasable or a phase refuses. */
#include "overlay_runtime.h"
#include "agc_word.h"
uint8_t lm_work_matrix(apollo_overlay_state *,agc_word word,uint8_t column);
#endif
