FROM alpine:3.12

RUN apk add --no-cache git make clang bash
WORKDIR /app
ADD . .
RUN rm -rf build/*