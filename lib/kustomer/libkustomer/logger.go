/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package libkustomer

import (
	"log"
	"os"

	kustomer "stash.kopano.io/kc/libkustomer"
)

func getSimpleLogger(prefix string) kustomer.Logger {
	return log.New(os.Stdout, prefix, 0)
}

var defaultLogger kustomer.Logger

func getDefaultDebugLogger() kustomer.Logger {
	if defaultLogger == nil {
		defaultLogger = getSimpleLogger("[kustomer-c debug] ")
	}

	return defaultLogger
}
