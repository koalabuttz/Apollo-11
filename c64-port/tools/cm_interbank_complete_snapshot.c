/* SPDX-License-Identifier: MIT */
#include "cm_interbank_complete_cases.h"
#include "cm_fixed_compat.h"
#include "cm_interbank_complete_dump.h"
#include <stdlib.h>
#include <string.h>
int main(int argc,char **argv){uint32_t n,start=argc>1?(uint32_t)strtoul(argv[1],0,0):0,end=argc>2?(uint32_t)strtoul(argv[2],0,0):CM_INTERBANK_COMPLETE_CASES;apollo_overlay_state s;cm_native_io io;uint16_t physical;agc_word literal,word;bool ok;
 for(n=start;n<end;++n){unsigned kind=cm_interbank_complete_kind(n);literal=cm_interbank_complete_input(&s,&io,n);word=0;
  if(kind==CM_IC_SUPDACAL&&(!cm_interbank_complete_data_address(&s,&io,&physical)||!cm_fixed_compat_read(physical,&word)))return 2;
  {cm_native_io unowned=io;ok=cm_interbank_complete(&s,&io,(enum cm_interbank_complete_entry)kind,literal,word);unowned.superbank=io.superbank;unowned.interrupt_enabled=io.interrupt_enabled;if(memcmp(&unowned,&io,sizeof io)!=0)return 3;}
  cm_interbank_complete_dump(n,(uint16_t)kind,(uint16_t)!ok,&s,&io);
 }return ferror(stdout)?2:0;}
