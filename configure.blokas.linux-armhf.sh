#!/bin/sh
QTDIR="$(dirname "$(readlink -f "$0")")"

# Share the desktop backends and feature selection with the other Linux builds.
# Cross builds pass -xplatform linux-armv6-gnueabihf-g++, -sysroot, and -extprefix.
exec "$QTDIR/configure.blokas.linux.sh" -no-feature-neon "$@"
