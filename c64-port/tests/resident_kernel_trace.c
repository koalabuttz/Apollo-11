/* SPDX-License-Identifier: MIT */
/* resident-kernel row tracer: runs one case of tests/resident_kernel_cases.c
 * by its flat number (services, then jobs, then idle) for
 * tools/resident_kernel_rows.py, which reads the kernel's branch arcs with
 * gcov. A case the certified routines refused (no yaAGC oracle) is not run.
 * Build with -DRK_LM=1 or -DRK_CM=1. */
#include <stdlib.h>
#include "resident_kernel_cases.h"
#if RK_LM
#include "lm_exec_services_vectors.h"
#include "lm_exec_jobs_vectors.h"
#include "lm_exec_idle_vectors.h"
#define E_S lm_exs_expected
#define E_J lm_exj_expected
#define E_I lm_exi_expected
#else
#include "cm_exec_services_vectors.h"
#include "cm_exec_jobs_vectors.h"
#include "cm_exec_idle_vectors.h"
#define E_S cm_exs_expected
#define E_J cm_exj_expected
#define E_I cm_exi_expected
#endif
int main(int argc,char **argv){apollo_overlay_state s;unsigned long n;uint32_t d,e;
 if(argc!=2)return 2;
 n=strtoul(argv[1],0,10);
 if(n<RK_SERVICES_CASES){e=E_S[n];if(e==RK_REFUSED_DIGEST)return 0;d=rk_services_digest(&s,(uint16_t)n);}
 else if((n-=RK_SERVICES_CASES)<RK_JOBS_CASES){e=E_J[n];if(e==RK_REFUSED_DIGEST)return 0;d=rk_jobs_digest(&s,(uint16_t)n);}
 else if((n-=RK_JOBS_CASES)<RK_IDLE_CASES){e=E_I[n];if(e==RK_REFUSED_DIGEST)return 0;d=rk_idle_digest(&s,(uint16_t)n);}
 else return 2;
 return d!=e;
}
