/* SPDX-License-Identifier: MIT */
/* cm-interp-bodies: the Comanche055 build of the compiled interpretive bodies
 * (src/cm_interp_*.c) on the frozen vector cases, each against the digest of
 * the original Comanche055 program run from the same image in yaAGC
 * (tools/lmi_body_differential.py --mission cm --vectors). */
#include <stdio.h>
#include "cm_interp_vectors.h"
#include "interp_vector_check.h"
int main(void){static apollo_overlay_state s;unsigned n,failures=0,bodies=0,last=0xffffu;uint16_t edit=0;
 for(n=0;n<INTERP_VECTOR_CASES;++n){
  if(interp_vector_header[n][0]!=last){last=interp_vector_header[n][0];++bodies;}
  if(!interp_vector_case(&s,(uint16_t)n,&edit)){++failures;if(failures<20u)fprintf(stderr,"FAIL case %u (body %u)\n",n,interp_vector_header[n][0]);}
 }
 printf("cm-interp-bodies: %u vector cases over %u bodies equal the original Comanche055 program; %u failures\n",INTERP_VECTOR_CASES,bodies,failures);
 return failures!=0u;
}
