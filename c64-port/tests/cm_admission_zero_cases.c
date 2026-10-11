/* SPDX-License-Identifier: MIT */
#include "cm_admission_zero_cases.h"
#include "cm_admission_complete_cases.h"
exec_address cm_admission_zero_input(apollo_overlay_state *s,cm_native_io *io,uint32_t n){exec_address job=cm_admission_complete_input(s,io,n%12288u);unsigned core=n/12288u;s->words[0167u+12u*core]=0;return job;}
