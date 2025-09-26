[![Build](https://github.com/milsanore/trader.cpp/actions/workflows/build.yml/badge.svg)](https://github.com/milsanore/trader.cpp/actions/workflows/build.yml)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=milsanore_trader.cpp&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=milsanore_trader.cpp)
[![codecov](https://codecov.io/github/milsanore/trader.cpp/graph/badge.svg?token=C787ZTXBQC)](https://codecov.io/github/milsanore/trader.cpp)

<img src="https://static.wikia.nocookie.net/surrealmemes/images/8/80/2f0.png"
	alt="stonks"
	width="250" />

# trader.cpp
a proof-of-concept, showcasing some c++ coding combined with some fintech concepts

## RUN REQUIREMENTS
- a Binance account, with an Ed25519 token that has FIX read permissions enabled 
- `stunnel` or a local proxy, for TLS encryption

## BUILD REQUIREMENTS
- C++20
- `Conan` (and a conan profile)
- `CMake`

## DEV REQUIREMENTS
- `clang-tidy`
- `clang-format`
- `lcov`

## BUILD AND RUN
(NB: this app uses `make` as a task runner, but it's not essential)
1. copy `.env.example` to `.env`, and set your public/private keys
2. run an SSL tunnel (e.g. `stunnel binance/stunnel_prod.conf`)
3. `make init`
4. `make build-debug`
5. `make withenv RECIPE=run-debug`

## TEST
`make test`

## HELP
`make`


# AIM

## FUNCTIONAL
- ✅ create a FIX connection to Binance
- ✅ subscribe to price updates
- create a basic trading signal (e.g. standard deviations)
- fire an order
- test in the Binance test environment


## NON-FUNCTIONAL
- ✅ basic cpp app to start with
- ✅ makefile and build chain
- ✅ package management
- ✅ debugging
- ✅ UI
  - ✅ publish messages to thread-safe queue
  - ✅ consume messages from thread-safe queue on a worker thread
  - interrupt/ctrl+c signal
  - 60fps limit
- ✅ logging
    - ✅ fast
    - structured
    - basic schema (severity, correlationId)
- ✅ dependency injection
- ✅ single-threaded to start with, then re-architect (and mermaid diagram)
- ✅ code formatting / clang-format
  - ✅ git hooks
  - ✅ integrated into build pipeline
  - ✅ coverage badge
- ✅ custom docker build image with all dependencies (for faster pipelines)
- ✅ static code analysis
  - ✅ sonarcloud integrated into build pipeline
- clang-tidy
- decimal type
- sparse arrays
- release binaries on github
- ccache.dev
- zeromq + protobufs?
- valgrind/cachegrind
- github releases
- local github action runner
- sonarqube integration
- nix virtual environment

# STANDARDS
- high unit-test coverage
- static code analysis
- configure debugging 
- git use
- integration test with mocked Binance
- load test with mocked Binance (k6?)
- profiling
- github action
    - containerised integration tests / dind
- observability
    - opentelemetry
    - grafana+tempo via docker-compose
- versioning
  - conventional commits
  - automated semantic versioning
  - github-changelog-generator
  - master branch merge check for conventional commit message
  - maybe a git gook check for merges
- memory-mapped files

# CREDITS
- https://github.com/binance/binance-fix-connector-python
