/* SPDX-License-Identifier: MIT */
#ifndef P66_THROTTLE_BRIDGE_H
#define P66_THROTTLE_BRIDGE_H
#include "p66_bias.h"
enum p66_bridge_status {P66_BRIDGE_OK=0,P66_BRIDGE_OUT_OF_SCOPE=1};
/* Canonical physical-image bridges, producer exclusion required. No decoded
 * execution: fixed historical EXIT/BANKCALL and THDUMP/SWRETURN/INTPRET paths.
 * Enter: DANZIG06061/LOC03507, MODE0, EDOP0, NEWJOB0, BB/BANKSET064007.
 * Return: THDUMP02367, BB062007, BUF2={03514,064000}, RTNHOLD04631.
 * Stops at THROTTLE+3 PC02221 or INTPRET NEWOPS06070/LOC03515 respectively.
 * Invalid boundary or IO state rejects both objects unchanged. A/L are full
 * register words; other erasable words are ordinary15-bit caller input. */
enum p66_bridge_status p66_throttle_enter(apollo_overlay_state *,p66_bias_io *);
enum p66_bridge_status p66_throttle_return(apollo_overlay_state *,p66_bias_io *);
#endif
