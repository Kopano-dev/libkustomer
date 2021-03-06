/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

/*
typedef void (*kustomer_cb_func_log_s) (char*);

void bridge_kustomer_log_cb_func_log_s(kustomer_cb_func_log_s f, char* s);
*/
import "C"
import (
	"fmt"
	"strings"

	kustomer "stash.kopano.io/kc/libkustomer"
)

type callbackLogger struct {
	cb C.kustomer_cb_func_log_s
}

func (logger *callbackLogger) Printf(format string, a ...interface{}) {
	s := strings.TrimRight(fmt.Sprintf(format, a...), "\n")
	C.bridge_kustomer_log_cb_func_log_s(logger.cb, C.CString(s))
}

func getCLogger(cb C.kustomer_cb_func_log_s) kustomer.Logger {
	return &callbackLogger{
		cb: cb,
	}
}
