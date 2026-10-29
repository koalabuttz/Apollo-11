/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_native_cases.h"
int main(void){unsigned n,i;uint16_t words[PN_WORDS];velocity_case c;
 for(n=0;n<8;++n){vl_prepare(&c,pn_number((uint16_t)n));vl_next(&c,pn_number((uint16_t)n),0);vl_words(&c,words);
  printf("%u",n);for(i=0;i<PN_WORDS;++i)printf(" %u",words[i]);puts("");
 }return 0;
}
