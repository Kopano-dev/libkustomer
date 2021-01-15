/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2021 Kopano and its licensors
 */

package libkustomer

import (
	kustomer "stash.kopano.io/kc/libkustomer"
)

// InitOptions defines the options for Init.
type InitOptions struct {
	Debug            bool
	AutoRefresh      bool
	ProductUserAgent *string

	DefaultDebugLogger kustomer.Logger
}
