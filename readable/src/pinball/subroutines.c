/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc: the small subroutines in
 * fixed-fixed memory -- LM p. 461, CM p. 379. The faithful C is the port's
 * Pinball modules (c64-port/src/lanec_pw_*.c). So far SLEFT5, which DSPIN
 * uses, and RELDSPON, which CHARIN uses. */
#include "pinball.h"
#include "../agc/names.h"

/* SLEFT5 -- LM p. 461, CM p. 379. A five bits up by doubling it five
 * times (DOUBLE), which, unlike LEFT5's CYL, carries nothing round. */
void sleft5(void)
{
    double_A();
    double_A();
    double_A();
    double_A();
    double_A();
    tc_return(Q_ADDRESS);
}

/* RELDSPON -- LM p. 461, CM p. 379. "TURN ON KEY RELEASE LIGHT": "BIT 5 OF
 * CHANNEL 11". */
void reldspon(void)
{
    set_A(BIT5);
    wor_dsalmout();
    tc_return(Q_ADDRESS);
}
