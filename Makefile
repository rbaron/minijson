SHELL=/bin/bash

SOURCE_FILES := $(shell find minijson.h tests examples -name *.h -o -name *.cc)

.PHONY: format lint build

format:
	clang-format -i $(SOURCE_FILES)

lint:
	clang-format $(SOURCE_FILES) | diff <(cat $(SOURCE_FILES)) -

build:
	cmake -Bbuild/ .
	cmake --build build/

unittests: build
	./build/tests/tokenizer_tests && \
	./build/tests/jsonnode_tests && \
	./build/tests/parser_tests