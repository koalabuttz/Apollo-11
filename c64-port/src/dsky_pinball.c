/* SPDX-License-Identifier: MIT
 * Direct original PINBALL DSPIN/11DSPIN/2BLANK source stores and branches. */
#include "dsky_pinball.h"
#include "agc_word.h"
/* Original INCR NOUT stores sign-extended addition after overflow correction;
 * a plain 15-bit sum gives the wrong sign at NOUT=037777. */
enum p66_bias_status dsky_pinball_write(apollo_overlay_state *s,p66_bias_io *io,enum dsky_pinball_entry entry){
 static const uint16_t masks[4]={037,01740,02000,03740};
 uint16_t *w=s->words,caller=w[2],count=w[0143],code=w[0124],relative,old,mag;agc_register a;
 if(w[3]>03400 || (w[3]&0377u)!=0 || w[4]!=060000 || w[6]!=(060000u|(w[3]>>8)) || io->superbank!=0100 || io->interrupt_enabled>1 || caller<0100 || caller>07777 || (unsigned)entry>(unsigned)DSKY_2BLANK)return P66_BIAS_OUT_OF_SCOPE;
 if(entry==DSKY_DSPIN){if((count&~020000u)>21 || code>037)return P66_BIAS_OUT_OF_SCOPE;relative=(uint16_t)((count&037u)>>1);}
 else if(entry==DSKY_11DSPIN){if(w[0]>10 || (code!=0 && code!=02000))return P66_BIAS_OUT_OF_SCOPE;relative=w[0];}
 else{if(w[0777]>21)return P66_BIAS_OUT_OF_SCOPE;relative=(uint16_t)(w[0777]>>1);}
 old=w[01023+relative];if(entry!=DSKY_2BLANK && old==0)return P66_BIAS_OUT_OF_SCOPE;
 if(entry==DSKY_2BLANK){w[021]=relative;w[01023+relative]=073777;a=agc_extend(old);if(old!=0 && (old&040000u)==0)w[01016]=agc_correct(agc_add_register(agc_extend(w[01016]),1));w[2]=3;}
 else{
  if(entry==DSKY_DSPIN){w[021]=relative;if((count&1u)!=0){code=(uint16_t)(code<<5);count=(count&020000u)!=0 ? 3 : 1;}else count=0;w[0124]=code;}
  else count=2;
  w[0141]=relative;w[0143]=count;w[0114]=caller;
  mag=(old&040000u)!=0 ? agc_negate(old) : old;w[0142]=mag;
  a=agc_add_register(agc_extend((agc_word)(mag&masks[count])),agc_extend(agc_negate(code)));
  if(a!=0 && a!=0177777){uint16_t updated=(uint16_t)((mag&~masks[count])+code);w[01023+relative]=agc_negate(updated);a=agc_extend(old);if((old&040000u)==0)w[01016]=agc_correct(agc_add_register(agc_extend(w[01016]),1));}
  w[2]=0115;
 }
 w[0]=a;w[5]=caller;w[7]=0;io->interrupt_enabled=1;return P66_BIAS_OK;
}
