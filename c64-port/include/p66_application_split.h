/* SPDX-License-Identifier: MIT */
#ifndef P66_APPLICATION_SPLIT_H
#define P66_APPLICATION_SPLIT_H
#include "p66_application.h"
enum p66_application_split_service {P66_APPLICATION_SPLIT_PREPARE=155,
 P66_APPLICATION_SPLIT_FINISH=159};
/* Additive composition of the audited native source-state bridges. It retains
 * the released binding ABI and all real source entry tuples. Services155/159
 * gather/control-prepare and import only: their C frames must return before
 * setup/bias/vertical, mode/CHARIN, observer or source-END overlays load. */
uint16_t p66_application_split_service(p66_application_binding *,uint16_t);
exec_continuation p66_application_split_resolve(void *,exec_address,enum exec_context_kind);
enum exec_status p66_application_split_init(exec_native *,executive *,p66_application_binding *);
#endif
