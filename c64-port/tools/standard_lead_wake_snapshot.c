/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "standard_lead_wake_cases.h"
#include "standard_lead_dump.h"
int main(void){uint16_t n;apollo_overlay_state s;enum exec_status status;
 for(n=0;n<STANDARD_LEAD_WAKE_CASES;++n){standard_lead_wake_input(&s,STANDARD_LEAD_MISSION,n);
  if(STANDARD_LEAD_MISSION==STANDARD_LEAD_LM){p66_bias_io io={0160,0};status=standard_lead_wake_lm(&s,&io);if(io.superbank!=0160||io.interrupt_enabled!=0)return 1;}
  else{cm_native_io io;memset(&io,0,sizeof io);io.superbank=0160;status=standard_lead_wake_cm(&s,&io);if(io.superbank!=0160||io.interrupt_enabled!=0)return 1;}
  if(status!=EXEC_OK&&status!=EXEC_NOT_FOUND){fprintf(stderr,"wake native reject case%u status%u PC%05o LOCCTR%06o FB%06o\n",n,(unsigned)status,s.words[5],s.words[064],s.words[4]);return 1;}
  standard_lead_dump(n,0,&s);
 }return 0;}
