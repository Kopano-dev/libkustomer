/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

import "C"
import (
	"fmt"

	kustomer "stash.kopano.io/kc/libkustomer"
)

func asKnownErrorOrUnknown(err error) C.ulonglong {
	switch e := err.(type) {
	case kustomer.ErrNumeric:
		return C.ulonglong(e)
	default:
		if debug {
			fmt.Printf("kustomer-c unknown error: %s\n", err)
		}
		return C.ulonglong(kustomer.ErrStatusUnknown)
	}
}

func asErrNumeric(errNum C.ulonglong) kustomer.ErrNumeric {
	if errNum == 0 {
		return 0
	}
	return kustomer.ErrNumeric(errNum)
}
