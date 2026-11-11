/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDMARK_SELECTION_FOR_CM.agc, the CM's (Comanche055 p. 936): a
 * page with one line on it, a comment, and no instructions, constants or
 * entry points. It follows the S-band antenna routine
 * (src/sband_antenna/). The line is kept here as the listing has it and
 * compared with the listing by make assembly-cases; no flight code reads
 * it. */
#include "assembly.h"

#if COMANCHE055

const char *const LUNAR_LANDMARK_SELECTION_FOR_CM[] = {
    "*** END OF TROUBLE .043 ***",
};
const unsigned LUNAR_LANDMARK_LINE_COUNT = COUNT(LUNAR_LANDMARK_SELECTION_FOR_CM);

#endif /* COMANCHE055 */
