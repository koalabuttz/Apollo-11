# LM mixed priority and sleep dispatch

`lm_context_lifecycle` translates literal basic PRIOCHNG (`05146`) and JOBSLEEP
(`05133`) through EJSCAN and the actual selected BASIC DTCB, interpreter
INTRSM/NEWOPS (`06070`), or idle ADVAN (`03214`) boundary. It returns an explicit
`exec_transfer`, preserving the interpreter marker and actual next address.
It performs no callback, opcode interpretation, admission or second NEWJOB
poll. The caller owns producer exclusion and adopts the source-selected result.

The additive group keeps the published basic-only helpers unchanged. It
compiles the source scan and eight accumulator exchanges directly against the
sole canonical image, rather than converting an incoming interpreter address
to a fictitious basic job. Negative stored LOC becomes its actual next
interpretive address only in the original ENDPRCHG/INTRSM epilogue. All scratch,
MODE/PUSHLOC, OVFIND, FIXLOC, outgoing slot words and CPU debris are retained.

Two source details matter for general callers. JOBSLEEP's `LOW7 MASK BBANK`
saves EB and SUPER without EXEC FB; the saved sleeping CADR carries the fixed
bank. EJSCAN evaluates `CCS(priority)-1`, so raw priority 1 is a zero candidate
and never wins. PRIOCHNG accepts ordinary low priority bits as written by the
source AD; an actual resulting +0 priority with no winning candidate takes
DUMMYJOB and idle, rather than returning to the old caller.

The qualified domain has an initially positive active core, nonzero other
core priorities, ordinary source registers/banks and legal stored next
addresses. New packed priority remains nonnegative. Positive candidates may
be basic or interpretive. Dormant and free −0 priorities are preserved.
Unsupported input, pointer or selected-pair guards leave every supplied image,
I/O object and result unchanged. Corrupt +0 candidate slots that invoke
CCSHOLE, negative/overflowing changed priorities and wider idle/self-check
branches require separate source qualification; this is not yet a whole-file
Executive certificate.

The independent original assembly run compares 32,768 complete 2048-word
images, SUPER, source IRQ, channel 11 and the explicit selected transfer.
The decoded stream SHA256 is
`0d3a11a0395727342b859ba1c73bdf9fcfc2d0b954038cec3ca299d14b170443`.
Full ASAN/UBSAN matches the same stream. Another 16,384 comparisons reproduce
the earlier basic-only helpers byte-for-byte; mutation/null guard checks cover
the unsupported domain.

The target diagnostic receives frozen original inputs from 32 independently
linked fixture images. Each C fixture frame returns to the resident before
the next image is loaded; the native lifecycle body then checks every raw
word via the original complete-state fingerprint and verifies the explicit
next transfer separately. The canonical image, $8000 module slot and C64 ROM
loader contract remain unchanged. Target evidence records actual stock-ROM
D81 results and measured stack extents.

`lm-context-lifecycle-port-test` and `lm-context-lifecycle-port-sanitize` stream
the retained original XZ evidence using private host build directories and
check all retained fixture PRG headers, SHA, sizes, BSS and CRCs. They do not
rebuild the original assembly engine or change target disks.
