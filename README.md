# Kopano Kustomer Daemon client library

[![GoDoc](https://godoc.org/stash.kopano.io/kc/libkustomer?status.svg)](https://godoc.org/stash.kopano.io/kc/libkustomer)

This project implements a C shared library with a public API to talk to the API
provided by a locally running Kopano Kustomer Daemon.

Also this project can be used directly from Go as an importable module.

## Compiling

Make sure you have Go 1.14 or later installed. This project uses Go modules.

As this is a C library, it is furthermore assumed that there is a working C
compiler toolchain in your path which includes autoconf and make.

```
git clone <THIS-PROJECT> libkustomer
cd libkustomer
./bootstrap.sh
./configure
make
```

This will produce the compiled library `.so` and the matching C header file in
the `./.libs` directory.


### Use as Go module

```
import "stash.kopano.io/kc/libkustomer"
```
