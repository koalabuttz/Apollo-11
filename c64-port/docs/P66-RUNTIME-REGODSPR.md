# P66 display admission with a pending job

`p66_runtime_regodspr` follows the actual nonflashing V06N60 REGODSPR entry
through NOVAC, SAVELOCR and BANKJUMP. It accepts NEWJOB0, valid pending core
displacements12..84 referring to positive active priorities, or the original
idle marker-0. It compares the admitted display priority against the pending
core and preserves or replaces NEWJOB exactly as the source requires.
Success stops at ENDOFJOB05155; exhaustion reaches BAILOUT1 with alarm01202.
MAKEPLAY is admitted as a real Executive job, not executed by this service.

The full canonical image and explicit SUPER/IRQ are authoritative. The
header states the supported basic bank/register boundary; unsupported inputs
leave both objects unchanged. The caller owns exclusion. Earlier bounded
REGODSPR code and its published evidence are unchanged.

`tools/runtime_regodspr_differential.py` compares 8,192 complete original
Luminary099 2,050-word states, covering pending priority selection, idle
marker, first free core and NOCORE. Snapshot SHA256:
`31494a11128e07a1e7cdb583e1e51baa2fad829a8ffbcf4ef4b1816dc3f37e2f`.
Strict tests cover 64 frozen complete images and 21 invalid inputs; all source
states also agree under ASAN/UBSAN. C64 PAL VICE passes all64 frozen images:
mailbox `a500004000`, PRG12,807 bytes, no extra zero page, measured hardware
stack19 bytes (plus14 ROM bytes) and software stack689 bytes.

Standalone recipes are in `tools/runtime_regodspr.mk`. Native code uses the
native Executive service and explicit source control effects; it contains no
AGC instruction decoder or emulated CPU.
