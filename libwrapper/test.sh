#!/bin/sh -xvv
WRAPPER=`pwd`/wrapper.so
export SUDO_ALLOWED="foo:bar:blah:xyzzy"
env LD_PRELOAD=${WRAPPER} ./test
