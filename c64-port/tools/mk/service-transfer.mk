# SPDX-License-Identifier: MIT
# Milestone service-transfer (lane A): SERVICE_ROUTINES entries of both
# missions as control-transfer ABI routines, checked against yaAGC.
SVT_SRC = tests/service_transfer_cases.c tests/service_transfer_check.c src/service_transfer.c src/service_routines.c \
	src/agc_transfer.c src/agc_word.c src/agc_multiword.c src/agc_image_ops.c src/agc_hardware_divide.c
SVT_HDR = include/agc_transfer.h include/service_transfer.h include/service_routines.h include/service_routines_source.h \
	src/lm_service_sites.inc src/cm_service_sites.inc tests/service_transfer_cases.h tests/service_transfer_check.h \
	tests/service_transfer_vectors.h
.PHONY: test-service-transfer sanitize-service-transfer c64-service-transfer vice-service-transfer service-transfer-differential
all: build/test-service-transfer
test: test-service-transfer
sanitize: sanitize-service-transfer
build/test-service-transfer: $(SVT_SRC) tests/test_service_transfer.c $(SVT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SVT_SRC) tests/test_service_transfer.c -o $@
test-service-transfer: build/test-service-transfer
	./build/test-service-transfer
sanitize-service-transfer: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(SVT_SRC) tests/test_service_transfer.c -o build/test-service-transfer-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-service-transfer-sanitize
# Host-only oracle run (needs deps/virtualagc and both rope builds).
service-transfer-differential:
	python3 tools/service_transfer_freeze.py
	git diff --exit-code -- tests/service_transfer_vectors.h
	python3 tools/agc_sites_generate.py lm-service --check
	python3 tools/agc_sites_generate.py cm-service --check
c64-service-transfer: build/service-transfer.prg
build/service-transfer.prg: $(SVT_SRC) platform/c64/service_transfer_diagnostic.c $(SVT_HDR) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/service-transfer.map $(SVT_SRC) platform/c64/service_transfer_diagnostic.c -o $@
vice-service-transfer: build/service-transfer.prg
	python3 tools/vice_run.py build/service-transfer.prg --cycles 3000000000 --timeout 1800
