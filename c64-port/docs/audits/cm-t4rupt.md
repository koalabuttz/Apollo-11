# Certificate review: Comanche055/T4RUPT_PROGRAM.agc

Reviewed 2026-10-14 by the certificate reviewer at `port/c64-native-c` ab6e4aa5,
after Kernel lane PR #293, against the certificate drafted by milestone
`cm-t4rupt-chain`. That certificate builds on `t4rupt-core`, `cm-t4rupt-imu`
and `cm-t4rupt-optics`. The certificate and native sources are unchanged at
3e390294. The bar is the same as for the accepted LM review
(docs/audits/lm-t4rupt.md).

Verdict: **accepted**.

## What was checked

- **Evidence.** All four categories are distinct records, and none cites the
  source map. Every hash matches, `port_coverage.py` passes, and
  `cert_precheck.py` reports 0 FAIL.
- **Reproduced at ab6e4aa5.**
  - Host and sanitizer targets give 0 failures for t4rupt-core (11,400),
    cm-t4rupt-imu (9,577), cm-t4rupt-optics (274) and cm-t4rupt-chain (12,295).
    The chain returns 4096 of 4096 interrupts with nothing refused.
  - yaAGC differentials on the unmodified Comanche055 rope (6e3150ae…) are
    equal to native for core (2,240 LM + 2,176 CM), imu (4,736), optics (4,096)
    and the whole-interrupt chain (4,096).
  - The vectors, C64 vectors and groups refreeze unchanged.
  - Every proof and source map, closure included, regenerates byte for byte.
- **VICE.** t4rupt-core 279, cm-t4rupt-imu 300, cm-t4rupt-optics 46 and
  cm-t4rupt-chain 269 checks, all with 0 failures. These equal `vice_targets`,
  and all four PRG hashes equal the closure VICE log.
- **Closure map.** It covers 833 words, and 772 of 773 instruction lines run.
  The one exception is 10,2110, which is unreachable. A is always +0 or
  0140000 there, because MASK sign-extends and RXOR L is a 16-bit XOR, so CCS
  never takes the positive slot. ZOPFIN1 is mapped to the listing's ZOPFINI
  (06,3057).
- **C64 cover.** This was checked with an independent per-object
  `gcc -O0 --coverage` build over the exact host cases and C64 rows: chain
  269 rows against 4,096 cases, optics 46 against 4,096, imu 300 against 4,736,
  and core. The C64 rows miss no native host arc. The chain rows equal the
  diagnostic loops and the cover record.
- **Transfer ABI.**
  - T4RUPT (06,2000 via the lead-in), the NOVAC, IBNKCALL, WAITLIST, ALARM and
    VARALARM returns, and the IMUMON, OPTMON, OPTDRIVE and SETISSW entries are
    all registered at their original addresses.
  - The only labels other Comanche files reference are T4RUPT and SETISSW.
  - Exits stop at RESUME, NOVAC, IBNKCALL, ALARM, VARALARM and WAITLIST.
  - SUPERBNK is randomized and compared.
- **Steward rule.** The optics spec sets `index_carry16`, and the generated
  `ixw` carries end around.
- **NEWJOB WARN.** The cert_precheck NEWJOB WARN at `cm_t4rupt_optics.c:36` is
  in dead code. `sane()` is reached only from the uncalled
  `at_newops`/`at_basic`, and `cm_t4rupt_optics_interpretive` is empty.

## Notes for the lane (not blocking)

- **TLIM `CCS IMODES30` = −0 is uncompared for CM** (T4RUPT_PROGRAM.agc:673,
  06,2511; `src/cm_t4rupt_imu.c:237-240`). It needs every IMODES30 bit set at
  once. The LM review judged that implausible in flight, and this review rates
  it the same way. LM has since added the case (#284). Add the matching CM
  case (IMODES30 037777, channel 30 077777) at the next imu refreeze, or state
  it in the supported domain.
- **DSPOUT `CCS FLAGWRD5` = −0** is compared for CM on the host. On the C64,
  however, only LM core row 143 takes it. Add CM core case 2240+143 as a cover
  row.
- **Stale log.** cm-t4rupt-optics' own release VICE log still records the PRG
  from before the refreeze. Refresh it. The certificate cites the current
  closure log.
- **Domain wording.** The domain says out-of-domain channel 13/14 inputs
  return EXEC_OUT_OF_SCOPE, but no routine refuses them. This is the same
  wording issue as LM.
- **Separate channel records.** The optics routines keep channels in
  cm_native_io/cm_channels, while core and imu use agc_hardware. A flight
  build must merge them (Lane D1 channel model).
- **Cosmetic.**
  - The closure map labels 10,2110 "STORCMD".
  - docs/CM-T4RUPT-IMU.md:9 gives IMUMON as 06,2172; CM's address is 06,2174.
