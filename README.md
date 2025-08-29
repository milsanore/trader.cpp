# trader.cpp
a proof-of-concept, showcasing some c++ coding combined with some fintech concepts

<img src="https://static.wikia.nocookie.net/surrealmemes/images/8/80/2f0.png"
	alt="stonks"
	width="250" />

## REQUIREMENTS
- C++
- Make
- CMake
- Conan

## BUILD
NB: this app uses `make` as a task runner

`make build`

## RUN
`make run`

## HELP
`make`


# AIM
- ✅ basic cpp app to start with
	- UI can come later (perhaps explore curses)
- ✅ makefile and build chain
- create a FIX connection to Binance
- subscribe to price updates
- create a basic trading signal (e.g. standard deviations)
- fire an order
- test in the Binance test enviroment
- single-threaded to start with, then re-architect (and mermaid diagram)

# STANDARDS
- high unit-test coverage + badge
- code formatting
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