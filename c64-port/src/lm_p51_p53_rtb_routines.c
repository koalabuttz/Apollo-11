/* SPDX-License-Identifier: MIT */
#include "p51_p53_rtb_routines.h"
#include "p51_p53_rtb.h"
#include "rtb_angles.h"

static enum exec_status rdcdus(const agc_native_program *p, apollo_overlay_state *s, void *io)
{
    (void)p;
    return p51_p53_rtb(s, (p66_bias_io *)io, P51_LM_RDCDUS) == RTB_OK ? EXEC_OK : EXEC_OUT_OF_SCOPE;
}
static const agc_routine_entry entries[1] = {
    {(uint16_t)(014u * 1024u + (03413u - 02000u)), rdcdus}};
const agc_routine_table lm_p51_p53_rtb_routines = {entries, 1};
