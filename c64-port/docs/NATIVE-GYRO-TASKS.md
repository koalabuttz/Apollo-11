# Native deferred gyro segment tasks

`imu_gyro_task` translates original LM/CM STRTGYRO and8192AUG through actual
WAITLIST, JOBWAKE or IMUBAD handoffs. It computes gyro selection, positive or
negative torquing mode, minimum-command suppression, fractional residuals,
initial pulse count, repeated8192-pulse augment count and source segment delay.
Command pairs and source scratch preserve the original read/exchange order.
The phase counter traverses Y, Z and X before the final wake handoff.

`imu_gyro_register` executes the original successful WAITLIST call with the
literal next-task pair, follows the actual GYROEXIT WOR activity bit, and stops
before the literal TCF TASKOVER instruction. Source GYROCMD and channel14 now
represent a pending pulse train. IRQ remains inhibited after registration.
The scheduler transfer and asynchronous pulse delivery are external services.
The JOBWAKE and IMUBAD statuses stop before those callee bodies; there is no
implicit wake, alarm, job completion or hardware consumption.

The caller owns nonnull image/device objects exclusively, including source
counter producers. RAM is ordinary15-bit, A/L raw16, bank07 and EB/BB coherent,
REGZERO0. Optics/gyro/coarse command counters enter idle0. Initialized LGYRO
selects phase0..3 and a bounded within-bank E4..E7 command region disjoint from
source scratch/queue. The independently qualified augmentation domain holds
channel14 activity bit01000 clear on entry; active-channel augmentation and
counter consumption need separate device qualification. That restriction is
checked before mutation. Registration requires a valid initialized nonfull
queue; unsupported queues are not a rollback contract for source mutations.

The retained constructor corpus contains524,288 complete2048-word states plus
IRQ, SUPER and channels11/12/14: both missions, STRTGYRO/8192AUG entries, and two
full-prefix boundaries. Inputs vary all four command banks, all four gyro
phases, caller EB/SUPER, signed raw command pairs, zero-command skips, caging
and augmentation channel12 failure tests. Queues are empty initialized queues
with varied TIME3. This is the declared deterministic corpus, not exhaustive
Cartesian coverage. Original, strict native and complete ASAN+UBSAN streams
are byte-identical; deterministic XZ retains one shared object per context.

Eight actual C64 diagnostics compare256 frozen original-only full states,
including every untouched erasable word and all five device latches. Artifacts
retain compiler maps, zero compiler ZP, stacks and mailbox observations.
Target frozen WAITLIST enum signedness warnings remain visible under
`-Wno-error=sign-conversion`; host strict conversions remain errors.

Original IMU source fixture/audit remains the immutable IMUPULSE dependency:
byte-original LM and four indentation-only CM repairs, whole ropes1f5326… and
6e3150… unchanged. The new source map binds actual bank07 words and constants.
This qualifies the stated source groups only; whole IMU and RTB/RT8 closure
still require the explicit remaining lifecycle and hardware continuations.
