/* SPDX-License-Identifier: MIT */
/* C64 rope image (include/rope_image.h): one two-byte REU fetch per word. */
#include "rope_image.h"
#include "overlay_reu.h"
static uint8_t cell[2];
bool apollo_rope_read(unsigned mission,uint16_t physical,agc_word *word){
 if(mission>1u||physical>=APOLLO_ROPE_WORDS||!word)return false;
 if(apollo_reu_transfer(APOLLO_REU_FETCH,cell,(mission?APOLLO_ROPE_REU_CM:APOLLO_ROPE_REU_LM)+2u*(uint32_t)physical,2u)&APOLLO_REU_STATUS_FAULT)return false;
 *word=(agc_word)((cell[0]|((unsigned)cell[1]<<8))&077777u);return true;
}
bool apollo_rope_load(unsigned mission,const char *path){(void)mission;(void)path;return false;}
