/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "p66_application_split.h"
static uint8_t enter(void*u){(void)u;return 0;}
static void leave(void*u,uint8_t token){(void)u;(void)token;}
static uint16_t math(void*u,apollo_overlay_state*m,uint16_t op){(void)u;(void)m;(void)op;return 0xffff;}
static uint16_t control(void*u,apollo_overlay_state*m,p66_bias_io*io,agc_word*c,uint16_t op){(void)u;(void)m;(void)io;(void)c;(void)op;return 0xffff;}
int main(void){static const agc_word banks[]={0,064007,064067,062007,062027,062047,062067,060102,060101,020067,010067,062101,060106,060107,077777,0164007};
 static p66_application_binding old,newer;static apollo_overlay_state image;
 executive e1,e2;exec_native r1,r2;unsigned b,kind,loc,mapped=0;
 exec_exclusion guard={enter,leave,0};p66_guidance_modules modules={math,0};
 p66_application_binding *objects[2]={&old,&newer};
 for(b=0;b<2;++b){p66_descent_binding*d=&objects[b]->display_job.descent;
  d->basic.arithmetic.image=&image;d->basic.arithmetic.producers=&guard;
  d->math_modules=&modules;d->invoke_control=control;}
 exec_init(&e1);e2=e1;
 assert(p66_application_init(&r1,&e1,&old)==EXEC_OK);
 assert(p66_application_split_init(&r2,&e2,&newer)==EXEC_OK);
 assert(!memcmp(&e1,&e2,sizeof e1));
 for(b=0;b<sizeof banks/sizeof banks[0];++b)for(kind=0;kind<3;++kind)for(loc=0;loc<=010000u;++loc){
  exec_address address={(agc_word)loc,banks[b]};
  exec_continuation a=p66_application_resolve(&old,address,(enum exec_context_kind)kind);
  exec_continuation c=p66_application_split_resolve(&newer,address,(enum exec_context_kind)kind);
  assert((a!=0)==(c!=0));
  if(a){++mapped;if(a!=c)assert(kind==EXEC_CONTEXT_BASIC&&(loc==02477u||loc==02536u||loc==03534u||loc==02077u));}
 }
 assert(mapped);assert(p66_application_split_init(&r2,&e2,0)==EXEC_OUT_OF_SCOPE);
 {apollo_overlay_state before=image;p66_bias_io io=newer.display_job.descent.basic.control;
  assert(p66_application_split_service(&newer,0)==0xffff);
  assert(!memcmp(&image,&before,sizeof image));
  assert(!memcmp(&io,&newer.display_job.descent.basic.control,sizeof io));}
 printf("PASS:196656 known-bank/context/location resolver comparisons; %u mapped entries and unchanged unsupported service\n",mapped);
 return 0;
}
