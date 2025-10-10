#!make
SHELL:=/bin/bash

# pp - pretty print function
yellow := $(shell tput setaf 3)
normal := $(shell tput sgr0)
define pp
	@printf '$(yellow)$(1)$(normal)\n'
endef

.PHONY: help
help: Makefile
	@echo " Choose a command to run:"
	@sed -n 's/^##//p' $< | column -t -s ':' | sed -e 's/^/ /'

## withenv: 😭 `make` executes every line as a new shell. this is a workaround => `make withenv RECIPE=init`
.PHONY: withenv
withenv:
	test -e .env || cp .env.example .env
	bash -c 'set -o allexport; source .env; set +o allexport; make "$$RECIPE"'

## init: 🏌️ initialize the project, fetch dependencies
.PHONY: init
init:
	rm -rf build && mkdir build
	python3 -m venv .venv
	source .venv/bin/activate && \
		pip install conan && \
		conan install . --build=missing -s build_type=Debug
	cmake --preset=debug

## init-build-container: 🚢 create the docker container for building the app (hosted on dockerhub)
.PHONY: init-build-container
init-build-container:
	docker build -f Dockerfile_build -t milss/tradercppbuild:latest .

## build-debug: 🔨 compile (debug)
.PHONY: build-debug
build-debug:
	cmake --build --preset=debug

## build-diagnostic: 🩺 compile (diagnostic)
.PHONY: build-diagnostic
build-diagnostic:
	cmake --preset diagnostic
	cmake --build --preset=diagnostic

## build-release: 🏎️ compile (prod)
.PHONY: build-release
build-release:
	source .venv/bin/activate && \
		conan install . --build=missing -s build_type=Release
	cmake --preset=release
	cmake --build --preset=release

## test: 🧪 run google-test
.PHONY: test
test:
	cmake --preset debug
	cmake --build --preset=debug
	ctest --preset=debug
	lcov --gcov-tool gcov --capture --directory . --output-file lcov.info

## tidy: 🧹 tidy things up before committing code
.PHONY: tidy
tidy:
	find src/ tests/ \( -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' \) -exec clang-format -i {} +	
	find tests/ -name '*.cpp' | xargs clang-tidy -p build/Debug --fix --format-style=.clang-format
	find src/   -name '*.cpp' | xargs clang-tidy -p build/Debug --fix --format-style=.clang-format

## run-debug: 🏃‍♂️  run the app (debug) (don't forget `withenv`)
.PHONY: run-debug
run-debug:
	build/Debug/tradercpp

## run-diagnostic: 🩺 run the app (diagnostic) (don't forget `withenv`)
.PHONY: run-diagnostic
run-diagnostic:
	ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1:fast_unwind_on_malloc=0:suppressions=asan.supp \
	build/Diagnostic/tradercpp

## run-release: 🏎️  run the app (prod)
.PHONY: run-release
run-release:
	build/Release/tradercpp
