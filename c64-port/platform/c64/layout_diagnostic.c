/* SPDX-License-Identifier: MIT */
#include "layout_check.h"

int main(void)
{
    volatile unsigned char *mailbox = (volatile unsigned char *)0xc000;
    apollo_overlay_state *image = (apollo_overlay_state *)0x7000;
    mailbox[0] = 0xff;
    if (!layout_check(image, (volatile uint16_t *)0x8000)) {
        mailbox[0] = 0xee;
        for (;;) {}
    }
    mailbox[1] = (unsigned char)LAYOUT_CHECK_BINDINGS;
    mailbox[2] = (unsigned char)(LAYOUT_CHECK_BINDINGS >> 8);
    mailbox[0] = 0xa5;
    for (;;) {}
}
