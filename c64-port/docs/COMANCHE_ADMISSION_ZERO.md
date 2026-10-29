# Comanche admission with an unavailable +0 core

`cm_admission_zero` is an additive canonical NOVAC/FINDVAC/SPVAC/SPVACIN
qualification. It retains the released admission body's source operations,
register/scratch/resource effects and real return/BAILOUT boundaries, while
accepting a +0 priority in any of the seven core slots. Released admission
helpers and historical proofs are unchanged. It requires an ordinary compiled
caller Q0100..07775 and the existing legal VAC sentinel, NEWJOB, positive
priority, coherent bank and producer-exclusion contracts.

The original scan executes `CCS PRIORITY`. Its +0 successor is the numeric
`NO.CORES DEC6` word followed by `TCF NEXTCORE`. In that concrete source control
path word00006 is EXTEND, so the following instruction executes as BZF; the
CCS result is A=+0 and control reaches NEXTCORE. This is a defined unavailable
core scan. Native C follows the compiled scan directly and never decodes the
word. The historical bounded inventory's description of this path as a TC6
fault was incorrect; this additive source proof corrects that interpretation
without rewriting old published evidence. Actual SPECTEST NEWJOB zero fault
branches and their CCSHOLE recovery remain distinct domains.

The full fresh byte-original-layout6e3150ae... oracle runs86016 cases: every
one of seven +0 slot positions across all12288 original admission input
cohorts, including first-free-core and VAC outcomes, exhaustion, four entry
forms, priority/bank/background contexts and placement before/after the
available core. All2048 physical words plus SUPER and IRQ match at every actual
return or BAILOUT entry. Stream SHA256 is
`e59ca10fabf23d895a3ea4803d696f30e44ae4d39347199fc2a708337eaedb48`.
Strict host and full ASAN/UBSAN runs retain that exact stream.

Stock PAL C64 executes56 representative contexts, all seven +0 locations and
four admission forms. It checks complete-state fingerprints at every source
boundary, two complete2050-field image/I/O samples, and the final2048-word
image externally. Mailbox `a500003800` is PASS; measured software stack extent
is719 bytes. Native221 loads5536 bytes, diagnostic input2544842 bytes and
checker25510019 bytes, with no added allocated zero-page storage. Slot swaps
occur only after native C frames return; incoming hardware I and ROM mapping
are checked. Alarm and selected application bodies remain later actual groups.

This is a broader admission branch qualification, not a complete Executive
certificate. It does not certify arbitrary priority/VAC corruption, reserved
CPU-word continuation descriptors, all valid inline caller aliases, or every
selected mission body. Source locations and fixed words retain original
provenance; no emulator or AGC decoder is linked into native code.

Run `make -f Makefile -f tools/cm_admission_zero.mk test-cm-admission-zero`
and `sanitize-cm-admission-zero`; target tools are
`cm_freeze_admission_zero.py`, `cm_admission_zero_build.py` and
`cm_admission_zero_vice_test.py`. MIT native C and GPL host-only oracle remain
separate.
