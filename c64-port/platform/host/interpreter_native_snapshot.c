/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include "interpreter_native_cases.h"
void in_row(unsigned n,unsigned step,const executive *s,enum exec_status status,
            exec_address current,enum exec_context_kind kind,exec_interpreter control) {
    unsigned i,j;
    printf("%u %u %u",n,step,(unsigned)status);
    for(i=0;i<EXEC_CORE_COUNT;++i)
        for(j=0;j<EXEC_CORE_WORDS;++j) printf(" %u",s->core[i][j]);
    for(i=0;i<EXEC_VAC_COUNT;++i)
        for(j=0;j<EXEC_VAC_WORDS;++j) printf(" %u",s->vac[i][j]);
    printf(" %u %u %u %u %u %u %u %u %u %u %u %u %u\n",s->newjob,s->newprio,
        s->newloc[0],s->newloc[1],s->locctr,s->fixloc,s->ovfind,s->alarm,
        current.location,current.bank,(unsigned)kind,control.intbit15,control.edop);
}
#ifndef IN_ROW_ONLY
int main(void) {
    unsigned n,step;
    for(n=0;n<IN_CASE_COUNT;++n) {
        in_run r;
        in_run_init(&r,n);
        for(step=0;step<in_steps(n);++step) {
            enum exec_status status=in_run_step(&r,step);
            in_row(n,step,&r.state,status,r.runtime.current,r.runtime.current_kind,
                   r.runtime.interpreter_control);
            if(r.failures) fprintf(stderr,"case %u step %u payload/frame/guard failures %u\n",n,step,r.failures);
        }
        if(r.failures) return 1;
    }
    return 0;
}
#endif
