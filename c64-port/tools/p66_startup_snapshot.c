/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_startup_cases.h"
static void emit(void *u,agc_word w) { (void)u; printf(" %u",w); }
int main(void) {
 uint32_t n; static p66_startup_case c;
 for(n=0;n<P66_STARTUP_CASE_COUNT;++n) { p66_startup_prepare(&c,n); printf("%lu %u",(unsigned long)n,p66_startup_apply(&c)); p66_startup_words(&c,emit,0); putchar('\n'); }
 return 0;
}
