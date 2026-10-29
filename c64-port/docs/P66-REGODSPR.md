# P66 display job admission

`p66_regodspr` translates original REGODSPR through NOVAC, SAVELOCR and
BANKJUMP to the actual ENDOFJOB entry. It admits an original MAKEPLAY job
with its verb/noun, calling CADR and flags in the selected Executive core;
it does not consume the queued display job. `p66_display_exit` supplies this
entry after the original final P66 interpreted EXIT and DISPEXIT branch.

The authoritative input is the full 2,048-word canonical image and SUPER/IRQ.
The header defines the supported bank, active core and admission boundary.
Source NOCORE stops at the real BAILOUT1 entrance with alarm01202; the native
caller must dispatch the alarm continuation. Unsupported inputs are unchanged.

`tools/regodspr_differential.py` independently runs original Luminary099
assembly: 8,192 complete 2,050-word checkpoints, including 1,024 NOCORE cases,
SHA256 `d1d8bd7dea68056cc34531153360729db463ee886ee7999014e2b61a8ee62fba`.
All erasable words, raw A/L/Q, banks, SUPER and IRQ are compared. The fixture
omits one unused PRIOSAVE alias declaration; source instructions are unchanged.
Strict tests cover 64 frozen complete images and 18 invalid inputs, with all
source cases also passing ASAN/UBSAN. C64 PAL VICE passes all64 frozen images:
mailbox `a500004000`, PRG11,904 bytes, no extra zero page, measured hardware
stack18 bytes (plus14 ROM bytes) and software stack678 bytes.

Standalone recipes are in `tools/regodspr.mk`. Frozen expected values are
literal source-image deltas, not native outputs. Parent runtime integration
owns actual ENDOFJOB completion and subsequent MAKEPLAY dispatch.
