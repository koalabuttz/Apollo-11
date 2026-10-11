/* SPDX-License-Identifier: MIT */
#include "lm_clock.h"
int lm_counter_pinc(uint16_t *word){uint16_t n;if(!word||*word>077777u)return -1;n=*word;if(n==037777u){*word=0;return 1;}n=(uint16_t)((n+1u)&077777u);if(n==0)n=1;*word=n;return 0;}
int lm_clock_advance(apollo_overlay_state *image,uint16_t centiseconds,uint16_t *pending_t3){uint16_t i,*w;if(!image||!pending_t3||*pending_t3>1u)return -1;w=image->words;if(w[024]>037777u||w[025]>037777u||w[026]>037777u)return -1;for(i=0;i<centiseconds;++i){if(w[024]==037777u){w[024]=0;w[025]=w[025]==037777u?0:(uint16_t)(w[025]+1u);}else ++w[024];if(w[026]==037777u){w[026]=0;*pending_t3=1;}else ++w[026];}return 0;}
