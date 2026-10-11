/* SPDX-License-Identifier: MIT */
#include "throttle_feedback_phased_cases.h"
unsigned tf_phase(throttle_feedback_case *c,uint8_t phase){switch(phase){
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==0
case 0:return (unsigned)p66_feedback_vector_phase(&c->previous.base.previous.previous.previous.preparation,&c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,&c->previous.base.scratch,&c->previous.norm,&c->previous.base.previous.previous.push,&c->feedback,&c->input,c->bias);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==1
case 1:return (unsigned)p66_feedback_length_phase(&c->previous.base.previous.previous.previous.preparation,&c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,&c->previous.base.scratch,&c->previous.norm,&c->previous.base.previous.previous.push,&c->feedback,&c->input,c->bias);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==2
case 2:return (unsigned)p66_feedback_elapsed_phase(&c->previous.base.previous.previous.previous.preparation,&c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,&c->previous.base.scratch,&c->previous.norm,&c->previous.base.previous.previous.push,&c->feedback,&c->input,c->bias);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==3
case 3:return (unsigned)p66_feedback_correction_phase(&c->previous.base.previous.previous.previous.preparation,&c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,&c->previous.base.scratch,&c->previous.norm,&c->previous.base.previous.previous.push,&c->feedback,&c->input,c->bias);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==4
case 4:return (unsigned)p66_feedback_limits_phase(&c->previous.base.previous.previous.previous.preparation,&c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,&c->previous.base.scratch,&c->previous.norm,&c->previous.base.previous.previous.push,&c->feedback,&c->input,c->bias);
#endif
#if !defined(FEEDBACK_PHASE_SELECTED) || FEEDBACK_PHASE_SELECTED==5
case 5:return (unsigned)p66_feedback_clamp_phase(&c->previous.base.previous.previous.previous.preparation,&c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,&c->previous.base.scratch,&c->previous.norm,&c->previous.base.previous.previous.push,&c->feedback,&c->input,c->bias);
#endif
default:return P66_FEEDBACK_OUT_OF_SCOPE;}}
