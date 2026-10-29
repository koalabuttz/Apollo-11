/* SPDX-License-Identifier: MIT */
/* main-assembly on the host: both MAIN.agc manifests against the ropes
 * (argv: Luminary099 then Comanche055 yaYUL binary). */
#include <stdio.h>
#include "main_manifest.h"
#include "rope_image.h"
int main(int argc,char **argv){unsigned m,failures=0;uint16_t checks=0;
 if(argc!=3)return 2;
 for(m=0;m<2u;++m){unsigned f;
  if(!apollo_rope_load(m,argv[1+m])){fprintf(stderr,"cannot read %s\n",argv[1+m]);return 2;}
  f=main_manifest_check(m,&checks);failures+=f;
  printf("%s/MAIN.agc: %u includes, %u fixed words, %u failures\n",m?"Comanche055":"Luminary099",
   (unsigned)main_manifests[m].file_count,(unsigned)main_manifests[m].emitted,f);}
 printf("main-assembly: %u checks, %u failures\n",(unsigned)checks,failures);
 return failures!=0;
}
