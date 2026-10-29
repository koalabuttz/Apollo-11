/* SPDX-License-Identifier: MIT
 * Source-qualified Comanche decimal body and literal source return stores. */
#include "cm_formatter.h"
#include "dsky_two_digit.h"
#include "dsky_decimal.h"
enum cm_formatter_status cm_two_digit(apollo_overlay_state *s,cm_native_io *io){p66_bias_io primitive;enum p66_bias_status status;
 if(s->words[5]!=03213u)return CM_FORMATTER_OUT_OF_SCOPE;
 primitive.superbank=io->superbank;primitive.interrupt_enabled=io->interrupt_enabled;
 status=dsky_two_digit(s,&primitive);if(status!=P66_BIAS_OK)return CM_FORMATTER_OUT_OF_SCOPE;
 /* Original CM TRACE1S TC DSPIN3157 saves its actual return3160 in
  * DSEXIT114. The decimal arithmetic/stores are separately CM-qualified. */
 s->words[0114]=03160;
 io->superbank=primitive.superbank;io->interrupt_enabled=primitive.interrupt_enabled;return CM_FORMATTER_OK;
}
enum cm_formatter_status cm_decimal_display(apollo_overlay_state *s,cm_native_io *io,uint8_t rounded){p66_bias_io primitive;enum p66_bias_status status;
 if(rounded>1u||s->words[5]!=(rounded!=0u?03133u:03167u))return CM_FORMATTER_OUT_OF_SCOPE;
 primitive.superbank=io->superbank;primitive.interrupt_enabled=io->interrupt_enabled;
 status=dsky_decimal_display(s,&primitive,rounded);if(status!=P66_BIAS_OK)return CM_FORMATTER_OUT_OF_SCOPE;
 /* Actual DSPSIGN caller3136/3172 in TEM1, and TRACE1S caller3160 in
  * DSEXIT. These are literal historical address stores, not flight inputs. */
 s->words[0144]=(uint16_t)(rounded!=0u?03136u:03172u);s->words[0114]=03160;
 io->superbank=primitive.superbank;io->interrupt_enabled=primitive.interrupt_enabled;return CM_FORMATTER_OK;
}
