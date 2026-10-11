/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "cm_restart_select_cases.h"
static apollo_overlay_state s;
int main(void){unsigned n,i;for(n=0;n<CM_RSSELECT_CASES;++n){enum cm_restart_select_exit status;cm_rsselect_input(&s,(uint16_t)n);status=cm_restart_select(&s,n<4096u?CM_RS_SELECT_GROUP:n<5120u?CM_RS_SELECT_PART2:CM_RS_SELECT_TYPEB_TABLE);printf("%u %u",n,(unsigned)status);for(i=0;i<2048u;++i)printf(" %u",s.words[i]);putchar('\n');}return 0;}
