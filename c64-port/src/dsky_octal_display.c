/* SPDX-License-Identifier: MIT */
#include "dsky_octal_display.h"
static uint16_t rotate_left(uint16_t value,unsigned n){return(uint16_t)((((unsigned)value<<n)&077777u)|((unsigned)value>>(15u-n)));}
static uint16_t rotate_right(uint16_t value,unsigned n){return(uint16_t)(((unsigned)value>>n)|(((unsigned)value<<(15u-n))&077777u));}
static enum p66_bias_status octal(apollo_overlay_state *s,p66_bias_io *io,enum dsky_octal_route *route,uint8_t cm){static const uint16_t relay[]={025,003,031,033,017,036,034,023};uint16_t *w=s->words,caller=w[2],count=w[0777],input=agc_correct(w[0]),code,value,relative,dspin_caller=cm ? 03320u : 03413u;uint16_t i;uint8_t two=(uint8_t)(w[5]==03416u),n,len=two ? 2u : 5u;p66_bias_io physical;enum p66_bias_status status;
 if(route==0||(!two&&w[5]!=03363u)||w[0]!=agc_extend(input)||caller<0100u||caller>07777u||count>21u||count<(two ? 1u : 4u)||w[3]>03400u||(w[3]&0377u)||w[4]!=062000u||w[6]!=(uint16_t)(062000u|(w[3]>>8))||w[7]!=0||io->superbank>0160u||(io->superbank&017u)||(io->superbank&0100u)==0||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
 for(i=3;i<2048u;++i)if(w[i]>077777u)return P66_BIAS_OUT_OF_SCOPE;
 w[1]=agc_extend(agc_correct(w[1]));w[0115]=caller;
 if(two){w[020]=agc_negate(rotate_right(input,3));w[022]=rotate_right(input,2);}else{w[022]=rotate_left(input,1);w[0777]=(uint16_t)(020000u+count);}
 for(n=0;n<len;++n){w[0137]=(uint16_t)(len-1u-n);if(two&&n==0)value=rotate_right(input,3);else{value=rotate_left(w[022],2);w[022]=rotate_left(w[022],3);}code=relay[value&7u];w[0124]=code;count=w[0777];w[0777]=code;w[0143]=count;if(count!=0)w[0777]=agc_diminished_abs(count);relative=(uint16_t)((count&037u)>>1);
  w[4]=060000;w[6]=(uint16_t)(060000u|(w[3]>>8));w[2]=dspin_caller;w[5]=cm ? 03227u : 03322u;
  if(w[01023u+relative]==0){w[021]=relative;w[0141]=relative;w[0114]=dspin_caller;if(count&1u){w[0124]=(uint16_t)(code<<5);w[0143]=(count&020000u)!=0 ? 3u : 1u;}else w[0143]=0;w[0]=0;w[2]=cm ? 03261u : 03354u;w[5]=cm ? 05747u : 05705u;io->interrupt_enabled=0;*route=DSKY_OCTAL_CCS_HOLE;return P66_BIAS_OK;}
  /* Physical bank40 is the same source image for all four retained SUPER
   * forms. Use the unchanged qualified store arithmetic in a transient
   * physical-bank view; authoritative SUPER never changes. The complete
   * original proof qualifies this expanded composition independently. */
  physical.superbank=0100;physical.interrupt_enabled=io->interrupt_enabled;status=dsky_pinball_write(s,&physical,DSKY_DSPIN);if(status!=P66_BIAS_OK)return P66_BIAS_OUT_OF_SCOPE;io->interrupt_enabled=physical.interrupt_enabled;w[4]=062000;w[6]=(uint16_t)(062000u|(w[3]>>8));
 }
 w[0777]=077754;w[0]=0177754;w[2]=0116;w[5]=caller;*route=DSKY_OCTAL_CALLER;return P66_BIAS_OK;
}
enum p66_bias_status dsky_octal_display(apollo_overlay_state *s,p66_bias_io *io,enum dsky_octal_route *route){return octal(s,io,route,0);}
enum exec_status cm_octal_display(apollo_overlay_state *s,cm_native_io *io,enum dsky_octal_route *route){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=octal(s,&p,route,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
