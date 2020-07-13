/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package kustomer

type Logger interface {
	Printf(string, ...interface{})
}

type nullLogger struct{}

func (l *nullLogger) Printf(format string, a ...interface{}) {
}

// DefaultLogger is the packageLogger used by this library if no other logger
// is explicitly specified.
var DefaultLogger Logger = &nullLogger{}
