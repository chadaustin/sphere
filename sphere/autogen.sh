#!/bin/sh

# Regenerate configuration files
aclocal && \
automake --foreign && \
autoconf

exit
