/* SPDX-License-Identifier: MIT */
#ifndef CM_ENEMA_IMAGE_H
#define CM_ENEMA_IMAGE_H
#include "cm_native_image.h"
/* Canonical original ENEMA through its actual MMDSPLAY/PTBAD/DOFSTART entry.
 * External metadata is explicit; phase records remain source-preserved.
 * Counter/publication exclusion and bounded legal WAITLIST/phase inputs follow
 * cm_enema_begin; subsequent selected application bodies are separate stages. */
enum cm_enema_result cm_enema_image(apollo_overlay_state *,cm_native_io *);
#endif
