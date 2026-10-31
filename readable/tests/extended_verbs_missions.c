/* SPDX-License-Identifier: MIT */
/* The readable extended verbs' front end of both missions under the
 * faithful module's name, lanec_ev_front_dispatch, for the tests: the
 * Makefile builds each mission's readable objects into one, its
 * lanec_ev_front_dispatch renamed readable_<m>_ev_front_dispatch. */
#include "lanec_ev_front.h"

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
