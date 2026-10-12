/* SPDX-License-Identifier: MIT */
/* Native side of the interpreter body differential: applies the named
 * lm_interp_orders/lm_interp_steps path to each record (16 header words,
 * then the 2048-word image; see tools/lmi_body_reference.c) and reports
 * which stop address the native path ended at. */
#include <stdio.h>
#include "lm_interp_orders.h"
#include "lm_interp_steps.h"
int main(void){
 static apollo_overlay_state s;uint16_t in[16+2048],out[1+2048];
 while(fread(in,sizeof in,1,stdin)==1){
  unsigned i;const agc_word *k=(in[15]&1u)?in+9:0;agc_word a0=in[7],a1=in[8];
  for(i=0;i<2048u;++i)s.words[i]=in[16+i];
  switch(in[0]){
  case 0:lmi_diradres(&s,a0,a1);break;
  case 1:lmi_diradres_pushup(&s,a0,a1);break;
  case 2:lmi_address_pushup(&s,a0,a1);break;
  case 3:lmi_store_code(&s,a0);break;
  case 4:lmi_stadr(&s,a0);break;
  case 5:lmi_dodload(&s);break;
  case 6:lmi_dovload(&s);break;
  case 7:lmi_dload_op(&s,k);break;
  case 8:lmi_vload(&s,k);break;
  case 9:lmi_pdvl(&s,k);break;
  case 10:lmi_pddl_op(&s,k);break;
  case 11:lmi_push(&s);break;
  default:return 2;
  }
  out[0]=4;for(i=0;i<4u;++i)if(in[1+i]&&s.words[5]==in[1+i]){out[0]=(uint16_t)i;break;}
  for(i=0;i<2048u;++i)out[1+i]=s.words[i];
  if(fwrite(out,sizeof out,1,stdout)!=1)return 2;
 }
 return 0;
}
