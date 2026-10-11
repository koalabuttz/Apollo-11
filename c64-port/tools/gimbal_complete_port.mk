# SPDX-License-Identifier: MIT
GIMBAL_COMPLETE_PORT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c src/guidance_shift.c src/guidance_sum.c src/guidance_trig.c src/guidance_dot.c src/guidance_dp_divide.c src/delcomp.c src/gimbal_stage.c src/gimbal_complete.c tests/delcomp_cases.c tests/gimbal_complete_cases.c
all: build/test-gimbal-complete-port
build/test-gimbal-complete-port: $(GIMBAL_COMPLETE_PORT_CORE) tests/test_gimbal_complete.c | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_gimbal_complete.c $(GIMBAL_COMPLETE_PORT_CORE) -o $@
test: test-gimbal-complete-port
test-gimbal-complete-port: build/test-gimbal-complete-port
	./build/test-gimbal-complete-port
sanitize: sanitize-gimbal-complete-port
sanitize-gimbal-complete-port: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined tests/test_gimbal_complete.c $(GIMBAL_COMPLETE_PORT_CORE) -o build/test-gimbal-complete-port-sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test-gimbal-complete-port-sanitize
.PHONY: test-gimbal-complete-port sanitize-gimbal-complete-port
