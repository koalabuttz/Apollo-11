/* SPDX-License-Identifier: MIT */
#ifndef INTERP_VECTOR_CHECK_H
#define INTERP_VECTOR_CHECK_H
/* Rebuilds vector case n (an interp vectors header: xorshift16 fill plus
 * its edits, which start at *edit and are consumed), runs its body with
 * interp_body_run and compares the digest of the stop tag and image with the
 * original program's. Include after the vectors header; walk the cases in
 * order from *edit = 0. Returns 1 when equal. */
#include "interp_body_run.h"
#include "agc_word.h"
static uint32_t interp_vector_digest_of(unsigned tag,const apollo_overlay_state *s){
 uint16_t s1=(uint16_t)tag,s2=(uint16_t)tag;unsigned i;
 for(i=0;i<2048u;++i){uint16_t w=s->words[i];if(i==2u)w&=077777u;s1=(uint16_t)(s1+w);s2=(uint16_t)(s2+s1);}
 return (uint32_t)s2<<16|s1;
}
static int interp_vector_case(apollo_overlay_state *s,uint16_t n,uint16_t *edit){
 uint16_t x=interp_vector_seed[n],i;unsigned tag;
 if(!x)x=1;
 for(i=0;i<2048u;++i){x^=(uint16_t)(x<<7);x^=(uint16_t)(x>>9);x^=(uint16_t)(x<<8);s->words[i]=(agc_word)(x&077777u);}
 for(i=0;i<interp_vector_count[n];++i,++*edit)s->words[interp_vector_edit[*edit][0]]=interp_vector_edit[*edit][1];
 tag=interp_body_run(s,interp_vector_header[n]);
 return interp_vector_digest_of(tag,s)==interp_vector_digest[n];
}
#endif
