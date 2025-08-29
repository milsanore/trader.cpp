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

## withenv: 😭 `make` executes every line as a new shell. this is a workaround. `make withenv RECIPE=init`
.PHONY: withenv
withenv:
	test -e .env || cp .env.example .env
	bash -c 'set -o allexport; source .env; set +o allexport; make "$$RECIPE"'

## init: 🏎️ initialize the project
.PHONY: init
init:
	python3 -m venv .venv
	source .venv/bin/activate && pip install conan

## build-debug: 🔨 compile the app
.PHONY: build-debug
build-debug:
	rm -rf build && mkdir -p build
	source .venv/bin/activate && \
		conan install . --build=missing -s build_type=Debug && \
		cmake --preset debug && \
		cmake --build --preset debug

## build-release: 🔨 compile the app
.PHONY: build-release
build-release:
	rm -rf build && mkdir -p build
	source .venv/bin/activate && \
		conan install . --build=missing -s build_type=Release && \
		cmake --preset release && \
		cmake --build --preset release

## run-debug: 💨 run the app
.PHONY: run-debug
run-debug:
	build/Debug/tradercpp

## run-release: 💨 run the app
.PHONY: run-release
run-release:
	build/Release/tradercpp
