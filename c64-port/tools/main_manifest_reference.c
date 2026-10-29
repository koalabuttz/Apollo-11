/* SPDX-License-Identifier: GPL-2.0-or-later
 * Host-only oracle for main-assembly, linked to Virtual AGC. NEVER part of the
 * native port. argv: Luminary099 rope, Comanche055 rope. yaAGC loads each rope
 * with its own loader (agc_load_binfile: bank order and parity) and then runs
 * from every unused word that holds TC to itself, which must stay there; each
 * included file of the native manifest (src/main_manifest.c) is compared with
 * the words yaAGC holds at its runs. Prints one line per mission and the
 * failures. */
#include <stdio.h>
#include <string.h>
#include "agc_engine.h"
#include "main_manifest.h"
void BacktraceAdd(agc_t *s,int cause){(void)s;(void)cause;}
int ChannelInput(agc_t *s){(void)s;return 0;}
void ChannelOutput(agc_t *s,int c,int v){if(c==7)s->InputChannel[7]=s->OutputChannel7=v&0160;}
void ChannelRoutine(agc_t *s){(void)s;}
void ShiftToDeda(agc_t *s,int d){(void)s;(void)d;}
void RequestRadarData(agc_t *s){(void)s;}
void UnblockSocket(int s){(void)s;}
static agc_t machine;
static unsigned char emitted[36u*1024u];
int main(int argc,char **argv){unsigned m,failures=0;
 if(argc!=3)return 2;
 for(m=0;m<2u;++m){const main_manifest *man=&main_manifests[m];unsigned f,r,i,files=0,loops=0,bad=0;
  memset(&machine,0,sizeof machine);if(agc_load_binfile(&machine,argv[1+m]))return 2;
  memset(emitted,0,sizeof emitted);
  for(f=0;f<man->file_count;++f){const main_manifest_file *e=&man->files[f];unsigned long sum=0;unsigned words=0;
   for(r=e->first_run;r<e->first_run+e->runs;++r)for(i=0;i<man->runs[r].count;++i){unsigned p=man->runs[r].start+i;
    sum+=(unsigned long)(machine.Fixed[p/1024u][p%1024u]&077777);++words;emitted[p]=1;}
   ++files;if(words!=e->words||(sum&0xfffffffful)!=e->sum){++bad;printf("FAIL: %s %s\n",m?"Comanche055":"Luminary099",e->file);}}
  /* yaAGC from each unused TC-to-itself word in a bank (FB) it can select: still there 4 instructions later. */
  for(i=0;i<36u*1024u;++i){unsigned bank=i/1024u,self=(bank==2u||bank==3u)?i:02000u+i%1024u,n;agc_t run;
   if(emitted[i]||(unsigned)(machine.Fixed[bank][i%1024u]&077777)!=self||bank>=32u)continue;
   run=machine;run.Erasable[0][RegZ]=(int16_t)self;run.Erasable[0][RegFB]=(int16_t)(bank<<10);
   run.Erasable[0][RegBB]=(int16_t)(bank<<10);run.AllowInterrupt=0;
   for(n=0;n<4u;++n){run.ScalerCounter=0;if(agc_engine(&run))return 2;}
   ++loops;if((unsigned)(run.Erasable[0][RegZ]&07777)!=self){++bad;printf("FAIL: %s fill %05o\n",m?"Comanche055":"Luminary099",i);}}
  printf("%s: %u included files equal yaAGC's loaded rope, %u unused TC-to-itself words loop in yaAGC, %u differing\n",
   m?"Comanche055":"Luminary099",files,loops,bad);
  failures+=bad;}
 printf("%u failures\n",failures);
 return failures!=0;}
