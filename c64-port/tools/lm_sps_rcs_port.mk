# SPDX-License-Identifier: MIT
LM_SPS_RCS_PORT_CORE = src/lm_sps_rcs.c src/agc_word.c src/agc_multiword.c tests/lm_sps_rcs_cases.c tests/frozen_words.c
all: build/test-lm-sps-rcs-port
build/test-lm-sps-rcs-port: $(LM_SPS_RCS_PORT_CORE) tests/test_lm_sps_rcs.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lm_sps_rcs.c $(LM_SPS_RCS_PORT_CORE) -o $@
test: test-lm-sps-rcs-port
test-lm-sps-rcs-port: build/test-lm-sps-rcs-port
	./build/test-lm-sps-rcs-port
sanitize: sanitize-lm-sps-rcs-port
sanitize-lm-sps-rcs-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie tests/test_lm_sps_rcs.c $(LM_SPS_RCS_PORT_CORE) -o build/test-lm-sps-rcs-port-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-lm-sps-rcs-port-san
.PHONY: test-lm-sps-rcs-port sanitize-lm-sps-rcs-port
