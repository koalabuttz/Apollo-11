# Native IMUPULSE registration and return

The native LM and CM IMUPULSE services reproduce the original lead-in,
successful WAITLIST registration, gyro reservation, three descending command
pair sign corrections, and actual MODEEXIT/SWRETURN caller return. The busy
branch stops at actual JOBSLEEP; the caged branch executes its failure return.
Deferred gyro output tasks and the resumed GWAKE sleeper remain untranslated.
This is a partial IMU source milestone and does not certify whole RTB/RT8.

`imu_pulse_enter` takes the original bank07 entry and canonical image/device
state. Status0 selects WAITLIST, status1 selects JOBSLEEP, status2 completes
SWRETURN. `imu_pulse_register` executes the immutable mission WAITLIST adapter
with the literal original STRTGYRO pair: LM `{03405,016103}`, CM
`{03270,016063}`. Successful registration returns to03337/03222 with IRQ0.
`imu_pulse_finish` consumes that state, reserves LGYRO, corrects all three
command pairs using the original TPAGREE limit/carry sequence, restores the
caller's EB/FB, exchanges the saved FB word with016000, and returns with Q0134,
A=oldEB and IRQ1. Its completed-return status is also2.

Caller objects must be nonnull and own the canonical image and device latches
exclusively across the composition. A/L are raw16; RAM/device words ordinary
15-bit. Banks must be coherent and REGZERO0. Initialized LGYRO is nonnegative
with +0 allowed; negative initialized values are outside this contract.
Optics counters, GYROCTR and three coarse-alignment command counters must be
idle0. The original instruction engine otherwise consumes those hardware
output counters asynchronously, which is independent of this compiled body.
Channel14 itself remains varied and the actual WOR power-supply bit is kept.
The suffix's six-word command is within one E4..E7 bank, disjoint from service
scratch and WAITLIST arena. Registration requires a valid initialized nonfull
queue and TIME3; source alarm/full-list paths are outside this packet.
Unsupported caller states are rejected; callers must not use registration's
out-of-domain return as a rollback promise for an already-mutated source queue.

The retained proof contains393,216 complete2048-word original/strict/full
ASAN+UBSAN states, plus IRQ, SUPER and channels11/12/14, across six contexts:
two missions and three full-prefix checkpoints. The lead-in corpus includes
16,384 busy,16,384 free and32,768 caged returns per mission. Registration and
reservation each use65,536 empty initialized queue contexts per mission with
varied TIME3, all eight caller EBs, saved fixed caller banks, command bank and
raw command words. These are the deterministic constructor's cases, not an
assertion that every Cartesian input combination was exhausted.

Frozen target fixtures come solely from those original assembly states and
compare every erasable word and all five device latches. Actual C64 artifacts,
mailboxes, memory extents and stack observations are retained in the proof.
The target build retains sign-conversion warnings from frozen WAITLIST helper
status enums as warnings (`-Wno-error=sign-conversion`); host compilation keeps
strict conversion warnings as errors. No frozen helper was changed.

The isolated fixture overlays byte-original IMU_MODE_SWITCHING_ROUTINES on
the already-qualified original-declaration RTB fixture. LM needs no repair;
CM needs four disclosed indentation-only pseudo-label repairs. Complete ropes
remain1f5326… and6e3150… respectively. The source map records emitted words
and actual bank07 addresses, avoiding the superseded bank06 frontier text.
No added scheduling check occurs between these basic service stages.
