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
	test -e .git/hooks/pre-commit || ln -s ../../.githooks/pre-commit .git/hooks/pre-commit
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

## test: 🧪 run google-test
.PHONY: test
test:
	cmake --build --preset=debug
	ctest --preset=debug
	lcov --gcov-tool gcov --capture --directory . --output-file lcov.info

## build-release: 🔨🔨 compile (prod)
.PHONY: build-release
build-release:
	source .venv/bin/activate && \
		conan install . --build=missing -s build_type=Release
	cmake --preset=release
	cmake --build --preset=release

## run-debug: 🏃‍♂️ run the app (debug) (don't forget `withenv`)
.PHONY: run-debug
run-debug:
	build/Debug/tradercpp

## run-release: 🏎️ run the app (prod)
.PHONY: run-release
run-release:
	build/Release/tradercpp
