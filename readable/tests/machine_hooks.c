/* SPDX-License-Identifier: MIT */
/* For the side-by-side tests of the bindings that point the machine at
 * hooks of their own (channels, a way to stop): whether a run that came
 * back left the machine pointed at them. A run that goes through the
 * interpreter (interpret_listing) leaves the machine at the interpreter's
 * hooks, which have no channels, and the binding points it back before
 * the next step or before it comes back; a binding that came back with
 * the interpreter's would leave the next call, and the next step of its
 * own, to stop through a frame that has returned. */
#include <stddef.h>
#include "../src/agc/agc.h"

int machine_hooks_bound(void)
{
    return agc.channel_in != NULL && agc.channel_out != NULL && agc.leave != NULL;
}
