# trader.cpp
a proof-of-concept, showcasing some c++ coding combined with some fintech concepts

<img src="https://static.wikia.nocookie.net/surrealmemes/images/8/80/2f0.png"
	alt="stonks"
	width="250" />

## REQUIREMENTS
- a Binance account, with an Ed25519 token that has FIX read permissions enabled 
- C++20
- `Conan` (and a conan profile)
- `CMake`
- `stunnel` or a local proxy, for TLS encryption

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
- nix virtual environment
- decimal type
- sparse arrays
- release binaries on github
- ccache.dev
- zeromq + protobufs?
- valgrind/cachegrind
- code formatting / auto-formatter
  - ✅ clang-format
  - ✅ git hooks
  - ✅integrated into build pipeline
- clang-tidy
- github releases
- custom docker build image with all dependencies

# STANDARDS
- high unit-test coverage + badge
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
- memory-mapped files

# CREDITS
- https://github.com/binance/binance-fix-connector-python
