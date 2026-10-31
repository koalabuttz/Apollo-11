/* SPDX-License-Identifier: MIT */
/* The readable extended verbs of both missions under the faithful
 * modules' names, lanec_ev_<part>_dispatch, for the tests: the Makefile
 * builds each mission's readable objects into one, each
 * lanec_ev_<part>_dispatch renamed readable_<m>_ev_<part>_dispatch. */
#include "lanec_ev_align.h"
#include "lanec_ev_dap.h"
#include "lanec_ev_front.h"
#include "lanec_ev_radar.h"
#include "lanec_ev_short.h"
#include "lanec_ev_time.h"

enum lanec_status readable_lm_ev_front_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);
enum lanec_status readable_cm_ev_front_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);

enum lanec_status lanec_ev_front_dispatch(apollo_overlay_state *s, lanec_io *io,
                                          enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_ev_front_dispatch(s, io, m)
                         : readable_cm_ev_front_dispatch(s, io, m);
}

enum lanec_status readable_lm_ev_short_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);
enum lanec_status readable_cm_ev_short_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);

enum lanec_status lanec_ev_short_dispatch(apollo_overlay_state *s, lanec_io *io,
                                          enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_ev_short_dispatch(s, io, m)
                         : readable_cm_ev_short_dispatch(s, io, m);
}

enum lanec_status readable_lm_ev_align_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);
enum lanec_status readable_cm_ev_align_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);

enum lanec_status lanec_ev_align_dispatch(apollo_overlay_state *s, lanec_io *io,
                                          enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_ev_align_dispatch(s, io, m)
                         : readable_cm_ev_align_dispatch(s, io, m);
}

enum lanec_status readable_lm_ev_radar_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);
enum lanec_status readable_cm_ev_radar_dispatch(apollo_overlay_state *, lanec_io *,
                                                enum lanec_mission);

enum lanec_status lanec_ev_radar_dispatch(apollo_overlay_state *s, lanec_io *io,
                                          enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_ev_radar_dispatch(s, io, m)
                         : readable_cm_ev_radar_dispatch(s, io, m);
}

enum lanec_status readable_lm_ev_dap_dispatch(apollo_overlay_state *, lanec_io *,
                                              enum lanec_mission);
enum lanec_status readable_cm_ev_dap_dispatch(apollo_overlay_state *, lanec_io *,
                                              enum lanec_mission);

enum lanec_status lanec_ev_dap_dispatch(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_ev_dap_dispatch(s, io, m)
                         : readable_cm_ev_dap_dispatch(s, io, m);
}

enum lanec_status readable_lm_ev_time_dispatch(apollo_overlay_state *, lanec_io *,
                                               enum lanec_mission);
enum lanec_status readable_cm_ev_time_dispatch(apollo_overlay_state *, lanec_io *,
                                               enum lanec_mission);

enum lanec_status lanec_ev_time_dispatch(apollo_overlay_state *s, lanec_io *io,
                                         enum lanec_mission m)
{
    return m == LANEC_LM ? readable_lm_ev_time_dispatch(s, io, m)
                         : readable_cm_ev_time_dispatch(s, io, m);
}
