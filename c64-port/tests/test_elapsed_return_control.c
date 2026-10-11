/* SPDX-License-Identifier: MIT */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "elapsed_return_control_cases.h"
int main(void){apollo_overlay_state image,before;agc_dp x={1,0},p={2,0};unsigned n;
 eq_prepare(&image,0);image.words[5]=06061;
 for(n=0;n<12u;++n){image.words[5]=06061;image.words[0164]=03360;image.words[023]=0;image.words[0163]=0;image.words[4]=064000;image.words[3]=01000;image.words[6]=064002;
  if(n<7u){static const unsigned addr[7]={5,0164,023,0163,4,3,6};image.words[addr[n]]^=1u;}
  before=image;
  if(n<7u)assert(p66_elapsed_return_control(&image,02701,x,p)==2u);
  else if(n==7u)assert(p66_elapsed_return_control(&image,0100000,x,p)==2u);
  else if(n==8u){agc_dp bad={0100000,0};assert(p66_elapsed_return_control(&image,02701,bad,p)==2u);}
  else if(n==9u){agc_dp bad={0,0100000};assert(p66_elapsed_return_control(&image,02701,bad,p)==2u);}
  else if(n==10u){agc_dp bad={0100000,0};assert(p66_elapsed_return_control(&image,02701,x,bad)==2u);}
  else {agc_dp bad={0,0100000};assert(p66_elapsed_return_control(&image,02701,x,bad)==2u);}
  assert(memcmp(&image,&before,sizeof image)==0);
 }
 assert(p66_elapsed_return_control(0,02701,x,p)==2u);
 puts("PASS:12 invalid source/control domains and null image preserve all2048 words");return 0;
}
