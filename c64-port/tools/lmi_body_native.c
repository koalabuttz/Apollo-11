/* SPDX-License-Identifier: MIT */
/* Native side of the interpreter body differential: applies the named
 * lm_interp_orders/lm_interp_steps path to each record (16 header words,
 * then the 2048-word image; see tools/lmi_body_reference.c) and reports
 * which stop address the native path ended at. */
#include <stdio.h>
#include "lm_interp_orders.h"
#include "lm_interp_steps.h"
/* T(X)'s POLY table (TIME_OF_FREE_FALL, bank 27 3736-3752). */
static const agc_word tx[13]={04,012525,012525,071463,057703,04423,017645,074604,043667,01626,037256,077404,052071};
int main(void){
 static apollo_overlay_state s;p66_bias_io io={0,0};uint16_t in[16+2048],out[1+2048];
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
  case 12:lmi_dad(&s,k);break;
  case 13:lmi_dsu(&s,k);break;
  case 14:lmi_bdsu(&s,k);break;
  case 15:lmi_dmp(&s,k);break;
  case 16:lmi_dmpr(&s,k);break;
  case 17:lmi_short_dispatch(&s);if(s.words[5]==02121u)lmi_shortv(&s);else lmi_shortt(&s);break;
  case 18:lmi_gshift(&s);break;
  case 19:lmi_tslc(&s);break;
  case 20:lmi_comp(&s);break;
  case 21:lmi_sign(&s,k);break;
  case 22:lmi_abs(&s);break;
  case 23:lmi_dsq(&s);break;
  case 24:lmi_vsq(&s);break;
  case 25:lmi_dot(&s,k);break;
  case 26:lmi_ddv(&s,k);break;
  case 27:lmi_bddv(&s,k);break;
  case 28:(void)lmi_sqrt(&s);break;
  case 29:(void)lmi_abvalabs(&s);break;
  case 30:(void)lmi_unit(&s);break;
  case 31:lmi_vxsc(&s,k);break;
  case 32:lmi_vxv(&s,k);break;
  case 33:(void)lmi_bov(&s);break;
  case 34:lmi_rtb(&s);break;
  case 35:lmi_swreturn(&s);break;
  case 36:lmi_poly(&s,tx);break;
  case 37:(void)lmi_bplbmn(&s);break;
  case 38:lmi_vad(&s,k);break;
  case 39:lmi_vsu(&s,k);break;
  case 40:lmi_bvsu(&s,k);break;
  case 41:lmi_tad(&s,k);break;
  case 42:lmi_round(&s);break;
  case 43:lmi_ssp(&s,a0);break;
  case 44:lmi_xad(&s);break;
  case 45:lmi_xsu(&s);break;
  case 46:lmi_lxa(&s);break;
  case 47:lmi_sxa(&s);break;
  case 48:(void)lmi_bhiz(&s);break;
  case 49:lmi_call(&s);break;
  case 50:lmi_goto_any(&s);break;
  case 51:lmi_exit(&s);break;
  case 52:lmi_intpret(&s,&io);break;
  case 53:lmi_tpmode(&s);break;
  case 54:lmi_15bitadr(&s,a0,a1);break;
  case 55:(void)lmi_bzegoto(&s);break;
  case 56:lmi_ita(&s);break;
  case 57:(void)lmi_tix(&s);break;
  case 58:lmi_tload(&s);break;
  default:return 2;
  }
  out[0]=4;for(i=0;i<4u;++i)if(in[1+i]&&s.words[5]==in[1+i]){out[0]=(uint16_t)i;break;}
  for(i=0;i<2048u;++i)out[1+i]=s.words[i];
  out[1+2]&=077777u; /* rows hold Q in 15 bits */
  if(fwrite(out,sizeof out,1,stdout)!=1)return 2;
 }
 return 0;
}
