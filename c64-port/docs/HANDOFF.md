# First milestone handoff

## Repository and branch

Local checkout preserves the full upstream history at
`https://github.com/chrislgarry/Apollo-11`, based on
`911e5c0283c629c50cb97666f34065e8c07d71a5`.
Branch: **port/c64-native-c**. No merge into master has occurred.
The final commit is recorded in the archive's `HANDOFF.txt`; recover it locally
with `git rev-parse port/c64-native-c`.

**GitHub blocker:** authentication identifies `koalabuttz`, but available GitHub
actions do not include fork/repository creation; there is no gh CLI credential.
No fork URL exists and no push was attempted. Remaining external action: fork
chrislgarry/Apollo-11 into that account, add the fork as a push remote, and push
this development branch. The local bundle preserves all commits and historical
files, so this does not depend on the transient checkout.

```sh
git clone Apollo-11.bundle Apollo-11
cd Apollo-11
git switch port/c64-native-c
git remote rename origin bundle
git remote add upstream https://github.com/chrislgarry/Apollo-11.git
# After creating the GitHub fork and authenticating locally:
git remote add origin https://github.com/koalabuttz/Apollo-11.git
git push -u origin port/c64-native-c
```

Do not create that origin remote until the fork actually exists. These are
follow-up instructions, not a claim that the external actions succeeded.

## Implemented original paths

Native C translates NOVAC/FINDVAC, FINDVAC2/VACFOUND, NOVAC2/NOVAC3/NEXTCORE,
CORFOUND/SPECTEST/SETLOC, and ENDJOB1/EJSCAN/EJ1/EJ2 at explicitly bounded saved
state transitions. Resource exhaustion preserves the 1201/1202 trigger boundaries,
including a retained VAC on 1202. Allocation uses all eight historical contexts.
The reusable word layer preserves both zeros and required one's-complement
representation, sign extension, carry and overflow correction.

`README.md` contains portable build/test commands. `evidence/executive.prg` is
the retained 5829-byte native executable; reproducible output is
`build/executive.prg`. The stock PAL C64 VICE run reports zero failures for
26 scenarios and 26 assembly-derived state fingerprints. Host tests compare
each saved word with original assembly and compare arithmetic separately against
Virtual AGC helpers. Exact commands, results and limits are in `VERIFICATION.md`.

This is a verified beginning, not a functioning flight OS. No admitted job is
actually dispatched yet, and no WAITLIST/timer/interrupt/restart/guidance path is
implemented. The 657-cycle measurement covers a single admission. Executive
state occupies 648 bytes; the complete diagnostic has 658 bytes of BSS.
Full fidelity and resource records are in `FIDELITY.md` and `PERFORMANCE.md`.

## Acceptance checklist

- [x] Clearly documented local working branch with full upstream history.
- [x] Historical source unchanged, verified against pinned base.
- [x] `c64-port/` exists with reproducible instructions and dependency pins.
- [x] Portable C builds/runs under host compiler.
- [x] Native C64 PRG builds with LLVM-MOS.
- [x] Native C64 diagnostic tested in VICE.
- [x] Tested minimal AGC numerical foundation.
- [x] Executive structures/dependencies/invariants documented.
- [x] Genuine bounded Executive paths translated into C.
- [x] Automated tests for translated paths.
- [x] Source-to-C traceability recorded.
- [x] Gaps/adaptations/unavailable verification recorded.
- [x] Concrete next milestone defined.
- [ ] Actual GitHub fork/push (local fallback criterion is met).
- [ ] Physical C64 verification (not required for this assignment).

The recommended next milestone is **basic CHANJOB context exchange and native
dispatch**, with sequential reference comparisons and actual native job execution.
The unresolved continuation/IRQ/bank/restart questions are listed in ROADMAP.md.

Review: executable independent assembly comparisons caught allocation and scratch
state mistakes; numerical helper comparisons cover the initial layer. No second
agent/human review occurred. License boundaries are explicit: native C is MIT /
historical public-domain translation; the separate host-only reference adapter
is GPL-2.0-or-later, with no AGC emulator linked into the native PRG.
