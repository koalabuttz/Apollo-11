PRODUCT_CORE = src/agc_word.c src/agc_multiword.c src/guidance_product.c
PRODUCT_INPUT = tests/prepare_cases.c tests/vector_cases.c tests/product_cases.c
PRODUCT_TEST = $(PRODUCT_INPUT) tests/product_check.c
PRODUCT_HEADERS = $(PREPARE_HEADERS) include/guidance_product.h tests/product_cases.h tests/product_products.h
PRODUCT_DEFINITIONS = -DPREPARE_INPUT_ONLY -DVECTOR_INPUT_ONLY
all: build/product-snapshot build/test-product
build/product-snapshot: $(PRODUCT_CORE) $(PRODUCT_INPUT) platform/host/product_snapshot.c $(PRODUCT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PRODUCT_DEFINITIONS) $(PRODUCT_CORE) $(PRODUCT_INPUT) platform/host/product_snapshot.c -o $@
build/test-product: $(PRODUCT_CORE) $(PRODUCT_TEST) tests/test_product.c $(PRODUCT_HEADERS) | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PRODUCT_DEFINITIONS) $(PRODUCT_CORE) $(PRODUCT_TEST) tests/test_product.c -o $@
test: test-product
.PHONY: test-product sanitize-product c64-product
test-product: build/test-product
	./build/test-product
sanitize: sanitize-product
sanitize-product: | build
	$(CC) $(CPPFLAGS) $(PRODUCT_DEFINITIONS) -std=c99 -g -O1 -Wall -Wextra -Werror -fsanitize=address,undefined $(PRODUCT_CORE) $(PRODUCT_TEST) tests/test_product.c -o build/test-product-sanitize
	./build/test-product-sanitize
c64-product: build/product.prg
build/product.prg: $(PRODUCT_CORE) $(PRODUCT_TEST) platform/c64/product_diagnostic.c $(PRODUCT_HEADERS) | build
	$(MOS_CC) $(CPPFLAGS) $(PRODUCT_DEFINITIONS) -std=c99 -Os -fno-static-stack -mlto-zp=0 -Wall -Wextra -Werror -Wl,-Map,build/product.map $(PRODUCT_CORE) $(PRODUCT_TEST) platform/c64/product_diagnostic.c -o $@
