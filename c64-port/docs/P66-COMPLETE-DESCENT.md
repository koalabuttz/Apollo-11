# Native P66 descent and display composition

This milestone composes the translated Luminary 099 `RODCOMP` path through
vertical-rate feedback, the actual `THROTTLE` basic service, horizontal-speed
calculation, enabled `DISPEXIT`, `REGODSPR` admission of `MAKEPLAY`, the three-row
V06N60 display consumer, and the original `ENDOFJOB` return to idle `ADVAN`.
It starts with a lawful active P66 job and explicit producer inputs. Flight
startup, the upstream navigation producers, keyboard entry, and recurring
waitlist delivery are separate integrations.

`p66_descent_runtime.c` binds known source addresses to translated C callbacks.
It extends the previously verified P66 prefix and basic bindings. Additional
display callbacks come from `p66_display_job.c`. The resolver compares compiled
address records and context kinds; it does not read or decode source opcodes.

## State and scheduling

The sole authoritative erasable image remains the 2,048 words at `$7000..$7fff`.
Numerical payloads gather physical views on entry and commit them before normal
C return. Views and resident Executive metadata are caches, not independently
persistent copies of AGC state. External SUPERBNK, AGC interrupt allowance,
channels 11 and 14 remain explicit metadata.

Every completed interpretive pair returns to the Executive at its actual source
check. The driver includes the velocity boundaries at `03366/03370/03373`,
acceleration `03414/03416/03420/03421/03422`, projection
`03427/03430/03433`, all branch-dependent feedback checks, the `SETPD`
`NOIBNKSW` check at `03506`, and horizontal checks. Internal UNIT/ABVAL and
basic trigonometric calls can return to the resident loader while retaining
ownership. Pending EDOP prevents an invented scheduler exchange.

The basic throttle bridges preserve pending NEWJOB through source `EXIT`,
`BANKCALL`, and re-entry. The first horizontal VLOAD is the next genuine check.
`DISPEXIT` calls the actual source admission and completion helpers directly;
there is no extra scheduler call between a basic boundary and its source
continuation. Source-selected completion uses the qualified dispatched actions
to adopt the already selected basic, interpreter, or idle context.

`p66_descent_prefix_control.c` records the raw PC, Q, and CYR effects at explicit
compiled prefix completions. DDV return Q comes from the independently verified
control observer and the preoperation operands. No retained output words are
rewritten by the raw proof comparison.

## Target layout

| Address | Use |
|---|---|
| below `$7000` | Resident loader, callback router, immutable module metadata |
| `$7000..$7fff` | Canonical AGC erasable image |
| `$8000..$bfff` | One replaceable arithmetic, basic, or scheduler payload |
| `$c000..$c03f` | Diagnostic fixture metadata |
| `$c080..$c085` | Diagnostic mailbox |
| `$c100..$c387` | Transient 648-byte Executive view |
| `$c388..$c3d3` | Reserved task-view lease |
| `$c400..$c47d` | 126-byte runtime, binding, and IRQ queue metadata |
| `$c800..$cfff` | Normal C software stack |

Compile-time assertions limit the Executive and metadata leases. The disk
contains 96 distinct modules, including two proof-only frozen input/check
payloads. Scheduler prepare and finish return before a guidance or display
payload replaces the slot; no C frame or function pointer into unloaded code
survives a swap. Exclusion guards restore the caller's native IRQ state and
the loader restores `$01`. Native IRQ producer metadata remains separate from
the source AGC interrupt-allowance flag.

## Reproduction and scope

Run the strict source comparison with:

```sh
python3 tools/p66_complete_live_differential.py --raw-control
python3 tools/p66_complete_live_differential.py --raw-control --sanitize
python3 tools/p66_descent_pending_differential.py
python3 tools/p66_descent_pending_differential.py --sanitize
```

The raw complete corpus contains 16 enabled-display cases and 1,204 checkpoints.
Each compares all 2,048 canonical words plus the four external source values
against unchanged original assembly. The retained stream SHA256 is
`9480366d0d7203683dd1d35ae06736dca9ef93646fd795bf210d405ae6689c52`.
Linked mapping tables, the compiled resolver, and bound scheduler service seams
produce identical streams. Address aliases in the original oracle are qualified
by source bank, including actual idle `03214` in bank `02000`.

The separate later pending-job corpus injects 404 requests at the real feedback
and horizontal checks. Its 808 observations compare 535 modeled words before
CHANG2 and at the selected higher-priority RODCOMP entry. Its SHA256 is
`513b4674a0414fab4bce8f42bf9755bc404978eb7ed4d6ef5f11ff682b1cc8a9`.
That bounded corpus does not claim full raw CPU/debris coverage for arbitrary
multi-job CHANG2 exchanges.

For the disk target:

```sh
python3 tools/p66_complete_build.py --rebuild-stages --cc /path/to/mos-c64-clang --c1541 /path/to/c1541
python3 tools/p66_complete_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/vice-data
```

The target uses frozen original input images and fingerprints all 2,052 raw
words at 148 checkpoints across two cases. It runs the actual disk modules on
stock PAL C64 ROMs, with enabled and masked native IRQ callers and both source
SUPER forms. The stock-ROM target passes with mailbox `a50000940002`. The measured
software-stack extent is 772 of 2,048 bytes; the hardware sentinel extent is
70 bytes, with the top 14 ROM stack bytes outside the filled prefix. Additional
zero-page allocation is zero. The two native IRQ/SUPER cases are enabled/SUPER0
and masked/SUPER060. The proof packet records the exact PRG, D81, module
identities, full source dependencies, and captured mailbox/stacks. This milestone covers the stated manual P66 path; it does not certify
other landing programs or the complete Luminary/Comanche source set.
