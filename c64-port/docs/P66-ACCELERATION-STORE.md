# P66 acceleration STORE20 and DAD boundaries

Original STORE20D returns to DANZIG with LOC03421 and EDOP0 before the next
DAD. `p66_acceleration_store_phase` and `p66_acceleration_add_phase` expose
these two source boundaries separately. The earlier combined sum API and
its published evidence remain unchanged.

STORE retains PUSHLOC at VACbase+2, writes VAC20/21, sets ADDRWD=base+20,
A=0 and L=the original VAC21. Source control is PC06061, Q3, FB064000,
EB03400, BB/BANKSET064007, LOC03421, EDOP0, CYR074000; IRQ is retained.
DAD pops the saved acceleration and ends LOC03422 with Q3/CYR020034.
Both APIs reject unsupported MODE, VAC ownership or push pointer unchanged.

`tools/acceleration_store_differential.py` independently compares all five
acceleration boundaries on original assembly with pinned hardware DV:
81,920 complete112-word checkpoints, SHA256
`d0b4c030a35450fb4ea75dbc906600ad163b041b004060473d4430fab002788d`.
The host-only hardware-DV engine variant corrects the reference engine's fast
signed-zero path; no engine, decoder or floating point enters native code.
Strict tests cover64 literal original entry/exit states and65,534 rejected
MODE values. ASAN/UBSAN agrees on the entire source corpus. Two C64 PAL images
pass32 complete states each, mailboxes `a500002000` and `a500002001`,
PRG19,002/20,653 bytes, no extra zero page, hardware stack14 bytes plus14 ROM,
software stack368/371 bytes. Recipes: `tools/acceleration_store.mk`.
