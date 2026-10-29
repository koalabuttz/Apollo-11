/* SPDX-License-Identifier: MIT */
#include "cm_single_return.h"

enum cm_trig_status cm_single_return(apollo_overlay_state *m)
{
    agc_word returned;
    if ((m->words[5] != 05130u && m->words[5] != 05133u) ||
        m->words[2] < 02000u || m->words[2] > 07777u || m->words[7] != 0)
        return CM_TRIG_OUT_OF_SCOPE;
    returned = m->words[2];
    m->words[1] = agc_extend(agc_correct(m->words[1]));
    m->words[5] = returned;
    m->words[2] = 3;
    return CM_TRIG_OK;
}
