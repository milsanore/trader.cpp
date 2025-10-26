# NOTES
- namespaces map to the first level of subdirectories in src/

# SUBFOLDERS

## binance
- Classes that faciliate connectivity to the Binance crypto exchange using the FIX protocol and the c++ "QuickFIX" library
- Pushes updates to two concurrent queues, that downstream consumers can use:
    1. order updates (prices/volumes)
    2. trade updates
- NB: Binance uses a custom authentication mechanism
- NB: the `Config` class is important

## core
- core trading engine functionality
- e.g. the order book
- some coupling to the binance namespace (symbol, side, and multiplier values).
- TODO(mils): move out.

## ui
- a basic terminal ui written using the c++ `ftxui` library (similar to ncurses)
- work done predominantly on background threads

## utils
- helpers
- non-portable (OS-specific) stuff
