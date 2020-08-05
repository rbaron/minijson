FROM alpine:3.12

RUN apk add --no-cache bash git make cmake g++ clang
ENV CXX=/usr/bin/clang++
ENV CC=/usr/bin/clang
WORKDIR /app
ADD . .
RUN rm -rf build/*

# For building with local changes, mount the volumes at runtime:
# $ docker run -it -v $(pwd)/minijson.h:/app/minijson.h -v $(pwd)/tests:/app/tests minijson
# And run with
# $ cd /app/build/ && cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ .. && cmake --build .
# $ ./tests/parser_tests