/* SPDX-License-Identifier: MIT; input-only baseline constructor. */
#include "lm_interbank_cases.h"
#include "lm_interbank_dump.h"
#include <stdlib.h>
int main(int argc,char **argv){int i;apollo_overlay_state s;p66_bias_io io;
 for(i=1;i<argc;++i){uint32_t n=(uint32_t)strtoul(argv[i],0,0);agc_word literal=lm_interbank_input(&s,&io,n);lm_interbank_dump(n,(uint16_t)lm_interbank_kind(n),literal,&s,&io);}
 return ferror(stdout)?2:0;
}
