#!/bin/bash -ex
#
# Copyright 2017, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#     * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# run-coverage.sh - is called inside a Docker container;
#                   builds syscall_intercept with the aim of collecting
#                   information on code coverage

cd $WORKDIR

mkdir build
cd build
CC=gcc cmake ..  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DCMAKE_C_FLAGS=-coverage \
		-DCMAKE_CXX_FLAGS=-coverage \
		-DEXPECT_SPURIOUS_SYSCALLS=ON

make
ctest --output-on-failure
bash <(curl -s https://codecov.io/bash) -c -F regular_tests
find . -name ".coverage" -exec rm {} \;
find . -name "coverage.xml" -exec rm {} \;
find . -name "*.gcov" -exec rm {} \;
find . -name "*.gcda" -exec rm {} \;

pushd ~/pmemfile/build
LD_LIBRARY_PATH=$WORKDIR/build ctest --output-on-failure -R preload_
popd
bash <(curl -s https://codecov.io/bash) -c -F pmemfile_tests

cd ..
rm -r build
