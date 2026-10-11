# P66 display control and admission

`p66_display_exit` translates the actual EXIT continuation, BANKCALL and
DISPEXIT+3. It starts at PC03531 after the horizontal arithmetic EXIT. The
suppressed branch reaches actual ENDOFJOB PC05155. The enabled vertical branch
forms V06N60 and enters actual REGODSPR PC02470 with the source saved return.
It preserves all unsaved registers, aliases, bank and IRQ effects.

All 8,192 original complete erasable images (2,048 words plus SUPER/IRQ) match.
SHA256 `b77721d0f877bf7d71452a34837d05cc72914aadc78fbc8dc22759d0a251571b`.
Strict tests cover 64 frozen full images and 14 invalid boundaries without
mutation; all source cases pass ASAN/UBSAN. C64 proof passes 64 complete images:
mailbox `a500004000`, PRG4,178 bytes, extra zero-page allocation0, hardware stack
14 bytes plus14 ROM bytes and software stack12 bytes.

`p66_regodspr` performs real nonflashing REGODSPR job admission. It calls the
verified native Executive NOVAC translation, copies the source NV, MPAC+2,
saved user CADR and display flags into the admitted MAKEPLAY core, then
executes the source saved-return and BANKJUMP effects through ENDOFJOB entry.
Success has A2, L02701, Q01476, EB7/FB31, IRQ enabled. The no-core route stops
actual BAILOUT1 entry with source alarm01202; its IRQ and register debris are
preserved. The runtime then invokes the existing verified Executive completion
or bailout service at this actual boundary.

All 8,192 complete original images match, including 1,024 no-core admissions.
SHA256 `d1d8bd7dea68056cc34531153360729db463ee886ee7999014e2b61a8ee62fba`.
Strict64 full images/18 invalid boundaries and all source sanitizer cases pass.
C64 proof64 images: mailbox `a500004000`, PRG11,904 bytes, extra zero-page0,
hardware stack18 bytes plus14 ROM bytes, software stack678 bytes.

The reference executes original instruction bodies. The fixture omits the
unused `PRIOSAVE EQUALS R1SAVE` declaration from DISPLAY_INTERFACE_ROUTINES,
which is separately defined by modern assembly support; the instruction rows
for the exercised bodies and Executive are unchanged. This admission milestone
queues the real MAKEPLAY job; display consumption is a separate source service.
