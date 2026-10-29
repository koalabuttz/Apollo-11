/* SPDX-License-Identifier: MIT */
#include "p66_guidance_invoke.h"
uint16_t p66_guidance_invoke(void *user,apollo_overlay_state *image,uint16_t selector){
 p66_guidance_modules *modules=user;uint16_t result;uint8_t i;
 static const uint8_t unit[3]={106,116,126},projection[3]={108,118,128};
 const uint8_t *phases=selector==6?unit:projection;
 if(modules==0||modules->call==0)return P66_NATIVE_UNSUPPORTED;
 if(selector!=6&&selector!=8)return modules->call(modules->user,image,selector);
 for(i=0;i<3;++i){
  result=modules->call(modules->user,image,phases[i]);
  if(result==P66_GUIDANCE_UNIT_TERMINAL&&selector==6)return P66_NATIVE_OK;
  if(result!=P66_NATIVE_OK)return result;
 }
 return P66_NATIVE_OK;
}
