/* SPDX-License-Identifier: MIT */
/* Native side of the interpreter body differential: applies the named
 * lm_interp_orders/lm_interp_steps path to each record (16 header words,
 * then the 2048-word image; see tools/lmi_body_reference.c) and reports
 * which stop address the native path ended at. */
#include <stdio.h>
#include "interp_body_run.h"
int main(void){
 static apollo_overlay_state s;uint16_t in[16+2048],out[1+2048];
 while(fread(in,sizeof in,1,stdin)==1){
  unsigned i;
  for(i=0;i<2048u;++i)s.words[i]=in[16+i];
  out[0]=(uint16_t)interp_body_run(&s,in);if(out[0]==5u)return 2;
  for(i=0;i<2048u;++i)out[1+i]=s.words[i];
  out[1+2]&=077777u; /* rows hold Q in 15 bits */
  if(fwrite(out,sizeof out,1,stdout)!=1)return 2;
 }
 return 0;
}
