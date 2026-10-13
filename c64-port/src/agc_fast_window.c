/* SPDX-License-Identifier: MIT */
/* The one-byte EBANK 6 window forms of the fast helpers; see
 * agc_fast_window.h. */
#include "agc_fast_window.h"
#define CELL(o) (&AFO_S->words[AFW_EBN*0400u+(o)])
/* Never inlined (LTO would put the address back at every site). */
#define NI __attribute__((noinline))
#define AFW(n) NI void afw_##n(uint8_t o){afo_##n(CELL(o));}
AFW(ca) AFW(cs) AFW(ad) AFW(su) AFW(mask) AFW(xch) AFW(lxch) AFW(qxch) AFW(ads) AFW(incr) AFW(aug) AFW(dim)
AFW(das) AFW(dca) AFW(dcs) AFW(dxch) AFW(mp) AFW(dv)
NI uint8_t afw_ts(uint8_t o){return afo_ts(CELL(o));}
NI uint8_t afw_ccs(uint8_t o){return afo_ccs(CELL(o));}
