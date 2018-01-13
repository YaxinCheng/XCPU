#!/bin/sh

if [ "$1" = "-u" ]; then
	./xsim 100000 examples/arithmetic.xo debug
else
	./xsim 100000 examples/arithmetic.xo > compare/normal
fi
