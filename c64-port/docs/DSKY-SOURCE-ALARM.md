# Native source display alarms and lamps

`dsky_source_alarm` and `cm_source_alarm` translate actual GODSPALM,
PREDSPAL, DSPALARM, CHARALRM, ALM/END and callable FALTON/FALTOF/RELDSPON.
They preserve source-selected ordinary caller, ENDOFJOB, PASTEVB, POODOO or
PINBRNCH continuations and complete canonical CPU/image/channel effects.
GODSPALM executes the original bank transfer; PREDSPAL blocks numerical
characters. DSPALARM distinguishes actual TC NVSUBEND from monitor PASTEVB
and keyboard returns. Monitor failure sets MONSAVE1 to the source kill bit,
lights operator error and reaches real PASTEVB. Internal-call failure reaches
actual POODOO with Q selecting original inline01501; it does not manufacture
an alarm or claim abort recovery has completed. ALM/END reaches PINBRNCH.

The three fixed-fixed lamp bodies support all aligned FB/EB, eight SUPER
latches and IRQ0/1, returning to ordinary Q0100..7777 with actual Q3 debris.
Banked entries require their actual bank40/41/43 and SUPER100/120/140/160.
ENTRET and DSPLOCK are arbitrary ordinary words: only actual matching source
return descriptors select monitor/internal-call branches. Canonical image is
sole authority; caller owns restoring producer exclusion. Unsupported image,
I/O, event output and route remain unchanged.

8192 complete2053-field original/native/fullASan/UBSan states cover eight
entries, all three display-alarm continuations, signed-zero ENTRET, all banks,
IRQ, raw A/L and channel edges. Full SHA256:
`8b5ce01b749a03a6541cc0ed1c89c1012261f7deb3724986bd77f75491cde9a4`.
The oracle executes unchanged complete original ropes, never the native body;
nine-file source audits and complete pinned-rope equality protect each
mission's addresses and source behavior independently.

Sixty-four frozen complete checkpoints pass strict host, sanitizers and native
stock-ROM C64 VICE. Mailbox a500004002; PRG6536 loaded6534 bytes; hardware/
software stack extents14/47. tools/dsky_source_alarm.mk supplies portable host
aliases and the explicit C64 target. Actual PASTEVB, POODOO recovery and
selected PINBRNCH/application continuations remain separate source bodies.
