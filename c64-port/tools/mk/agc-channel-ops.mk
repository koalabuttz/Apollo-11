# SPDX-License-Identifier: MIT
# The I/O channel model of compiled Luminary routines (include/agc_channel_ops.h).
CHOPS_SRC = tests/test_agc_channel_ops.c src/agc_image_ops.c src/agc_hardware_divide.c src/agc_word.c src/agc_multiword.c
.PHONY: test-agc-channel-ops sanitize-agc-channel-ops
all: build/test-agc-channel-ops
test: test-agc-channel-ops
sanitize: sanitize-agc-channel-ops
build/test-agc-channel-ops: $(CHOPS_SRC) include/agc_channel_ops.h include/agc_input_channels.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CHOPS_SRC) -o $@
test-agc-channel-ops: build/test-agc-channel-ops
	./build/test-agc-channel-ops
sanitize-agc-channel-ops: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(CHOPS_SRC) -o build/test-agc-channel-ops-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-agc-channel-ops-sanitize
