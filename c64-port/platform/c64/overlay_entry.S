/* SPDX-License-Identifier: MIT
 * Native C ABI jump entry; args and return frame remain untouched. */
.section .overlay_entry,"ax",@progbits
.globl apollo_overlay_jump
.type apollo_overlay_jump,@function
apollo_overlay_jump:
    jmp apollo_overlay_main
.size apollo_overlay_jump,.-apollo_overlay_jump
