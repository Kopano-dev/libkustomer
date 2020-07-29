/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package kustomer

// A Config holds the configuration for this module.
type Config struct {
	Logger Logger

	Debug       bool
	AutoRefresh bool

	ProductUserAgent *string
}
