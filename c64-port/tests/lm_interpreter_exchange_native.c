/* SPDX-License-Identifier: MIT */
#include "lm_interpreter_exchange_native.h"
#include "p66_native.h"
enum exec_action lm_interpreter_exchange_callback(executive *state,void *user,exec_address *resume){lm_interpreter_exchange_binding *b=user;enum exec_status status;uint8_t token=b->runtime->exclusion->enter(b->runtime->exclusion->user);++b->callbacks;
 if(b->invoke_change)status=(enum exec_status)b->invoke_change(b->change_modules,b->image,b->io,&b->transfer);
#ifndef LM_EXCHANGE_DISPATCH_ONLY
 else status=lm_interpreter_change(b->image,b->io,&b->transfer);
#else
 else status=EXEC_OUT_OF_SCOPE;
#endif
 b->status=(uint16_t)status;
 if(status!=EXEC_OK){b->runtime->fault=status;b->runtime->exclusion->leave(b->runtime->exclusion->user,token);return EXEC_EXCHANGED_BASIC;}
 p66_native_import_executive(state,b->image);*resume=b->transfer.address;
 if(b->transfer.interpretive){b->runtime->interpreter_control.intbit15=b->image->words[0115];b->runtime->interpreter_control.edop=b->image->words[023];b->runtime->exclusion->leave(b->runtime->exclusion->user,token);return EXEC_EXCHANGED_INTERPRETER;}
 b->runtime->exclusion->leave(b->runtime->exclusion->user,token);
 return EXEC_EXCHANGED_BASIC;
}
/* Explicit bounded source entry families from the original CHANG2 corpus.
 * The selected callback is a control-boundary observer, not a translated
 * flight body. It is never executed in the one-exchange source-state proof. */
static enum exec_action selected(executive *state,void *user,exec_address *resume){lm_interpreter_exchange_binding *b=user;(void)state;*resume=b->runtime->current;++b->callbacks;return EXEC_YIELD;}
exec_continuation lm_interpreter_exchange_resolve(void *user,exec_address address,enum exec_context_kind kind){(void)user;
 if(kind==EXEC_CONTEXT_INTERPRETIVE&&address.location==05126&&(address.bank&076000u)==064000u)return lm_interpreter_exchange_callback;
 if((address.bank&(uint16_t)~076167u)!=0)return 0;
 if(kind==EXEC_CONTEXT_BASIC&&address.location>=02001&&address.location<=02007&&(address.bank&076000u)==062000)return selected;
 if(kind==EXEC_CONTEXT_INTERPRETIVE&&address.location>=03362&&address.location<=03370&&(address.bank&076000u)==064000)return selected;
 return 0;
}
