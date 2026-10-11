EXEC_SERVICE_CORE = src/agc_word.c src/executive.c
EXEC_SERVICE_INPUT = tests/scenarios.c tests/executive_service_cases.c
EXEC_SERVICE_TEST = $(EXEC_SERVICE_INPUT) tests/state_digest.c tests/executive_service_check.c
EXEC_SERVICE_HEADERS = $(HEADERS) tests/executive_service_cases.h tests/executive_service_check.h tests/executive_service_vectors.h
all: build/test-executive-services build/executive-service-snapshot
build/executive-service-snapshot: $(EXEC_SERVICE_CORE) $(EXEC_SERVICE_INPUT) platform/host/executive_service_snapshot.c $(EXEC_SERVICE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXEC_SERVICE_CORE) $(EXEC_SERVICE_INPUT) platform/host/executive_service_snapshot.c -o $@
build/test-executive-services: $(EXEC_SERVICE_CORE) $(EXEC_SERVICE_TEST) tests/test_executive_services.c $(EXEC_SERVICE_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(EXEC_SERVICE_CORE) $(EXEC_SERVICE_TEST) tests/test_executive_services.c -o $@
test: test-executive-services
.PHONY: test-executive-services sanitize-executive-services c64-executive-services
test-executive-services: build/test-executive-services
	./build/test-executive-services
sanitize: sanitize-executive-services
sanitize-executive-services: | build
	$(CC) $(CPPFLAGS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(EXEC_SERVICE_CORE) $(EXEC_SERVICE_TEST) tests/test_executive_services.c -o build/test-executive-services-sanitize
	./build/test-executive-services-sanitize
c64-executive-services: build/executive-services.prg
build/executive-services.prg: $(EXEC_SERVICE_CORE) $(EXEC_SERVICE_TEST) platform/c64/executive_service_diagnostic.c $(EXEC_SERVICE_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/executive-services.map $(EXEC_SERVICE_CORE) $(EXEC_SERVICE_TEST) platform/c64/executive_service_diagnostic.c -o $@
