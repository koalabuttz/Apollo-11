# Comanche 055 shared numerical qualification

Original CM assembly now independently qualifies the unchanged MIT `guidance_norm`, `guidance_dp_divide` and `guidance_unit` native arithmetic. These bounded numerical operations use ordinary words and raw A/L registers; they retain every modeled scratch, VAC, MPAC and output word. They do not execute a decoder, load rope into the native target, or provide the surrounding CM flight-program logic. Existing generic input structures retain their `p66_` names; this qualification concerns arithmetic and interpreter helpers common to both applications.

| Original CM entries | Source cases | Complete words per case | C64 full vectors | Source snapshot SHA256 |
|---|---:|---:|---:|---|
| SQRT/SQRTSUB | 132096 | 20 | 637 | `cb141db0803cd747bc022b65f779e71df0590eace82af5fe037ffb0fa0b311d2` |
| DDV/BDDV | 163840 | 17 | 573 | `43f40a751558ac2f30deb014d24925940f91c449c5f0e1c3fa74f043ac8c9c42` |
| ABS/ABVAL/UNIT | 50736 | 104 | 118 | `fdeebfdb5b7d2d1aeb707df1beb1bc5a538a2f7af1ef2ff0bf865db3e7f32e28` |

Each row includes the routine status in addition to the indicated owned words. Source snapshots are byte-identical to fresh native snapshots and to the provenance of the existing lossless frozen vectors. Host and ASan/UBSan run those complete vectors plus unsupported-input no-mutation checks. The original host-only GPL oracle uses actual CM addresses: DDV `00,7546`, BDDV `00,7552`, DANZIG `06030`. SQRT/SQRTSUB and UNIT addresses match the original LM labels, but CM interpreter return and dispatch addresses are independently resolved.

The fixture is `deps/cm-numeric-fixture`, pinned Virtual AGC `1b5d6474841a7ab53237ead070f76f35f162f68d`. Original CM INTERPRETER and POWERED_FLIGHT_SUBROUTINES overlays are byte-preserved beside the fixture. There are 36 and one pseudo-label indentation repairs respectively, with no semantic edit. Both the complete normalized-original assembly and a separate assembly of the pinned corrected sources have rope SHA256 `2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79`. Historical originals remain untouched. Original/normalized hashes, patches and both listings are retained under `evidence/cm-numeric-*`; the supporting service overlays are audited separately under `evidence/cm-services-*`.

The host oracle selects the pinned engine's existing `SimulateDV` path for all DV operands, including signed zero. The single branch-selection patch, original engine hash `9c59728cdd968503caaaafe50a18ab3ed841f6176da3299c137f3a6a158fcbef` and variant hash `3fcdb1fe93d24e5bd25eebfb85e9f3bec43fcd84575539ac3d397c342004f242` are recorded. This changes no native code and introduces no GPL dependency into the target.

Stock PAL C64 passes all 1328 complete frozen vectors. SQRT mailbox `a500007d02`, PRG 35536 bytes, final image end `$93bd`, hardware/software stack extents 30/158 bytes. DDV mailbox `a500003d02`, PRG 29320, end `$7b3d`, stacks 24/135. UNIT mailbox `a500007600`, PRG 44655, end `$b740`, stacks 30/293. Each diagnostic fits below `$c000` with no additional zero-page sections. These standalone diagnostic sizes do not establish whole-application overlay fit or WCET; UNIT application integration must retain its existing explicit split groups to fit the 16KB slot.

Reproduce with `make cm-numeric-differential test-cm-numeric sanitize-cm-numeric c64-cm-numeric`, then the three `tools/cm_{norm,dp_divide,unit}_vice_test.py` scripts with `--vice`, `--rom-dir` and `--data-dir`. Before shared Makefile integration, add `-f Makefile -f tools/cm_numeric.mk`. Differential preparation can bootstrap the audited CM service fixture. Evidence hashes are `evidence/cm-numeric-evidence.sha256`.
