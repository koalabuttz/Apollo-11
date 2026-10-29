/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_native_check.h"
static apollo_overlay_state m;static executive s;static exec_native r;static velocity_case input;
static uint8_t excluded;
static uint8_t enter(void*u){uint8_t old=excluded;(void)u;excluded=1;return old;}
static void leave(void*u,uint8_t token){(void)u;excluded=token;}
int main(void){static const exec_exclusion guard={enter,leave,0};unsigned errors=pn_check(&m,&s,&r,&input,&guard,0,0);
 if(excluded)++errors;
 printf("P66 native driver: %u failures, 160 original-source fingerprints\n",errors);return errors!=0;
}
