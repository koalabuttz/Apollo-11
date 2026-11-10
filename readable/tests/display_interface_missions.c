/* SPDX-License-Identifier: MIT */
/* The readable display interface routines of both missions under the
 * faithful modules' names, lanec_di_<part>_dispatch, for the tests: the
 * Makefile builds each mission's readable objects into one, each
 * lanec_di_<part>_dispatch renamed readable_<m>_di_<part>_dispatch. */
#include "lanec_di_dsp.h"
#include "lanec_di_mark.h"
#include "lanec_di_play.h"
#include "lanec_di_sleep.h"
#include "lanec_di_idle.h"
#include "lanec_di_nvdsp.h"

enum lanec_status readable_lm_di_dsp_dispatch(apollo_overlay_state *, lanec_io *,
                                              enum lanec_mission);
enum lanec_status readable_cm_di_dsp_dispatch(apollo_overlay_state *, lanec_io *,
                                              enum lanec_mission);

enum lanec_status lanec_di_dsp_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_di_dsp_dispatch(s, io, m)
                         : readable_cm_di_dsp_dispatch(s, io, m);
}

enum lanec_status readable_lm_di_mark_dispatch(apollo_overlay_state *, lanec_io *,
                                               enum lanec_mission);
enum lanec_status readable_cm_di_mark_dispatch(apollo_overlay_state *, lanec_io *,
                                               enum lanec_mission);

enum lanec_status lanec_di_mark_dispatch(apollo_overlay_state *s, lanec_io *io,
                                         enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_di_mark_dispatch(s, io, m)
                         : readable_cm_di_mark_dispatch(s, io, m);
}

enum lanec_status readable_lm_di_play_dispatch(apollo_overlay_state *, lanec_io *,
                                               enum lanec_mission);
enum lanec_status readable_cm_di_play_dispatch(apollo_overlay_state *, lanec_io *,
                                               enum lanec_mission);

enum lanec_status lanec_di_play_dispatch(apollo_overlay_state *s, lanec_io *io,
                                         enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_di_play_dispatch(s, io, m)
                         : readable_cm_di_play_dispatch(s, io, m);
}

enum lanec_status readable_lm_di_sleep_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);
enum lanec_status readable_cm_di_sleep_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);

enum lanec_status lanec_di_sleep_dispatch(apollo_overlay_state *s, lanec_io *io,
                                          enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_di_sleep_dispatch(s, io, m)
                         : readable_cm_di_sleep_dispatch(s, io, m);
}

enum lanec_status readable_lm_di_idle_dispatch(apollo_overlay_state *, lanec_io *,
                                               enum lanec_mission);
enum lanec_status readable_cm_di_idle_dispatch(apollo_overlay_state *, lanec_io *,
                                               enum lanec_mission);

enum lanec_status lanec_di_idle_dispatch(apollo_overlay_state *s, lanec_io *io,
                                         enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_di_idle_dispatch(s, io, m)
                         : readable_cm_di_idle_dispatch(s, io, m);
}

enum lanec_status readable_lm_di_nvdsp_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);
enum lanec_status readable_cm_di_nvdsp_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);

enum lanec_status lanec_di_nvdsp_dispatch(apollo_overlay_state *s, lanec_io *io,
                                          enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_di_nvdsp_dispatch(s, io, m)
                         : readable_cm_di_nvdsp_dispatch(s, io, m);
}
