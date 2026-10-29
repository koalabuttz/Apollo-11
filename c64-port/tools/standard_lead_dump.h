/* SPDX-License-Identifier: MIT */
#include <stdio.h>
static void standard_lead_dump(unsigned n,unsigned status,const apollo_overlay_state *s){unsigned i;printf("%u %u",n,status);for(i=0;i<2048u;++i)printf(" %u",(unsigned)s->words[i]);putchar('\n');}
