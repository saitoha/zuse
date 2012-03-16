#!/bin/sh

glibtoolize
aclocal
autoheader
automake -a
autoconf

