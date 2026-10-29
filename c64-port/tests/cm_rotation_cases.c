/* SPDX-License-Identifier: MIT */
#include "cm_rotation_cases.h"
unsigned cm_rot_apply(rotation_case *c) {
 return cm_axis_rotate(&c->previous.base.previous.previous.previous.preparation,
 &c->previous.base.previous.previous.previous.scratch,&c->previous.base.previous.elapsed,
 &c->previous.base.scratch,&c->previous.norm,&c->trig,&c->rotation,c->direction,c->return_word);
}
