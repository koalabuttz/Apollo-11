/* SPDX-License-Identifier: MIT */
/* Native side of the interpreter body differential: applies the named
 * lm_interp_orders/lm_interp_steps path to each record (16 header words,
 * then the 2048-word image; see tools/lmi_body_reference.c) and reports
 * which stop address the native path ended at. */
#include <stdio.h>
#include "interp_body_run.h"
/* The rope (argv[1]), for matrix operands too long for the record header:
 * header flag 2 names the bank (word 9) and address (word 10) instead. */
static uint8_t rope[36u*2048u];
static agc_word rope_word(unsigned bank,unsigned off){
 unsigned i=(bank<4u?(unsigned)"\2\3\0\1"[bank]:bank)*1024u+off;
 return (agc_word)((((unsigned)rope[2u*i]<<8)|rope[2u*i+1u])>>1);
}
int main(int argc,char **argv){
 static apollo_overlay_state s;uint16_t in[16+2048],out[1+2048];agc_word mk[18];
 if(argc>1){FILE *f=fopen(argv[1],"rb");if(!f||fread(rope,1,sizeof rope,f)!=sizeof rope)return 2;fclose(f);}
 while(fread(in,sizeof in,1,stdin)==1){
  unsigned i;
  if(in[15]&2u)for(i=0;i<18u;++i)mk[i]=rope_word(in[9],in[10]-02000u+i);
  for(i=0;i<2048u;++i)s.words[i]=in[16+i];
  out[0]=(uint16_t)interp_body_run(&s,in,(in[15]&2u)?mk:0);if(out[0]==5u)return 2;
  for(i=0;i<2048u;++i)out[1+i]=s.words[i];
  out[1+2]&=077777u; /* rows hold Q in 15 bits */
  if(fwrite(out,sizeof out,1,stdout)!=1)return 2;
 }
 return 0;
}
