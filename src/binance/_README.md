Classes that faciliate connectivity to the Binance crypto exchange.
Uses the FIX protocol and the c++ "QuickFIX" library.
Pushes updates to two concurrent queues, that downstream consumers can use:
1. order updates (prices/volumes)
2. trade updates
NB: Binance uses a custom authentication mechanism.
