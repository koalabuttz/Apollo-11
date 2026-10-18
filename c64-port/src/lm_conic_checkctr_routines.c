/* SPDX-License-Identifier: MIT */
#include "lm_conic_checkctr_routines.h"
#include "lm_conic_checkctr.h"
#include "rtb_angles.h"

static enum exec_status checkctr(const agc_native_program *p, apollo_overlay_state *s, void *io)
{
    (void)p;
    (void)io;
    return lm_conic_checkctr(s) == RTB_OK ? EXEC_OK : EXEC_OUT_OF_SCOPE;
}
static const agc_routine_entry entries[1] = {
    {(uint16_t)(012u * 1024u + (02644u - 02000u)), checkctr}};
const agc_routine_table lm_conic_checkctr_routines = {entries, 1};
