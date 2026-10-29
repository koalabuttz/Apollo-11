/* SPDX-License-Identifier: MIT; input-only baseline constructor. */
#include "cm_interbank_complete_cases.h"
#include "cm_interbank_complete_dump.h"
#include <stdlib.h>
int main(int argc,char **argv){int i;apollo_overlay_state s;cm_native_io io;
 for(i=1;i<argc;++i){uint32_t n=(uint32_t)strtoul(argv[i],0,0);agc_word literal=cm_interbank_complete_input(&s,&io,n);cm_interbank_complete_dump(n,(uint16_t)cm_interbank_complete_kind(n),literal,&s,&io);}
 return ferror(stdout)?2:0;
}
