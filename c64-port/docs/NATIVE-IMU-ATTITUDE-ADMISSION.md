# Native IMU attitude admission and R02 controls

`imu_attitude_admission` translates all36 newly selected local source words per
mission from original `IMU_MODE_SWITCHING_ROUTINES.agc`: GOMANUR admission,
R02 status, R02ZERO flag call and the actual flag/alarm return instructions,
including six passive data words per mission. The additive ledger leaves79
standby/time-recovery words open in each containing file.

The API uses the complete 2048-word canonical image with IRQ/SUPER and
channels11/12/14. Inputs require ordinary15-bit RAM, raw16-bit A/L, coherent
EB3, actual containing bank07 (LM R02 uses bank04), fixed Q02000..07777,
quiet channel14 gyro/coarse activity and excluded producers. GOMANUR requires
its source saved fixed caller and aligned saved bank. Unsupported inputs remain
unchanged. Nonnull image/I/O objects are caller preconditions.

| Stage | LM entry | CM entry | True selected boundary |
| --- | --- | --- | --- |
| MANEUVER | 03750 | 03627 | POSTJUMP or POODOO |
| STATUS | 03254 | 03653 | UPFLAG or VARALARM |
| SET_USE | 03270 | 03667 | UPFLAG |
| FLAG_RETURN | 03272 | 03671 | SWRETURN |
| ALARM_RETURN | 03267 | 03666 | GOTOPOOH |

GOMANUR's BZF treats either signed zero ATTCADR as free. Other values select
literal alarm1210. Success exchanges BUF2 into ATTCADR, preserves the old minor
word in L, adds retained EB bits to the saved aligned bank and masks PRIORITY
with037000 for ATTPRIO. KALEBCON selects EB6 before the true POSTJUMP.
R02 tests REFSMBIT010000; a set bit calls UPFLAG with diminished07777. Missing
reference selects alarm0210 when IMODES30 BIT9 is set, otherwise0220. Source
TC and TCF distinctions retain or change Q at their actual instructions.
The native bodies stop at genuine external callee entries. They do not run
flag, alarm, attitude, bank-return or display services and add no Executive
check at internal C returns.

`imu_attitude_literal` exposes six original passive words per mission by
physical fixed address. All131072 accessor arguments match the independent
original word map; invalid mission/null output/unowned address preserves output.
Fixtures, original declarations and disclosed indentation-only CM repairs are
pinned by `evidence/imu-pulse-source-audit.json`; original rope bytes are unchanged.
The GPL original source engine is host-only and absent from native targets.

The constructor cohort contains65536 complete original states per stage and
mission,655360 total. Every2056-word row retains identity, source latches,
status and all2048 RAM words. Strict native and full ASAN/UBSAN streams match
the original byte for byte. Varied ATTCADR signed values, flags, saved callers,
raw A/L, source channels and nonzero unowned RAM patterns provide a reproducible
cohort; no exhaustive Cartesian claim is made. One deterministic XZ retains each
original stream; native/sanitized outputs compare directly to its decoded bytes.

Ten actual stock PAL C64 diagnostics pass320 complete original-frozen states,
120 unchanged guard calls and60 passive literal checks. PRGs, maps, mailboxes and
measured hardware/software stack dumps are retained; compiler ZP allocation is
zero and allocated sections end below C000. Isolated reproduction uses
`tools/imu_attitude_admission.mk` all/test/sanitize/check-host and mission/entry
vice-test. Regeneration creates separate build archives. This packet provides
local source coverage without complete containing-file or application credit.
