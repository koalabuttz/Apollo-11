/* SPDX-License-Identifier: MIT */
#include "cm_powered_quick_return.h"
enum cm_powered_quick_return_status cm_powered_quick_return(apollo_overlay_state *m){if(m->words[5]!=03605u||m->words[4]!=046000u||m->words[3]>03400u||(m->words[3]&0377u)!=0||m->words[6]!=(046000u|(m->words[3]>>8))||m->words[7]!=0||m->words[2]!=3u||m->words[0]<02000u||m->words[0]>07777u)return CM_POWERED_QUICK_RETURN_OUT_OF_SCOPE;m->words[5]=m->words[0];m->words[2]=1;return CM_POWERED_QUICK_RETURN_OK;}
