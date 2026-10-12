/* SPDX-License-Identifier: MIT */
/* Per-case coverage trace for the host-only yaAGC oracles: built with
 * -DCASE_TRACE=path, CASE_MARK() records the fixed word the oracle starts
 * an instruction at (physical: bank * 1024 + offset, fixed-fixed as its own
 * bank), and CASE_END(n) appends one line "n a1 a2 ..." to path with the
 * words case n executed. The freeze tools use it to pick the C64 rows by
 * outcome (tools/c64_rows.py). Without CASE_TRACE nothing changes. */
#ifndef CASE_TRACE_H
#define CASE_TRACE_H
#ifdef CASE_TRACE
#include <stdio.h>
#include <string.h>
#define CASE_STR2(x) #x
#define CASE_STR(x) CASE_STR2(x)
static unsigned char case_seen[44u*1024u];
static FILE *case_file;
static void case_mark(unsigned z,unsigned fb,unsigned ch7){unsigned bank;
 if(z<02000u||z>=010000u)return;
 if(z>=04000u){case_seen[(z>>10)*1024u+(z&01777u)]=1;return;}
 bank=(fb>>10)&037u;if(bank>=030u&&(ch7&0100u))bank+=010u;
 case_seen[bank*1024u+(z-02000u)]=1;
}
static void case_end(unsigned n){unsigned i;
 if(!case_file)case_file=fopen(CASE_STR(CASE_TRACE),"w");
 if(case_file){fprintf(case_file,"%u",n);for(i=0;i<sizeof case_seen;++i)if(case_seen[i])fprintf(case_file," %u",i);fputc('\n',case_file);fflush(case_file);}
 memset(case_seen,0,sizeof case_seen);
}
#define CASE_MARK() case_mark((unsigned)get(RegZ)&07777u,(unsigned)get(4),(unsigned)machine.OutputChannel7)
#define CASE_END(n) case_end(n)
#else
#define CASE_MARK() ((void)0)
#define CASE_END(n) ((void)0)
#endif
#endif
