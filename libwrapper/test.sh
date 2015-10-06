#!/bin/sh -xvv
WRAPPER=`pwd`/wrapper.so
export SUDO_ALLOWED="foobar:blah:xyzzy"
env LD_PRELOAD=${WRAPPER} ./test
