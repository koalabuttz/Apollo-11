/* SPDX-License-Identifier: MIT */
#include "p51_p53_rtb_routines.h"
#include "cm_native_image.h"
#include "p51_p53_rtb.h"
#include "rtb_angles.h"

/* p51_p53_rtb takes the LM record; the CM's carries the same superbank
 * and interrupt-enable words, latched across the call. */
static enum exec_status run(apollo_overlay_state *s, void *v, enum p51_p53_rtb_kind kind)
{
    cm_native_io *io = v;
    p66_bias_io l;
    uint8_t st;
    if (!s || !io)
        return EXEC_OUT_OF_SCOPE;
    l.superbank = io->superbank;
    l.interrupt_enabled = io->interrupt_enabled;
    st = p51_p53_rtb(s, &l, kind);
    io->superbank = l.superbank;
    io->interrupt_enabled = l.interrupt_enabled;
    return st == RTB_OK ? EXEC_OK : EXEC_OUT_OF_SCOPE;
}
static enum exec_status rdcdus(const agc_native_program *p, apollo_overlay_state *s, void *io)
{
    (void)p;
    return run(s, io, P51_CM_RDCDUS);
}
static enum exec_status set1pdt(const agc_native_program *p, apollo_overlay_state *s, void *io)
{
    (void)p;
    return run(s, io, P51_CM_SET1PDT);
}
static const agc_routine_entry entries[2] = {
    {(uint16_t)(014u * 1024u + (03275u - 02000u)), set1pdt},
    {(uint16_t)(015u * 1024u + (02324u - 02000u)), rdcdus}};
const agc_routine_table cm_p51_p53_rtb_routines = {entries, 2};
