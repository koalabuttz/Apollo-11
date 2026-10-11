/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "p66_status_check.h"
static apollo_overlay_state image;static executive state;static exec_native runtime;
static uint8_t excluded;
static uint8_t enter(void *u){uint8_t old=excluded;(void)u;excluded=1;return old;}
static void leave(void *u,uint8_t token){(void)u;excluded=token;}
int main(void){static const exec_exclusion guard={enter,leave,0};
 unsigned failures=p66_status_check(&image,&state,&runtime,&guard);
 if(excluded)++failures;
 printf("P66 full-width status: %u failures; five raw results, committed alarm payload and fault retention\n",failures);
 return failures!=0;
}
