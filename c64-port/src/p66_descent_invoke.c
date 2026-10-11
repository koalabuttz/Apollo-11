/* SPDX-License-Identifier: MIT */
#include "p66_descent_invoke.h"
uint16_t p66_descent_invoke(void *user,apollo_overlay_state *image,uint16_t selector){
 p66_guidance_modules *modules=user;uint16_t result;uint8_t i,count;
 static const uint8_t feedback[4]={210,211,212,213};
 static const uint8_t unit[4]={230,231,232,233};
 static const uint8_t length[3]={234,235,236};
 const uint8_t *phases;
 if(modules==0||modules->call==0)return P66_NATIVE_UNSUPPORTED;
 if(selector!=191u&&selector!=223u&&selector!=225u)
  return p66_guidance_invoke(user,image,selector);
 phases=selector==191u?feedback:selector==223u?unit:length;
 count=selector==225u?3u:4u;
 for(i=0;i<count;++i){
  result=modules->call(modules->user,image,phases[i]);
  if(result==P66_GUIDANCE_UNIT_TERMINAL&&selector==223u){
   /* Root overflow has already completed UNIT; its pending VXSC remains.
    * A finish overflow also continues that source right-hand operation. */
   if(i==1u)i=2u;
  }else if(result!=P66_NATIVE_OK)return result;
 }
 return P66_NATIVE_OK;
}
