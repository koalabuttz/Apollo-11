/* SPDX-License-Identifier: MIT */
/* Certificate coverage trace for the host-only yaAGC oracles: built with
 * -DCERT_TRACE=path, the oracle records every fixed word it starts an
 * instruction at (basic code) and every interpretive word it is at NEWOPS
 * with LOC on (interpretive order pairs), and writes them, one physical
 * fixed address (bank * 1024 + offset) per line, to path at exit. Without
 * CERT_TRACE nothing changes. */
#ifndef CERT_TRACE_H
#define CERT_TRACE_H
#ifdef CERT_TRACE
#include <stdio.h>
#include <stdlib.h>
#define CERT_STR2(x) #x
#define CERT_STR(x) CERT_STR2(x)
static unsigned char cert_seen[44u*1024u];
static unsigned cert_physical(unsigned z,unsigned fb,unsigned ch7){unsigned bank;
 if(z>=04000u)return (z>>10)*1024u+(z&01777u);
 bank=(fb>>10)&037u;if(bank>=030u&&(ch7&0100u))bank+=010u;
 return bank*1024u+(z-02000u);
}
static void cert_write(void){FILE *f=fopen(CERT_STR(CERT_TRACE),"w");unsigned i;
 if(!f)return;for(i=0;i<sizeof cert_seen;++i)if(cert_seen[i])fprintf(f,"%u\n",i);fclose(f);
}
/* At an instruction boundary: Z, FB, channel 7, LOC and BANKSET. */
static void cert_mark(unsigned z,unsigned fb,unsigned ch7,unsigned loc,unsigned bankset){static int armed;
 if(!armed){armed=1;atexit(cert_write);}
 if(z>=02000u&&z<010000u)cert_seen[cert_physical(z,fb,ch7)]=1;
 if(z==06070u&&loc>=02000u&&loc<010000u)cert_seen[cert_physical(loc,bankset&076000u,ch7)]=1;
}
#define CERT_MARK() cert_mark((unsigned)get(RegZ)&07777u,(unsigned)get(4),(unsigned)machine.OutputChannel7,(unsigned)get(0164)&07777u,(unsigned)get(0165))
#else
#define CERT_MARK() ((void)0)
#endif
#endif
