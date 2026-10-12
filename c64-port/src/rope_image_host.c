/* SPDX-License-Identifier: MIT */
/* Host rope image (include/rope_image.h): the yaYUL binary, reordered to
 * physical banks. */
#include <stdio.h>
#include "rope_image.h"
static agc_word rope[2][APOLLO_ROPE_WORDS];
static bool loaded[2];
bool apollo_rope_load(unsigned mission,const char *path){FILE *f;unsigned i;unsigned char b[2];
 if(mission>1u||!path||(f=fopen(path,"rb"))==0)return false;
 for(i=0;i<APOLLO_ROPE_WORDS;++i){unsigned bank=i/1024u;
  if(fread(b,1,2,f)!=2u){fclose(f);return false;}
  bank=bank<2u?bank+2u:bank<4u?bank-2u:bank;              /* file order 2, 3, 0, 1, 4, ... */
  rope[mission][bank*1024u+i%1024u]=(agc_word)((((unsigned)b[0]<<8)|b[1])>>1);}
 if(fread(b,1,1,f)!=0u){fclose(f);return false;}
 fclose(f);loaded[mission]=true;return true;
}
bool apollo_rope_read(unsigned mission,uint16_t physical,agc_word *word){
 if(mission>1u||!loaded[mission]||physical>=APOLLO_ROPE_WORDS||!word)return false;
 *word=rope[mission][physical];return true;
}
