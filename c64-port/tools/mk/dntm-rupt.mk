# SPDX-License-Identifier: MIT
# The flight store's DOWNRUPT ends (Lane E): the DOWNRUPT vector and RESUME
# on the DNTM io records, packed beside the down-telemetry tables so one
# module run takes a whole DOWNRUPT, against the routines they copy.
DNTM_RUPT_SRC = src/lm_dntm_rupt.c src/cm_dntm_rupt.c src/rupt_lead_ins.c src/lm_t6rupt.c src/lm_downtelemetry.c src/cm_downtelemetry.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_transfer.c src/agc_word.c src/agc_multiword.c tests/lm_downtelemetry_cases.c tests/cm_downtelemetry_cases.c
DNTM_RUPT_HDR = include/dntm_rupt_routines.h include/rupt_lead_ins.h include/lm_t6rupt_routines.h include/lm_downtelemetry.h include/cm_downtelemetry.h include/agc_transfer.h include/agc_image_ops.h tests/lm_downtelemetry_cases.h tests/cm_downtelemetry_cases.h
.PHONY: test-dntm-rupt sanitize-dntm-rupt
all: build/test-dntm-rupt
test: test-dntm-rupt
sanitize: sanitize-dntm-rupt
build/test-dntm-rupt: $(DNTM_RUPT_SRC) tests/test_dntm_rupt.c $(DNTM_RUPT_HDR) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DNTM_RUPT_SRC) tests/test_dntm_rupt.c -o $@
test-dntm-rupt: build/test-dntm-rupt
	./build/test-dntm-rupt
sanitize-dntm-rupt: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(DNTM_RUPT_SRC) tests/test_dntm_rupt.c -o build/test-dntm-rupt-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-dntm-rupt-sanitize
