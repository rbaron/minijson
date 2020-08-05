#!/bin/bash
set -euox pipefail

make build
make unittests