# trader.cpp
a proof-of-concept, showcasing some c++ coding combined with some fintech concepts

<img src="https://static.wikia.nocookie.net/surrealmemes/images/8/80/2f0.png"
	alt="stonks"
	width="250" />

## REQUIREMENTS
- C++20
- Conan
- CMake
- a local proxy (e.g. stunnel) for TLS encryption

## BUILD AND RUN
(NB: this app uses `make` as a task runner, but it's not essential)
1. `make init`
2. `make build-debug`
3. `make withenv RECIPE=run-debug`

## HELP
`make`


# AIM

## FUNCTIONAL
- ✅ create a FIX connection to Binance
- ✅ subscribe to price updates
- create a basic trading signal (e.g. standard deviations)
- fire an order
- test in the Binance test enviroment


## NON-FUNCTIONAL
- ✅ basic cpp app to start with
- ✅ makefile and build chain
- ✅ package management
- ✅ debugging
- UI can come later (perhaps explore curses)
- logging
    - fast
    - structured
    - basic schema (severity, correlationId)
- dependency injection
- single-threaded to start with, then re-architect (and mermaid diagram)
- demo video (https://asciinema.org)
- nix virtual environment
- decimal type
- sparse arrays
- release binaries on github
- ccache.dev
- zeromq + protobufs?
- interrupt/ctrl+c signal

# STANDARDS
- high unit-test coverage + badge
- code formatting / auto-formatter
  - clang?
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
- conventional commits
- automated semantic versioning

# CREDITS
- https://github.com/binance/binance-fix-connector-python