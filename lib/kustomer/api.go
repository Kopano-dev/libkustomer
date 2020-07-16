/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

/*
#define KUSTOMER_API 1
#define KUSTOMER_API_MINOR 0

#define KUSTOMER_VERSION (KUSTOMER_API * 10000 + KUSTOMER_API_MINOR * 100)

*/
import "C"

import (
	"context"
	"time"

	"stash.kopano.io/kc/libkustomer"
)

//export kustomer_initialize
func kustomer_initialize(productNameCString *C.char) C.ulonglong {
	err := Initialize(context.Background(), C.GoString(productNameCString))
	if err != nil {
		asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}

//export kustomer_uninitialize
func kustomer_uninitialize() C.ulonglong {
	err := Uninitialize()
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}
	return kustomer.StatusSuccess
}

//export kustomer_wait_until_ready
func kustomer_wait_until_ready(timeout C.ulonglong) C.ulonglong {
	err := WaitUntilReady(time.Duration(timeout) * time.Second)
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}
	return kustomer.StatusSuccess
}
