# Comanche real resident task/service overlays

The real C64 resident now executes the qualified continuous T3RUPT2 task loop with normal returned native C callbacks and actual D64 module loading. Its resolver stays below `$7000`, the sole 2048-word canonical image stays at `$7000..$7fff`, and each body occupies the shared `$8000..$bfff` slot only after its predecessor's C frame returned. The four mission modules execute actual task transfer, SETJTAG/CLOKTASK/SVCT3 bodies, WAITLIST/FIXDELAY and NOVAC/FINDVAC admission. Two separate proof-data overlays supply original-source inputs and expectations. No executable AGC rope is loaded or decoded.

| Module | Actual native implementation | Loaded bytes |
| --- | --- | ---: |
| 1 | T3RUPT2/TASKOVER canonical task transfer | 3015 |
| 2 | Actual SETJTAG/CLOKTASK/TIG-5/SVCT3 source groups | 7768 |
| 3 | Canonical WAITLIST/FIXDELAY service return | 5353 |
| 4 | Canonical NOVAC/FINDVAC caller return | 4215 |
| 250 | Independent original-source proof inputs | 3338 |
| 251 | Whole-image source fingerprints and complete sample | 7351 |

External CM I/O latches live at `$c400..$c41f`, resident service runtime at `$c430..$c43f`, and transient compiled call metadata at `$c440..$c45f`. Compile-time bounds enforce each lease. Diagnostic metadata uses `$c000..$c03f` and mailbox `$c080`; software stack uses `$c800..$cfff`. The flight image remains authoritative; overlays retain no private persistent flight words. Producer exclusion spans each actual group and load/call, while the shared platform restores the caller's processor I/D and `$01` ownership. Logical AGC interrupt-enable state remains explicit source metadata.

Stock PAL C64 ROMs and a real D64 virtual IEC run all six continuous source paths twice, for incoming C64 interrupts enabled and masked. All 58 complete 2048-word checkpoints match the independently retained original-source fingerprints. Both complete case-zero samples compare directly, and the captured final 4096-byte canonical image independently matches every source word of case 320 at actual hardware RESUME entry. Source-selected NBDONLY is genuinely admitted, CLOKTASK genuinely reschedules itself, and SVCT3 genuinely queues its delayed recheck. No dummy housekeeping task is skipped. C64 mailbox is `a500003a06`, resident PRG 6684 bytes, resident image end `$2232`, measured software-stack extent 773 bytes, and every real module fits the 16 KB slot with no private zero page. I and `$01` restore checks pass for both caller mask states.

The source proof is the unchanged `COMANCHE_SERVICE_LOOP.md` corpus: 384 cases/1856 exact sequential original-assembly states, SHA256 `fd6a0f1cdefdaa0e3d201e763976dd47c2e374a234bd018de6447ddd97284033`, full host and sanitizers. Counter drivers remain held zero and source scaler frozen in this comparison; the logical timer publisher and actual suspended DANZIG continuation are subsequent composition work. The proof ends before hardware RESUME and does not certify whole flight-application completion, source jobs not yet executed, or real-time WCET. The resident loader currently supplies the four qualified task/service bodies; longer restart/display/longcall mappings are registered by the resolver but need their own loader and continuation integration.

Native code is MIT; the original assembly oracle remains GPL and host-only. Historical source files are untouched. Reproduce with `python3 tools/cm_service_overlay_build.py --cc /path/to/mos-c64-clang --c1541 /path/to/c1541`, then `python3 tools/cm_service_overlay_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/vice`. Retain exact resident/module binaries, maps, D64, full captured image, stack/mailbox and proof identities with `python3 tools/cm_service_overlay_report.py`; manifest is `evidence/cm-service-overlay-evidence.sha256`.
