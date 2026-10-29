/* SPDX-License-Identifier: MIT */
#include "p66_nv_render.h"
#include "agc_word.h"
enum p66_bias_status p66_nv_scale_select(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words,k=w[0117],eb=(uint16_t)(w[3]>>8);static const uint16_t displacement[3]={03025,04322,04331};
 if(k>2u||w[5]!=02537u||w[3]>03400u||(w[3]&0377u)!=0||w[4]!=062000u||w[6]!=(uint16_t)(062000u|eb)||w[7]!=0||io->superbank!=0100u||io->interrupt_enabled>1u||w[0140]!=2u||w[0147]!=060512u||w[0153]!=010347u)return P66_BIAS_OUT_OF_SCOPE;
 w[0154]=(uint16_t)(w[01003u+k]^AGC_MASK);w[0155]=w[0156]=0;w[0777]=(uint16_t)(14u-5u*k);w[0114]=02561;
 w[0123]=k==2u?01046:01031;w[0124]=k==2u?015700:021032;
 if(k==2u)w[022]=077773;else if(k==1u)w[020]=077770;
 w[0]=02600;w[1]=displacement[k];w[2]=k==2u?00710:00717;w[4]=060000;w[6]=(uint16_t)(060000u|eb);w[5]=k==2u?02710:02717;return P66_BIAS_OK;
}
/* Actual DSPDCEND BANKCALL and SWRETURN around the verified decimal body. */
#include "dsky_decimal.h"
enum p66_bias_status p66_nv_decimal_row(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words,eb=(uint16_t)(w[3]>>8),count=w[0777];uint8_t i;
 if(w[5]!=02566u||w[3]>03400u||(w[3]&0377u)!=0||w[4]!=062000u||w[6]!=(uint16_t)(062000u|eb)||w[7]!=0||io->superbank!=0100u||io->interrupt_enabled>1u||(count!=14u&&count!=9u&&count!=4u)||((w[0154]^w[0155])&040000u)!=0)return P66_BIAS_OUT_OF_SCOPE;
 for(i=0;i<11u;++i)if(w[01023u+i]==0)return P66_BIAS_OUT_OF_SCOPE;
 w[0133]=02570;w[0134]=062000;w[2]=04631;w[4]=060000;w[6]=(uint16_t)(060000u|eb);w[5]=03226;
 (void)dsky_decimal_display(s,io,1);
 w[0134]=060000;w[4]=062000;w[6]=(uint16_t)(062000u|eb);w[2]=0134;w[5]=02570;return P66_BIAS_OK;
}
enum p66_bias_status p66_nv_next_row(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words,eb=(uint16_t)(w[3]>>8),saved=w[01040],k=w[0117];
 if(k>2u||w[5]!=02570u||w[3]>03400u||(w[3]&0377u)!=0||w[4]!=062000u||w[6]!=(uint16_t)(062000u|eb)||w[7]!=0||io->superbank!=0100u||io->interrupt_enabled>1u||(k==0u&&(w[01037]!=03122u||(saved&077617u)!=020007u||w[0136]!=04202u)))return P66_BIAS_OUT_OF_SCOPE;
 if(k!=0u){w[0]=(uint16_t)(k-1u);w[0117]=w[0];w[2]=02575;w[5]=02537;}
 else{w[01037]=0;w[01040]=0;w[0]=saved;w[1]=03122;w[2]=2;w[3]=03400;w[4]=020000;w[6]=020007;w[5]=03122;io->superbank=(uint16_t)(saved&0160u);}
 return P66_BIAS_OK;
}
