# P66 normal MAKEPLAY control

`p66_makeplay_priority` follows the actual MAKEPLAY2546 entry to PRIOCHNG5146,
saving the user's priority before raising the display job to priority33.
`p66_makeplay_select` follows the priority return2553 through normal LINUS
selection and COPIES2. It reaches real JOBWAKE5137, GOSLEEPS2754 or duplicate
ENDO FJOB5155. `p66_makeplay_wake_return` follows JOBWAKE's caller2614 through
XCHTOEND to GOPLAY3070, replacing a found dormant caller's LOC with ENDINST.

The supported domain is the original P66 nonflashing V06N60 context stated
in the header. All erasable words, raw CPU registers, banks, SUPER/IRQ and
channel11 are authoritative. Invalid boundaries leave all owned inputs
unchanged; the caller owns exclusion. Priority changes and wake/sleep
scheduling remain actual native Executive services at the returned entries.
This module does not coalesce scheduling or implement the subsequent sleep
body. Earlier GOPLAY and consumer modules remain unchanged.

`tools/makeplay_differential.py` compares 12,288 complete original Luminary099
2,051-word states, covering all three boundaries, pending display flags,
LINUS duplicates, waking and no-match XCHTOEND returns. Snapshot SHA256:
`64e5ec904a0bb5bda6c58a919ef96effa7914b05065b543b8dd54de5d6be722a`.
Strict tests cover 64 frozen complete images and45 invalid boundaries; all
source cases also agree under ASAN/UBSAN. C64 PAL VICE passes all64 images:
mailbox `a500004000`, PRG4,752 bytes, no extra zero page, measured hardware
stack14 bytes (plus14 ROM bytes) and software stack22 bytes.

Standalone recipes are in `tools/makeplay.mk`. Expected fixtures are frozen
from original assembly. Native code contains no AGC instruction decoder.
