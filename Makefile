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

## withenv: 😭 run `make` with envars from `.env`. like so `make withenv RECIPE=init`
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
		pip install gcovr conan && \
		conan install . --lockfile=conan.lock --build=missing -s build_type=Debug
	cmake --preset=debug

## lock-conan: 📦 run after installing conan dependencies
.PHONY: lock-conan
lock-conan:
	conan lock create . --profile:host=default -s build_type=Debug --lockfile-out=conan.lock
	conan lock create . --profile:host=default -s build_type=Release --lockfile=conan.lock --lockfile-out=conan.lock

## build-debug: 🔨 compile (debug)
.PHONY: build-debug
build-debug:
	$(call pp,assuming `make init` has been called)
	cmake --build --preset=debug

## build-release: 🏎️ compile (prod)
.PHONY: build-release
build-release:
	source .venv/bin/activate && \
		conan install . --lockfile=conan.lock --build=missing -s build_type=Release
	cmake --preset=release
	cmake --build --preset=release

## test: 🧪 run google-test
.PHONY: test
test:
	cmake --preset debug
	cmake --build --preset=debug
	ctest --preset=debug
	lcov --gcov-tool gcov --capture --directory . --output-file lcov.info
	source .venv/bin/activate && \
		gcovr -r . --exclude 'tests/*' --sonarqube -o sonar-coverage.xml

## tidy: 🧹 tidy things up before committing code
.PHONY: tidy
tidy:
	find src/ tests/ \( -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' \) -exec clang-format -i {} +	
	find tests/ -name '*.cpp' | xargs clang-tidy -p build/Debug --fix --format-style=.clang-format
	find src/   -name '*.cpp' | xargs clang-tidy -p build/Debug --fix --format-style=.clang-format

## run-debug: 🏃‍♂️  run the app (debug) (don't forget `withenv`)
.PHONY: run-debug
run-debug:
	ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1:fast_unwind_on_malloc=0 \
	build/Debug/tradercpp

## run-release: 🏎️  run the app (prod)
.PHONY: run-release
run-release:
	build/Release/tradercpp
