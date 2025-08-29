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

## build: 🔨 compile the app
.PHONY: build
build:
	rm -rf build && mkdir -p build
	source .venv/bin/activate && \
		conan install . --output-folder=build --build=missing --conf tools.cmake.cmaketoolchain:generator=False && \
		cmake --preset default && \
		cmake --build --preset default

## run: 💨 run the app
.PHONY: run
run:
	build/tradercpp
