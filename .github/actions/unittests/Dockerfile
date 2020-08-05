FROM alpine:3.12

RUN apk add --no-cache bash git make cmake g++ clang
ENV CXX=/usr/bin/clang++
ENV CC=/usr/bin/clang
WORKDIR /app
ADD . .
ADD entrypoint.sh /entrypoint.sh
RUN rm -rf build/*
ENTRYPOINT ["/entrypoint.sh"]