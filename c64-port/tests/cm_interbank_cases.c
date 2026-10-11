/* SPDX-License-Identifier: MIT */
#include "cm_interbank_cases.h"
#include "cm_waitlist_image_cases.h"
#include "fixed_data.h"
agc_word cm_interbank_input(apollo_overlay_state *s,cm_native_io *io,uint16_t n){unsigned kind=n/512u,local=n%512u;agc_word literal=(agc_word)(((local>>3)&31u)*02000u+0200u+local%1024u);static const agc_word pc[14]={04662,04666,04675,04701,04704,04711,04715,04740,04744,04753,04757,04773,04707,04710};cm_waitlist_image_input(s,io,n);s->words[5]=pc[kind];s->words[3]=(agc_word)((local&7u)*0400u);s->words[4]=(agc_word)(((local>>3)&31u)*02000u);s->words[6]=(agc_word)(s->words[4]|(local&7u));s->words[0]=agc_extend((agc_word)((local*977u)&AGC_MASK));s->words[2]=0705;s->words[0705]=literal;s->words[0133]=0710;s->words[0134]=(agc_word)(((local>>4)&31u)*02000u);s->words[072]=0710;s->words[073]=s->words[0134];io->superbank=(agc_word)(((local^(local>>6))&7u)*020u);
 if(kind==CM_IB_SWCALL||kind==CM_IB_BANKJUMP||kind==CM_IB_ISWCALL)s->words[0]=agc_extend(literal);
 if(kind==CM_IB_QJUMP)s->words[2]=(agc_word)(local%1024u);
 if(kind==CM_IB_SUPDACAL){apollo_fixed_table table;uint16_t address;apollo_noun_table_info(APOLLO_FIXED_CM,APOLLO_NNTYPTAB,&table);address=(uint16_t)(table.physical_fixed_word+local%table.word_count);s->words[0]=agc_extend((agc_word)(((address/1024u>=32u?address/1024u-8u:address/1024u)*02000u)|(address%1024u)));s->words[1]=0100;}
 return literal;}
