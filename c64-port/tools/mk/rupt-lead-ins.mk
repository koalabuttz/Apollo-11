# SPDX-License-Identifier: MIT
# Milestone rupt-lead-ins (lane A): INTERRUPT_LEAD_INS.agc of Luminary099 and
# Comanche055 and Comanche's RESUME/NOQRSM/NOQBRSM interrupt exits as native
# routines (lm_lead_in_routines, cm_lead_in_routines).
RLI_SRC = src/rupt_lead_ins.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/rupt_lead_ins_cases.c
RLI_HDR = include/rupt_lead_ins.h include/agc_transfer.h include/agc_image_ops.h tests/rupt_lead_ins_cases.h
RLI_FLAGS = -ffunction-sections -fdata-sections -Wl,--gc-sections
.PHONY: test-rupt-lead-ins sanitize-rupt-lead-ins c64-rupt-lead-ins vice-rupt-lead-ins rupt-lead-ins-differential
all: build/test-rupt-lead-ins
test: test-rupt-lead-ins
sanitize: sanitize-rupt-lead-ins
build/test-rupt-lead-ins: $(RLI_SRC) tests/test_rupt_lead_ins.c tests/rupt_lead_ins_vectors.h $(RLI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RLI_FLAGS) $(RLI_SRC) tests/test_rupt_lead_ins.c -o $@
test-rupt-lead-ins: build/test-rupt-lead-ins
	./build/test-rupt-lead-ins
sanitize-rupt-lead-ins: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror $(RLI_FLAGS) -fsanitize=address,undefined $(RLI_SRC) tests/test_rupt_lead_ins.c -o build/test-rupt-lead-ins-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-rupt-lead-ins-sanitize
# Host-only oracle comparison (needs deps/virtualagc and both listing builds).
build/rupt-lead-ins-native: $(RLI_SRC) platform/host/rupt_lead_ins_snapshot.c $(RLI_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RLI_FLAGS) $(RLI_SRC) platform/host/rupt_lead_ins_snapshot.c -o $@
rupt-lead-ins-differential: build/rupt-lead-ins-native
	python3 tools/rupt_lead_ins_freeze.py
	python3 tools/rupt_lead_ins_source_map.py
	git diff --exit-code -- tests/rupt_lead_ins_vectors.h tests/rupt_lead_ins_c64_vectors.h
c64-rupt-lead-ins: build/rupt-lead-ins.prg
build/rupt-lead-ins.prg: $(RLI_SRC) platform/c64/rupt_lead_ins_diagnostic.c tests/rupt_lead_ins_c64_vectors.h $(RLI_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror $(RLI_FLAGS) -Wl,-Map,build/rupt-lead-ins.map $(RLI_SRC) platform/c64/rupt_lead_ins_diagnostic.c -o $@
vice-rupt-lead-ins: build/rupt-lead-ins.prg
	python3 tools/vice_run.py build/rupt-lead-ins.prg --cycles 3000000000 --timeout 1200
