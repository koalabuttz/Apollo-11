/* SPDX-License-Identifier: MIT */
#include "p66_descent_control.h"
#include "p66_runtime_fetch_bridge.h"
#include "p66_runtime_bridge.h"
#include "p66_runtime_regodspr.h"
#if !defined(P66_DESCENT_ONLY_CONTROL)||P66_DESCENT_ONLY_CONTROL==91
#include "p66_descent_view.h"
static uint16_t service(apollo_overlay_state *image,p66_bias_io *io,agc_word *channel14){
 descent_frame f;uint16_t base=image->words[0120];unsigned i;
 if(image->words[5]!=02221u||image->words[6]!=062007u||image->words[2]>07777u||
    io->superbank>060u||(io->superbank&017u)||io->interrupt_enabled>1u||
    base<0401u||base>0661u||(base-0401u)%054u||image->words[base-1u]!=0||
    (image->words[0167]&0777u)!=base||image->words[0166]!=base)return 0xffff;
 for(i=0;i<sizeof fields/sizeof fields[0];++i)*word(&f,fields[i].offset)=image->words[fields[i].address];
 f.push.base=base;f.push.pushloc=image->words[0166];for(i=0;i<43u;++i)f.push.words[i]=image->words[base+i];
 /* Q, BB and source hardware I/O have one authoritative owner, shared with
  * control views only for this call. No duplicate aliases are committed. */
 f.service.q=f.control.q;f.service.bbank=f.control.bb;
 f.service.superbank=io->superbank;f.service.interrupt_enabled=io->interrupt_enabled;f.service.channel14=*channel14;
 if(!p66_throttle_service(&f.p,&f.s,&f.e,&f.d,&f.n,&f.feedback,&f.inputs,&f.service))return 0xffff;
 f.control.q=f.service.q;f.control.bb=f.service.bbank;f.control.pc=02367;
 for(i=0;i<sizeof fields/sizeof fields[0];++i)image->words[fields[i].address]=*word(&f,fields[i].offset);
 *channel14=f.service.channel14;io->superbank=f.service.superbank;io->interrupt_enabled=f.service.interrupt_enabled;
 return P66_NATIVE_OK;
}
#endif
uint16_t p66_descent_control_entry(apollo_overlay_state *image,p66_bias_io *io,agc_word *channel14,uint16_t selector){
 if(!image||!io||!channel14)return 0xffff;
 switch(selector){
#if !defined(P66_DESCENT_ONLY_CONTROL)||P66_DESCENT_ONLY_CONTROL==90
 case 90:return (uint16_t)p66_runtime_throttle_fetch(image,io);
#endif
#if !defined(P66_DESCENT_ONLY_CONTROL)||P66_DESCENT_ONLY_CONTROL==91
 case 91:return service(image,io,channel14);
#endif
#if !defined(P66_DESCENT_ONLY_CONTROL)||P66_DESCENT_ONLY_CONTROL==92
 case 92:return (uint16_t)p66_runtime_throttle_return(image,io);
#endif
#if !defined(P66_DESCENT_ONLY_CONTROL)||P66_DESCENT_ONLY_CONTROL==93
 case 93:return (uint16_t)p66_runtime_display_exit(image,io);
#endif
#if !defined(P66_DESCENT_ONLY_CONTROL)||P66_DESCENT_ONLY_CONTROL==94
 case 94:return (uint16_t)p66_runtime_regodspr(image,io);
#endif
 default:return 0xffff;
 }
}
