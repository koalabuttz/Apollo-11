/* SPDX-License-Identifier: MIT
 * Fresh zero/INCR edge corpus; previous published inputs remain unchanged. */
#include "dsky_pinball_zero_cases.h"
#include "dsky_pinball_cases.h"
enum dsky_pinball_entry dsky_pinball_zero_entry_for(uint16_t n){return n%4u<2 ? DSKY_2BLANK : (n%4u==3 ? DSKY_DSPIN : DSKY_11DSPIN);}
void dsky_pinball_zero_prepare(apollo_overlay_state *s,uint16_t n){
 static const uint16_t nouts[]={0,077777,1,037777,040000};
 static const uint16_t words[]={0,077777,1,040000,037777,073777,04000};
 enum dsky_pinball_entry e=dsky_pinball_zero_entry_for(n);uint16_t relative,i;
 dsky_pinball_prepare(s,n);s->words[01016]=nouts[(n/2u)%5u];
 if(e==DSKY_2BLANK){s->words[5]=02601;s->words[0777]=(uint16_t)(n%22u);relative=(uint16_t)(s->words[0777]/2u);if((n/154u)%2u==0)for(i=0;i<11;++i)s->words[01023+i]=0;s->words[01023+relative]=words[(n/4u)%7u];}
 else if(e==DSKY_DSPIN){s->words[5]=03322;s->words[0143]=(uint16_t)(n%22u);s->words[0124]=025;relative=(uint16_t)(s->words[0143]/2u);s->words[01023+relative]=04000;}
 else{s->words[5]=03404;s->words[0]=(uint16_t)(n%11u);s->words[0124]=02000;s->words[01023+s->words[0]]=04000;}
}
