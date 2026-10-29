/* SPDX-License-Identifier: MIT */
#include <stdlib.h>
#include "cm_interbank_extended_cases.h"
#include "cm_fixed_compat.h"
#include "../../tools/cm_fixed_compat_dump.h"
static apollo_overlay_state s;static cm_native_io io;
int main(void){uint32_t n;for(n=0;n<CM_INTERBANK_EXTENDED_CASES;++n){agc_word word=0,literal=cm_interbank_extended_input(&s,&io,n);unsigned kind=(unsigned)(n/65536u);enum exec_status status;if(kind==CM_IB_SUPDACAL){uint16_t address;if(cm_interbank_extended_address(&s,&io,&address)!=EXEC_OK||!cm_fixed_compat_read(address,&word))return 2;}status=cm_interbank_extended(&s,&io,(enum cm_interbank_entry)kind,literal,word);if(status!=EXEC_OK&&!(kind==CM_IB_BANKJUMP&&(s.words[0]&0140000u)==0100000u&&status==EXEC_OUT_OF_SCOPE))return 2;cm_fixed_dump(n,(uint16_t)kind,(unsigned)status,&s,&io);}return 0;}
