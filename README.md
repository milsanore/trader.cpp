[![Build](https://github.com/milsanore/trader.cpp/actions/workflows/build.yml/badge.svg)](https://github.com/milsanore/trader.cpp/actions/workflows/build.yml)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=milsanore_trader.cpp&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=milsanore_trader.cpp)
[![codecov](https://codecov.io/github/milsanore/trader.cpp/graph/badge.svg?token=C787ZTXBQC)](https://codecov.io/github/milsanore/trader.cpp)

<img src="https://static.wikia.nocookie.net/surrealmemes/images/8/80/2f0.png"
	alt="stonks"
	width="250" />

# trader.cpp
a proof-of-concept, showcasing some c++ coding combined with some fintech concepts

## Run Requirements
- a Binance account, with an Ed25519 token that has FIX read permissions enabled 
- `stunnel` for TLS encryption (or a local proxy)

## Build Requirements
- C++20
- `Conan` (and a conan profile)
- `CMake`
- `ninja`

## Dev Requirements
- `make` (convenience)
- `clang-tidy`
- `clang-format`
- `lcov`

## Build and Run
(NB: this app uses `make` as a task runner, but it's not essential)
1. copy `.env.example` to `.env`, and set your public/private keys
2. run an SSL tunnel (e.g. `stunnel binance/stunnel_prod.conf`)
3. `make init`
4. `make build-debug`
5. `make withenv RECIPE=run-debug`

## Test
`make test`

## Debug
- vscode
  - app and test debug profiles are pre-configured in the following files:
    - `.vscode/launch.json`
    - `.vscode/tasks.json`
- intellij (clion)
  - enable the `debug` CMake profile

## Help
`make`

## Cloud Config
- Sonarcloud (click the badge)
- Codecov (click the badge)


# Aims

## Functional
- ✅ create a FIX connection to Binance
- ✅ subscribe to price updates
- create a basic trading signal (e.g. standard deviations)
- fire an order
- test in the Binance test environment

## Non-functional
- ✅ basic cpp app to start with
- ✅ makefile and build chain
- ✅ package management
- ✅ debugging
- ✅ single-threaded to start with, then re-architect (and mermaid diagram)
- ✅ UI
  - ✅ publish messages to thread-safe queue
  - ✅ consume messages from thread-safe queue on a worker thread
  - interrupt/ctrl+c signal
  - 60fps limit
- ✅ logging
    - ✅ fast
    - structured
    - basic schema (severity, correlationId)
- code quality
  - ✅ clang-format
    - ✅ configure editor to auto-format
    - ✅ fail commits if not formatted
    - ✅ fail builds if not formatted
  - clang-tidy
    - ✅ all files tidied
    - ✅ configured clang-tidy => clang-format
    - ✅ fail commits/merges if not tidy
    - ✅ fail builds if not tidy
    - ✅ `clang-tidy-diff.py` (alias 18)
  - ✅ git hooks
  - ✅ integrated into build pipeline
  - ✅ badges
  - ✅ sonarcloud integrated into build pipeline
  - sonarcloud coverage
- pipeline
  - ✅ custom docker build image with all dependencies (hosted on GHCR for faster pipelines)
  - cron
    - comprehensive clang-tidy & clang-format checks
    - sonarcloud
  - nix
  - ccache (faster pipelines)
  - github releases
  - local github action runner (`act`)
  - containerised integration tests / dind
- testing
  - ✅ dependency injection
  - integration test with mocked Binance
- performance
  - profiling (valgrind/cachegrind)
  - load test with mocked Binance server (k6?)
  - sparse arrays
  - memory-mapped files
- observability
  - opentelemetry
  - grafana+tempo via docker-compose
- versioning
  - master branch merge check for conventional commit message (e.g. regex)
    - maybe a merge git gook check
  - automated semantic versioning
  - github-changelog-generator
- other
  - decimal type
  - zeromq + protobufs?
  - shellcheck?
  - conan build_requires
- deployment
  - terraform

# Credits
- https://github.com/binance/binance-fix-connector-python
