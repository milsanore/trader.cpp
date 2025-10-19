#############################################
# milss/tradercpp:1.0
# Build tradercpp
#############################################

FROM milss/tradercppbuild:v1.6 AS build_container

WORKDIR /tradercpp

COPY src/ ./src/
COPY tests/ ./tests/
COPY CMakeLists.txt .
COPY CMakePresets.json .
COPY conanfile.txt .
COPY Makefile .

RUN make init
RUN make build-debug
RUN make test


COPY binance/fixconfig .
COPY binance/spot-fix-md.xml .
COPY binance/stunnel_prod.conf .
# COPY binance/keys/prod-key-prv.pem .

FROM alpine
WORKDIR /tradercpp

COPY --from=build_container /tradercpp/fixconfig            /tradercpp/binance/fixconfig
COPY --from=build_container /tradercpp/spot-fix-md.xml      /tradercpp/binance/spot-fix-md.xml
COPY --from=build_container /tradercpp/stunnel_prod.conf    /tradercpp/binance/stunnel_prod.conf
# COPY --from=build_container /tradercpp/prod-key-prv.pem     /tradercpp/binance/keys/prod-key-prv.pem

COPY --from=build_container /tradercpp/build/Debug/tradercpp /tradercpp/tradercpp
ENTRYPOINT ["/tradercpp/tradercpp"]
