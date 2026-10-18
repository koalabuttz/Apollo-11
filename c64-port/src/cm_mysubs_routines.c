/* SPDX-License-Identifier: MIT */
#include "cm_mysubs_routines.h"
#include "cm_mysubs.h"
#define B21(z) (uint16_t)(021u * 1024u + ((z) - 02000u))

/* cm_mysubs_return needs a fixed-code Q; check it first so a refusal leaves
 * the image as it was. */
static enum exec_status run(apollo_overlay_state *s,
                            enum cm_mysubs_status (*body)(apollo_overlay_state *))
{
    if (!s || s->words[2] < 02000u || s->words[2] > 07777u) return EXEC_OUT_OF_SCOPE;
    if (body(s) != CM_MYSUBS_OK) return EXEC_OUT_OF_SCOPE;
    return cm_mysubs_return(s) == CM_MYSUBS_OK ? EXEC_OK : EXEC_OUT_OF_SCOPE;
}
static enum exec_status smallmp(const agc_native_program *p, apollo_overlay_state *s, void *io)
{
    (void)p;
    (void)io;
    return run(s, cm_smallmp);
}
static enum exec_status dpadd(const agc_native_program *p, apollo_overlay_state *s, void *io)
{
    (void)p;
    (void)io;
    return run(s, cm_dpadd);
}
static const agc_routine_entry entries[2] = {{B21(02042u), smallmp}, {B21(02054u), dpadd}};
const agc_routine_table cm_mysubs_routines = {entries, 2};
