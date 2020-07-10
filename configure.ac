AC_PREREQ(2.60)
AC_INIT([libkustomer], [0.0.1], [development@kopano.io])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
if ! which "${CC}" >/dev/null ; then
    AC_MSG_ERROR([Please install a C compiler before trying to build libkustomer])
fi
AC_PROG_CXX

AC_PATH_PROG([GO],[go])
if ! which "${GO}" >/dev/null ; then
    AC_MSG_ERROR([Please install Go (https://golang.org) before trying to build libkustomer])
fi

GO_VERSION_MIN=1.14
GO_VERSION=$(${GO} version | sed 's/^go version go//' | sed 's/ .*//')
AX_COMPARE_VERSION([$GO_VERSION], [ge], [$GO_VERSION_MIN],
    AC_MSG_NOTICE([Go ${GO_VERSION} found]),
    AC_MSG_ERROR([Please install Go ${GO_VERSION_MIN} or newer before trying to build libkustomer (found Go ${GO_VERSION}).]))

AC_PATH_PROG([GOFMT],[gofmt])
AC_PATH_PROG([GOLINT],[golangci-lint])
AC_PATH_PROG([GO2XUNIT],[go2xunit])

AC_PATH_PROGS([PYTHON],[python3 python python2],[python])

AC_OUTPUT(Makefile)
