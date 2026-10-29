# LM basic job sleep

`lm_basic_sleep` translates JOBSLEEP05133 through the actual selected basic
DTCB dispatch, or idle ADVAN03214 after the original activity-light output.
It stores the sleeping CADR, complements the current priority, saves the
original low bank/SUPER bits and scans the seven other core sets. Its input
is the canonical image and explicit SUPER/IRQ/channel11; the returned next
pair is the selected basic entry or {0,0} at idle. The header defines the
supported active caller and basic selected-context domain. Unsupported
inputs leave every owned object unchanged. The caller owns exclusion.

Source EJSCAN compares CCS(priority)-1. Raw priority1 is a zero candidate
and cannot win an empty scan. The wrapper models that distinction in its
scan and temporarily excludes such candidates from the native typed scan,
then restores their canonical priority words before committing. The shared
Executive kernel is unchanged. Raw caller L, scan BUF and banks are retained
with original effects; selected dispatch retains the caller EB in source L.

`tools/lm_sleep_differential.py` compares 8,192 complete original Luminary099
2,051-word states. Cases cover all initial EB/FB/SUPER values, active/dormant
slots, ties, raw priorities1/2, and both selected and idle outcomes. SHA256:
`91b04ca61fbab8c32c94aaa40b4f54e9dcaff418aa5367b58a6dfa27ad457f0d`.
Strict tests cover 64 frozen complete images and17 invalid boundaries; all
source cases agree under ASAN/UBSAN. C64 PAL VICE passes all64 images:
mailbox `a500004000`, PRG14,334 bytes, no extra zero page, measured hardware
stack18 bytes (plus14 ROM bytes) and software stack692 bytes.

Standalone recipes are in `tools/lm_sleep.mk`. Earlier narrower-domain
exploratory snapshots remain `build/lm-sleep-prior-domain-*`; published
expected fixtures use the final corpus above. Native code contains no AGC
instruction decoder or emulated CPU. Continuing idle SELFCHK is outside this
service boundary and remains a separate basic source body.
