/* SPDX-License-Identifier: MIT */
#include "lm_interbank_cases.h"
#include "lm_fixed_compat.h"
#include "lm_interbank_dump.h"
#include <stdlib.h>
int main(int argc,char **argv){uint32_t n,start=argc>1?(uint32_t)strtoul(argv[1],0,0):0,end=argc>2?(uint32_t)strtoul(argv[2],0,0):LM_INTERBANK_CASES;apollo_overlay_state s;p66_bias_io io;uint16_t physical;agc_word literal,word;bool ok;
 for(n=start;n<end;++n){unsigned kind=lm_interbank_kind(n);literal=lm_interbank_input(&s,&io,n);word=0;
  if(kind==LM_IB_SUPDACAL&&(!lm_interbank_data_address(&s,&io,&physical)||!lm_fixed_compat_read(physical,&word)))return 2;
  ok=lm_interbank(&s,&io,(enum lm_interbank_entry)kind,literal,word);lm_interbank_dump(n,(uint16_t)kind,(uint16_t)!ok,&s,&io);
 }return ferror(stdout)?2:0;}
