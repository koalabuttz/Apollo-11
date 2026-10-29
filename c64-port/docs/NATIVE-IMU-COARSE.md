# Native IMU coarse alignment stages

`imu_coarse` translates 120 original local words per mission from
`IMU_MODE_SWITCHING_ROUTINES.agc`. This adds SETCOARS/RNDREFDR, the complete
COARS angular-difference calculation, COARS2 three-axis command segmentation,
and the settled-angle check. It is a partial containing-file milestone.

The canonical 2048-word source image remains authoritative. The API takes the
mission and a compiled stage, not an AGC instruction stream. It preserves every
unowned image word and the source IRQ/SUPER/channel latches. Ordinary 15-bit
memory and raw 16-bit A/L, coherent bank07/EB3, fixed Q02000..07777, excluded
producers and idle command counters are required. Idle counters accept either
signed zero; a negative-zero gyro counter additionally requires its activity
bit clear. The three coarse CDU activity bits must be clear. Unsupported inputs
return without mutation. Positive counters or active coarse pulse transport
require a separate device owner and are not admitted into these numeric calls.

| Stage | LM entry | CM entry | Actual returned boundary |
| --- | --- | --- | --- |
| SET | 03144 | 03027 | Source TC Q, Q3; existing-coarse shortcut retained |
| CALCULATE | 03010 | 02673 | VARDELAY05224/05272, A2, Q03040/02723 |
| SEGMENT | 03040 | 02723 | SENDPULS03133/03016 or FIXDELAY05221/05267 |
| CHECK | 03072 | 02755 | ENDIMU03631/03514 or ALARM05567/05650 |

CALCULATE and SEGMENT first execute the literal CAGETEST result: a caged IMU
returns at actual IMUBAD03637/03522. Its callee body is a separate qualified
service. CHECK begins at CAF TWO, immediately before internal CHKCORS.

The angular difference is the source modulo subtraction with negative
diminution. The quarter multiply and low-word DOUBLE/TS rounding preserve raw
signed-zero results, the old command word in L and the final ITEMP1 residue.
Desired-angle addresses differ: LM THETAD0321, CM THETAD01155. Command words
01471..01473 and CDUIND01474 are shared source bindings.

Segmentation uses source CCS magnitude minus one and literal -191. Each nonzero
axis issues at most 192 pulses while retaining its signed remainder. The
COMNEG and COMZERO TC NEXTCDU sites leave their distinct Q values. The numeric
return stops before SENDPULS writes channel14: no pulse is delivered by this
API. With all axes zero, the literal FIXDELAY parameter is 150 centiseconds.
The settled test uses literal077511 (-182); CCS diminution admits an absolute
difference up to 183 raw counts and sends 184 or more to ALARM0211. The alarm
callee and subsequent local TCF IMUBAD are not executed or credited here.

`imu_coarse_literal` exposes six original local data words using physical
fixed-word addresses: inline delay150, inline alarm0211, COARSTOL, OCT40010,
-COMMAX and -COMMAX-. It leaves the output unchanged for unowned addresses.
The selected-word ledger includes these values and excludes SENDPULS, CA+ECE,
IMUCOARS admission, all init/fine/standby bodies and the post-alarm continuation.

The unchanged original assembled fixtures use the original declarations and
retain their existing disclosed indentation-only CM repairs. The authoritative
source audit is `evidence/imu-pulse-source-audit.json`; no instruction words are
patched. The host-only GPL source engine is an independent oracle and is not
linked into native target binaries.

Verification covers a constructor cohort of 65,536 complete states per mission
and stage: 524,288 states in total, each 2056 words including case identity,
IRQ/SUPER, channels11/12/14, returned status and all 2048 erasable words. Strict
native and full ASAN/UBSAN streams match every byte of the original streams.
The factory varies actual angles/commands/latches, signed-zero idle counters,
raw A/L and nonzero unowned-word patterns, with targeted 191/192/193 segment
and 182/183/184 tolerance cases. It is not an exhaustive Cartesian claim.

Eight stock PAL C64 diagnostics check 256 complete source-frozen states,
96 unchanged-state guard calls and 48 constant values. Compiler ZP allocation
is zero, all allocated sections end below C000, and measured hardware/software
stack bounds are retained with the target PRGs, maps and monitor output.
The constant accessor additionally passes every 65,536 possible address for
each mission against the original word ledger. Reproduction uses the isolated
`tools/imu_coarse.mk` all/test/sanitize/check-host/vice-test targets. Source-only
fixtures come from `tools/freeze_imu_coarse.py`.

Only one deterministic XZ object per original full stream is retained; strict
and sanitized streams are compared directly. This establishes these local
source stages and handoffs. It does not execute external WAITLIST/delay/alarm
services, asynchronous CDU delivery, or a complete mission application.
