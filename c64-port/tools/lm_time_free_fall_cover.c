/* SPDX-License-Identifier: MIT */
/* Branch outcomes of every native TIME_OF_FREE_FALL phase step over the
 * sampled call chains (tools/edge_cover.h): one line per step whose edge
 * set is new, tagged "n frontier phase", where frontier counts the rows
 * tools/lm_time_free_fall_phases.c emits for case n. */
#include <stdio.h>
#include <stdlib.h>
#include "lm_time_free_fall.h"
#include "lm_time_free_fall_cases.h"
#include "edge_cover.h"
static uint8_t first(const apollo_overlay_state *m){
 switch(m->words[0164]){case 03360u:return LTF_ENTRY;case 03361u:return LTF_CONMU;case 03427u:return LTF_RPRA;case 03465u:return LTF_CALCTPER;default:return LTF_CALCTFF;}
}
int main(int argc,char **argv){
 uint32_t n,states=argc>1?(uint32_t)strtoul(argv[1],0,0):65536u;char tag[48];
 for(n=0;n<states;++n){apollo_overlay_state m;p66_bias_io io;uint8_t k,calls=lm_time_free_fall_calls(n),phase=LTF_RETURNED,before;unsigned guard,row=0;
  lm_time_free_fall_input(&m,&io,n);edge_cover_mark(stdout,"");
  for(k=0;k<calls&&phase==LTF_RETURNED;++k){
   if(k)lm_time_free_fall_call(&m,n,k);
   edge_cover_mark(stdout,"");
   phase=first(&m);
   for(guard=0;guard<64u&&phase<LTF_RETURNED;++guard){
    before=phase;phase=lm_time_free_fall_step(&m,&io,phase);
    snprintf(tag,sizeof tag,"%lu %u %u",(unsigned long)n,row,(unsigned)before);edge_cover_mark(stdout,tag);++row;}
   ++row;
   if(phase!=LTF_RETURNED&&phase!=LTF_ABORTED)return 3;}}
 fprintf(stderr,"%u edges\n",edge_cover_edges());
 return 0;}
