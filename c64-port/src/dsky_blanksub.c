/* SPDX-License-Identifier: MIT */
#include "dsky_blanksub.h"
static enum p66_bias_status blanksub(apollo_overlay_state *s,p66_bias_io *io,uint8_t cm){uint16_t *w=s->words,mask=(uint16_t)(w[0]&7u),busy,q,oldbank,a,l,i;
 static const uint16_t singular[]={14,5,4};
 if(w[5]!=(cm ? 04271u : 04255u)||w[7]!=0||(w[4]&~076000u)||w[3]>03400u||(w[3]&0377u)||w[6]!=(uint16_t)(w[4]|(w[3]>>8))||w[2]<0100u||w[2]>07775u||w[01012]>1u||io->superbank>0160u||(io->superbank&017u)||io->interrupt_enabled>1u)return P66_BIAS_OUT_OF_SCOPE;
 busy=(uint16_t)((w[01021]&020000u)+w[01012]);
 if(!busy&&mask)for(i=0;i<3;++i)if((mask&(1u<<i))&&w[01023+(singular[i]>>1)]==0)return P66_BIAS_OUT_OF_SCOPE;
 q=w[2];w[0123]=mask;w[1]=agc_extend(agc_correct(w[1]));w[0]=agc_diminished_abs(busy);
 if(busy){w[2]=3;w[5]=q;return P66_BIAS_OK;}
 ++q;w[0]=agc_diminished_abs(mask);
 if(mask==0){w[2]=3;w[5]=q;return P66_BIAS_OK;}
 oldbank=(uint16_t)(w[6]|io->superbank);a=w[0130];l=w[0131];w[0130]=oldbank;w[0131]=q;w[0]=agc_extend(a);w[1]=agc_extend(l);w[3]=0400;w[4]=060000;w[6]=060001;io->superbank=0100;w[0]=0100;w[2]=cm ? 04316u : 04302u;w[5]=cm ? 03412u : 03505u;
 if(cm){cm_native_io p={0};p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;(void)cm_blank(s,&p,DSKY_BLNKSUB1);(void)cm_blank(s,&p,DSKY_BLANK_RETURN);io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;}else{(void)dsky_blank(s,io,DSKY_BLNKSUB1);(void)dsky_blank(s,io,DSKY_BLANK_RETURN);}return P66_BIAS_OK;
}
enum p66_bias_status dsky_blanksub(apollo_overlay_state *s,p66_bias_io *io){return blanksub(s,io,0);}
enum exec_status cm_blanksub(apollo_overlay_state *s,cm_native_io *io){p66_bias_io p;enum p66_bias_status status;p.superbank=io->superbank;p.interrupt_enabled=io->interrupt_enabled;status=blanksub(s,&p,1);if(status!=P66_BIAS_OK)return EXEC_OUT_OF_SCOPE;io->superbank=p.superbank;io->interrupt_enabled=p.interrupt_enabled;return EXEC_OK;}
