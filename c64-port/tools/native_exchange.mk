NATIVE_EXCHANGE_CORE = src/agc_word.c src/executive.c src/native_dispatch.c src/irq_delivery.c src/p66_native.c src/lm_interpreter_change.c
NATIVE_EXCHANGE_TEST = tests/lm_interpreter_change_cases.c tests/lm_interpreter_exchange_native.c
NATIVE_EXCHANGE_FLAGS = -std=c99 -Wall -Wextra -Werror -Wconversion -pedantic -Iinclude -Itests -DP66_NATIVE_DISPATCH_ONLY -ffunction-sections -Wl,--gc-sections
.PHONY: test-native-exchange sanitize-native-exchange differential-native-exchange c64-native-exchange
test-native-exchange:
	$(CC) -O2 $(NATIVE_EXCHANGE_FLAGS) $(NATIVE_EXCHANGE_CORE) $(NATIVE_EXCHANGE_TEST) tests/test_native_exchange.c -o build/test-native-exchange
	./build/test-native-exchange
sanitize-native-exchange:
	$(CC) -O1 $(NATIVE_EXCHANGE_FLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -no-pie $(NATIVE_EXCHANGE_CORE) $(NATIVE_EXCHANGE_TEST) tests/test_native_exchange.c -o build/test-native-exchange-san
	ASAN_OPTIONS=detect_leaks=0 ./build/test-native-exchange-san
differential-native-exchange:
	python3 tools/lm_interpreter_exchange_differential.py
	python3 tools/lm_interpreter_exchange_differential.py --sanitize
c64-native-exchange:
	python3 tools/native_scheduler_overlay_build.py --cc $(MOS_CC)
	python3 tools/lm_interpreter_exchange_target_build.py --cc $(MOS_CC) --c1541 $(C1541)
	python3 tools/lm_interpreter_exchange_vice_test.py --vice $(VICE) --rom-dir $(ROM_DIR) --data-dir $(VICE_DATA_DIR)
