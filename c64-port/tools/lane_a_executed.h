/* SPDX-License-Identifier: GPL-2.0-or-later; included by the lane A yaAGC
 * oracles after cm_executive_reference.c. Records the fixed-memory words the
 * original program executes (physical bank*1024+offset, per mission) and the
 * interpretive words its interpreter fetches; when LANE_A_EXECUTED names a
 * file, writes them there as "mission physical" lines for the source maps
 * (tools/lane_a_source_map.py). Banked superbank words
 * (02000-03777 under FB 060000+ with SUPERBNK set) are not recorded. */
#include <stdlib.h>
static unsigned char lane_a_ran[2][36u*1024u];
static unsigned lane_a_physical(unsigned pc,unsigned fb){return pc>=06000u?3u*1024u+(pc&01777u):pc>=04000u?2u*1024u+(pc&01777u):(fb>>10)*1024u+(pc&01777u);}
static void lane_a_mark(unsigned mission){static const unsigned fetch[2]={06070u,06037u};unsigned pc,fb,loc,bb;if(machine.PendFlag||machine.ExtraDelay)return;pc=(unsigned)get(RegZ);fb=(unsigned)get(RegFB)&076000u;
 /* At the interpreter's fetch point LOC (0164) under BANKSET (0165) is the
  * interpretive word being executed. */
 if(pc==fetch[mission&1u]){loc=get(0164u);bb=get(0165u)&076000u;
  if(loc>=02000u&&loc<010000u&&!(loc<04000u&&bb>=060000u&&(machine.OutputChannel7&0100)))lane_a_ran[mission&1u][lane_a_physical(loc,bb)]=1;}
 if(pc<02000u||pc>=010000u||(pc<04000u&&fb>=060000u&&(machine.OutputChannel7&0100)))return;
 lane_a_ran[mission&1u][lane_a_physical(pc,fb)]=1;}
static int lane_a_write_executed(void){const char *path=getenv("LANE_A_EXECUTED");FILE *f;unsigned m,i;if(!path)return 0;
 if(!(f=fopen(path,"w")))return 2;for(m=0;m<2u;++m)for(i=0;i<36u*1024u;++i)if(lane_a_ran[m][i])fprintf(f,"%u %u\n",m,i);return fclose(f)?2:0;}
