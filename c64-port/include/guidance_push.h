/* SPDX-License-Identifier: MIT */
#ifndef GUIDANCE_PUSH_H
#define GUIDANCE_PUSH_H
#include "guidance_prepare.h"
#define P66_VAC_WORDS 43u
/* Own one historical VAC payload, excluding its preceding VACUSE word.
 * base represents FIXLOC and is one of 0401,0455,0531,0605,0661; pushloc is its raw AGC pointer.
 * This object does not alias or automatically update an Executive VAC. */
typedef struct { agc_word base, pushloc, words[P66_VAC_WORDS]; } p66_pushdown;
enum p66_push_status { P66_PUSH_OK=0, P66_PUSH_OUT_OF_SCOPE=1 };
/* Vector PDDL THISTPIP only: MODE=-1, six free words at pushloc. Preserves
 * exchange debris in MPAC/A/L and advances pushloc by six. Caller owns both
 * objects and excludes producers. Stops at DANZIG before DSU PIPTIME.
 * Unsupported mode/base/pointer returns OUT_OF_SCOPE without mutation;
 * this is a native scope check, not a historical alarm or overflow route. */
enum p66_push_status p66_push_timestamp(p66_preparation *state,
                                       p66_pushdown *push);
#endif
