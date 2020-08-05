FROM alpine:3.12

RUN apk add --no-cache git make clang bash
WORKDIR /app
ADD . .
ADD entrypoint.sh /entrypoint.sh
RUN rm -rf build/*
ENTRYPOINT ["/entrypoint.sh"]