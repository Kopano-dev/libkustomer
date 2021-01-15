/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

import (
	"stash.kopano.io/kc/libkustomer/lib/kustomer/libkustomer"
)

var (
	debug bool
)

func init() { //nolint:gochecknoinits // This library uses init to set up env.
	debug = libkustomer.Init(nil)
}

func main() {}
