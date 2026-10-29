# SPDX-License-Identifier: MIT
# Milestone self-check (lane A2): Luminary099 and Comanche055 AGC_BLOCK_TWO_SELF-CHECK (bank 43)
# as routines of the control-transfer ABI.
SELF_CHECK_CORE = src/lm_self_check.c src/cm_self_check.c src/lm_self_check_idle.c src/cm_self_check_idle.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c tests/self_check_cases.c
SELF_CHECK_SRC = $(SELF_CHECK_CORE) src/rope_image_host.c
SELF_CHECK_C64_COMMON = src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/self_check_cases.c src/overlay_reu.c platform/c64/overlay_reu_platform.c platform/c64/rope_image_reu.c platform/c64/self_check_diagnostic.c
SELF_CHECK_C64_LM = src/lm_self_check.c src/lm_self_check_idle.c src/lm_interbank_routines.c src/lm_interbank_idle.c src/lm_interbank.c $(SELF_CHECK_C64_COMMON)
SELF_CHECK_C64_CM = src/cm_self_check.c src/cm_self_check_idle.c src/cm_interbank_routines.c src/cm_interbank_idle.c src/cm_interbank_complete.c $(SELF_CHECK_C64_COMMON)
SELF_CHECK_ROPES = build/fixed-data/Luminary099/MAIN.agc.bin build/fixed-data/Comanche055/MAIN.agc.bin
SELF_CHECK_HDR = include/self_check.h include/rope_image.h src/self_check_body.inc src/lm_self_check_operands.inc src/cm_self_check_operands.inc include/agc_transfer.h include/agc_image_ops.h tests/self_check_cases.h
.PHONY: test-self-check sanitize-self-check c64-self-check vice-self-check self-check-differential
all: build/test-self-check
test: test-self-check
sanitize: sanitize-self-check
build/test-self-check: $(SELF_CHECK_SRC) tests/test_self_check.c tests/lm_self_check_vectors.h tests/cm_self_check_vectors.h tests/lm_self_check_c64_vectors.h tests/cm_self_check_c64_vectors.h $(SELF_CHECK_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SELF_CHECK_SRC) tests/test_self_check.c -o $@
test-self-check: build/test-self-check $(SELF_CHECK_ROPES)
	./build/test-self-check $(SELF_CHECK_ROPES)
sanitize-self-check: $(SELF_CHECK_ROPES) | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SELF_CHECK_SRC) tests/test_self_check.c -o build/test-self-check-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-self-check-sanitize $(SELF_CHECK_ROPES)
# Host-only oracle comparison (needs deps/virtualagc and the Luminary099 and Comanche055 listing builds).
build/self-check-native: $(SELF_CHECK_SRC) platform/host/self_check_snapshot.c $(SELF_CHECK_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SELF_CHECK_SRC) platform/host/self_check_snapshot.c -o $@
self-check-differential: build/self-check-native
	python3 tools/self_check_freeze.py
	git diff --exit-code -- tests/lm_self_check_vectors.h tests/lm_self_check_c64_vectors.h tests/cm_self_check_vectors.h tests/cm_self_check_c64_vectors.h
c64-self-check: build/self-check-lm.prg build/self-check-cm.prg
build/self-check-lm.prg: $(SELF_CHECK_C64_LM) tests/lm_self_check_c64_vectors.h $(SELF_CHECK_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DSELF_CHECK_LM_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/self-check-lm.map $(SELF_CHECK_C64_LM) -o $@
build/self-check-cm.prg: $(SELF_CHECK_C64_CM) tests/cm_self_check_c64_vectors.h $(SELF_CHECK_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -DSELF_CHECK_CM_ONLY -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/self-check-cm.map $(SELF_CHECK_C64_CM) -o $@
# Both ropes in a 2 MB REU image (include/rope_image.h).
build/self-check.reu: tools/rope_image_pack.py $(SELF_CHECK_ROPES) | build
	python3 tools/rope_image_pack.py $@
vice-self-check: build/self-check-lm.prg build/self-check-cm.prg build/self-check.reu
	python3 tools/vice_run.py build/self-check-lm.prg --reu build/self-check.reu --cycles 8000000000 --timeout 2400
	python3 tools/vice_run.py build/self-check-cm.prg --reu build/self-check.reu --cycles 8000000000 --timeout 2400
