# Luminary CDUTRIGS and CD*TR*GS

The canonical native basic trig loop computes all three CDU sine/cosine pairs, including source two-complement conversion and preservation of incoming MPAC. Original LM SINE/COSINE and CDULOGIC scratch/control addresses are retained. All 32768 CDU words occur for both live-CDU and stored-angle entry. Internal polynomial and basic-call returns retain the same owner; pending second operations do not create Executive checks.

The fixture overlays byte-original LM POWERED, INTERPRETER, RTB_OP_CODES and SINGLE_PRECISION sources with zero edits. It preserves original declaration/data rope `1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`. The containing file SHA256 is `379505fe1429c8b0d6ef20d85bad9d8fb991083fd7202dc89c46e584f0ba92b1`.

All 655,360 complete 2048-word-plus-IRQ checkpoints from 65,536 inputs match original assembly, strict native and whole ASAN/UBSAN execution. Every unowned physical image word is compared. Decoded SHA256 is `ccd35cfa9b838783d296193861f712f8d57ad69db4128b9a79dbe56a6f077ab3`. One lossless XZ object represents the three identical logical streams. Source-only frozen fixtures reconstruct all expected target images; expected flight values never enter the native numerical routines.

10 actual C64 PRGs pass 320 complete frozen states. Maximum loaded end is $4334; measured hardware/software stack maxima are 35/327 bytes, with compiler-reserved zero-page0. These are standalone native diagnostics. Resident16KB overlay allocation, application Executive composition and sensor/actuator integration remain separate acceptance work.

Reproduce after `python tools/lm_powered_prepare_reference.py` and the original hardware-DV oracle prerequisites with `make -f tools/lm_powered_trig.mk test sanitize check-host`. `python tools/freeze_lm_powered_trig.py` reconstructs the frozen fixtures. C64/VICE targets accept SELECTOR0..9 (default0), MOS_CC, VICE, C64_ROM_DIR and VICE_DATA_DIR. Native code follows explicitly compiled source operations; it does not decode AGC opcodes or use host floating point. External control retains producer exclusion and canonical scratch ownership across internal stages.
