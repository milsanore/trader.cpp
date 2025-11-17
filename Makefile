#!make
SHELL:=/bin/bash

# ###############################################
# A set of execution recipes (all PHONY).
# In essence, a programmatic entrypoint into the app.
# Run `make` for help.
# ###############################################

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

## withenv: 😭 execute `make` with environment variables defined in `.env`. like so > `make withenv RECIPE=init`
.PHONY: withenv
withenv:
	test -e .env || cp .env.example .env
	bash -c 'set -o allexport; source .env; set +o allexport; make "$$RECIPE"'

## init: 🏌️ initialize the project
.PHONY: init
init:
	$(call pp,initializing project)
	git config core.hooksPath hooks
	rm -rf build && mkdir build
	python3 -m venv .venv
	source .venv/bin/activate && \
		pip install gcovr conan && \
		conan install . --lockfile=conan.lock --build=missing -s build_type=Debug && \
		conan install . --lockfile=conan.lock --build=missing -s build_type=Release
	cmake --preset=release
	cmake --preset=debug
	$(MAKE) restore-cpus

## lock: 📦 update Conan dependency lock file. (NB run after adding conan dependencies to conanfile.txt, but before installing)
.PHONY: lock
lock:
	$(call pp,updating conan lock file to match contents of conanfile.txt)
	source .venv/bin/activate && \
		conan lock create . --profile:host=default -s build_type=Debug   --lockfile-out=conan.lock && \
		conan lock create . --profile:host=default -s build_type=Release --lockfile=conan.lock --lockfile-out=conan.lock

## build-debug: 🔨 compile (debug)
.PHONY: build-debug
build-debug:
	$(call pp,NB assuming `make init` has been called)
	cmake --preset=debug
	cmake --build --preset=debug

## build-release: 🏎️ compile (prod)
.PHONY: build-release
build-release:
	cmake --preset=release
	cmake --build --preset=release

## test: 🧪 run unit tests
.PHONY: test
test:
	cmake --preset debug
	cmake --build --preset=debug
	ctest -j$(shell nproc) --preset=debug -LE BENCHMARK
# 	coverage
	lcov --gcov-tool gcov --capture --directory . --output-file lcov.info
	source .venv/bin/activate && \
		gcovr -r . --exclude 'tests/*' --sonarqube -o sonar-coverage.xml

## bench: ⏱️ build and run benchmarks
.PHONY: bench
bench:
	cmake --build --preset release
	build/Release/benchmarks/benchmarks \
	--benchmark_out=bench_results.json \
  	--benchmark_out_format=json \
	--benchmark_report_aggregates_only=false

## tidy: 🧹 tidy things up before committing code
.PHONY: tidy
tidy:
	find src/ tests/ benchmarks/ \( -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' \) -exec clang-format -i {} +	
	hooks/check_shell.sh
	hooks/check_clang_tidy.sh

## run-debug: 🏃‍♂️ run the app (debug) (don't forget `withenv`)
.PHONY: run-debug
run-debug:
	ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1:fast_unwind_on_malloc=0 \
	build/Debug/tradercpp

## run-release: 🏎️ run the app (prod)
.PHONY: run-release
run-release:
	$(call pp,moving IRQs)
	set -o allexport; source .env; set +o allexport; sudo -E scripts/pin_irqs.sh
	$(call pp,moving CPUs and starting app)
	set -o allexport; source .env; set +o allexport; sudo -E scripts/pin_cpus.sh build/Release/tradercpp

## restore-cpus: 🖥️ hand back pinned CPUs and IRQs to the operating system. (NB app must not be running) 
.PHONY: restore-cpus
restore-cpus:
	$(call pp,handing CPU management back to the kernel (NB: app must not be running))
	$(call pp,NB: for re-assigning IRQs reboot the machine)
	set -o allexport; source .env; set +o allexport; sudo -E scripts/unpin_cpus.sh

# CONTAINERISATION RECIPES ----------------------------------------------------

## build-container: 🚢 create the docker container for building the app (hosted on dockerhub and ghcr)
.PHONY: build-container
build-container:
	IMAGE_VERSION=
	@if [ -z "$(IMAGE_VERSION)" ]; then \
		echo "Error: IMAGE_VERSION is not set"; \
		echo "(you can set it on the command line like so: \`make build-container IMAGE_VERSION=1.8\`)"; \
		exit 1; \
	fi
	docker build -f Dockerfile_build -t milss/tradercppbuild:latest -t milss/tradercppbuild:v$(IMAGE_VERSION) .

## docker: 🚢 create an app docker image
.PHONY: docker
docker:
	docker build .
