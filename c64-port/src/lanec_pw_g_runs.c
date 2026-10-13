/* SPDX-License-Identifier: MIT */
/* The lanec_run adapters of the pw-* suites of batch g (include/lanec_pw_g_run.h). */
#include "lanec_pw_g_run.h"
#include "dsky_charin_number.h"
#include "dsky_charin_nv.h"
#include "dsky_decimal.h"
#include "dsky_nv_key.h"
#include "dsky_pinball.h"
#include "dsky_scale.h"
#include "dsky_two_digit.h"

LANEC_PW_G_CHARIN_RUN(lanec_pw_charin_number_run, dsky_charin_number(s, &p))

LANEC_PW_G_CHARIN_RUN(lanec_pw_charin_nv_run, dsky_charin_nv(s, &p))

LANEC_PW_G_P66_RUN(lanec_pw_decimal_run, dsky_decimal_display(s, &p, (uint8_t)(s->words[LANEC_Z] == 03226u)))

LANEC_PW_G_P66_RUN(lanec_pw_nv_key_run, dsky_nv_key(s, &p, s->words[LANEC_Z] == 02370u ? DSKY_KEY_NOUN : DSKY_KEY_VERB))

LANEC_PW_G_P66_RUN(lanec_pw_pinball_run, dsky_pinball_write(s, &p, s->words[LANEC_Z] == 03322u ? DSKY_DSPIN
                   : s->words[LANEC_Z] == 03404u ? DSKY_11DSPIN : DSKY_2BLANK))

LANEC_PW_G_P66_RUN(lanec_pw_scale_run, dsky_scale_output(s, &p, s->words[LANEC_Z] == 02710u ? DSKY_DP1OUT
                   : s->words[LANEC_Z] == 02715u ? DSKY_DP2OUT : DSKY_DP3OUT))

LANEC_PW_G_P66_RUN(lanec_pw_two_digit_run, dsky_two_digit(s, &p))
