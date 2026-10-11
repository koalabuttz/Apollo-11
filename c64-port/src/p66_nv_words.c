/* SPDX-License-Identifier: MIT */
#include "p66_nv_words.h"
#include "dsky_two_digit.h"
static uint8_t ready(const apollo_overlay_state *s,const p66_bias_io *io,uint16_t pc){const uint16_t *w=s->words;return(uint8_t)(w[5]==pc&&w[3]==00400u&&w[4]==062000u&&w[6]==062001u&&w[7]==0&&io->superbank==0100u&&io->interrupt_enabled<=1u);}
static void return_update(uint16_t *w,uint16_t caller){w[4]=062000;w[6]=(uint16_t)(062000u|(w[3]>>8));w[2]=0120;w[5]=caller;}
enum p66_bias_status p66_nv_verb(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words;if(!ready(s,io,02337)||w[0]!=6u||w[2]!=03602u||w[01034]==0)return P66_BIAS_OUT_OF_SCOPE;
 w[01001]=6;w[0117]=03602;w[0777]=023;w[0]=6;w[2]=03320;w[4]=060000;w[6]=060001;w[5]=03306;
 (void)dsky_two_digit(s,io);return_update(w,03602);return P66_BIAS_OK;}
enum p66_bias_status p66_nv_noun(apollo_overlay_state *s,p66_bias_io *io){uint16_t *w=s->words;if(!ready(s,io,03602)||w[0157]!=60u||w[01033]==0)return P66_BIAS_OUT_OF_SCOPE;
 w[01002]=60;w[0117]=03604;w[0146]=024074;w[0147]=060512;w[0140]=2;w[0153]=010347;w[0150]=02262;w[0151]=03473;w[0152]=03534;
 w[01017]=024074;w[0145]=01474;w[3]=0;w[6]=060000;w[4]=060000;w[0777]=021;w[0]=60;w[2]=03320;w[5]=03306;
 (void)dsky_two_digit(s,io);return_update(w,03604);return P66_BIAS_OK;}
