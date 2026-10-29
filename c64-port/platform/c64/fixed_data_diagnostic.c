/* SPDX-License-Identifier: MIT */
#include "fixed_data.h"

int main(void)
{
    volatile uint16_t *output = (volatile uint16_t *)0x7000;
    volatile unsigned char *mailbox = (volatile unsigned char *)0xc000;
    uint16_t address, count = 0, word = 0;
    unsigned char mission;
    mailbox[0] = 0xff;
    for (mission = 0; mission != 2; ++mission) {
        for (address = 0; address != 36864u; ++address) {
            if (apollo_fixed_data_read((enum apollo_fixed_mission)mission,
                                       address, &word)) {
                if (count >= 4096u) {
                    mailbox[0] = 0xee;
                    for (;;) {}
                }
                output[count++] = word;
            }
        }
    }
    mailbox[1] = (unsigned char)count;
    mailbox[2] = (unsigned char)(count >> 8);
    mailbox[0] = 0xa5;
    for (;;) {}
}
