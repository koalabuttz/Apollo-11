/* SPDX-License-Identifier: MIT; compile-only MOS ABI allocation probe. */
#include <stddef.h>
#include "p66_application_pending.h"
#include "native_module_registry.h"
typedef struct {exec_native runtime;p66_application_pending binding;exec_irq_queue queue;} pending_metadata;
const uint16_t pending_layout_values[]={sizeof(exec_native),sizeof(p66_application_pending),
 sizeof(exec_irq_queue),sizeof(pending_metadata),offsetof(pending_metadata,binding),
 offsetof(pending_metadata,queue),sizeof(executive),sizeof(native_registry_lease)};
