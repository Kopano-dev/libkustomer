/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

/*
#define KUSTOMER_API 1
#define KUSTOMER_API_MINOR 0

#define KUSTOMER_VERSION (KUSTOMER_API * 10000 + KUSTOMER_API_MINOR * 100)

#include "kustomer_callbacks.h"
*/
import "C" //nolint

import (
	"context"
	"encoding/json"
	"time"
	"unsafe" //nolint

	"github.com/mattn/go-pointer"

	"stash.kopano.io/kc/libkustomer"
)

//export kustomer_set_autorefresh
func kustomer_set_autorefresh(flagCInt C.int) C.ulonglong {
	var flag bool
	if flagCInt != 0 {
		flag = true
	}
	err := SetAutoRefresh(flag)
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}

//export kustomer_set_logger
func kustomer_set_logger(cb C.kustomer_cb_func_log_s, debug C.int) C.ulonglong {
	logger := getCLogger(cb)
	var flag *bool
	if debug >= 0 {
		var f bool
		if debug != 0 {
			f = true
		}
		flag = &f
	}
	err := SetLogger(logger, flag)
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}

//export kustomer_initialize
func kustomer_initialize(productNameCString *C.char) C.ulonglong {
	var productName *string
	if productNameCString != nil {
		productNameString := C.GoString(productNameCString)
		productName = &productNameString
	}

	err := Initialize(context.Background(), productName)
	if err != nil {
		return asKnownErrorOrUnknown(err)
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

//export kustomer_set_notify_when_updated
func kustomer_set_notify_when_updated(updateCb C.kustomer_cb_func_watch, exitCb C.kustomer_cb_func_watch) C.ulonglong {
	err := SetNotifyWhenUpdated(func() {
		if updateCb != nil {
			C.bridge_kustomer_watch_cb_func_updated(updateCb)
		}
	}, func() {
		if exitCb != nil {
			C.bridge_kustomer_watch_cb_func_updated(exitCb)
		}
	})
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}
	return kustomer.StatusSuccess
}

//export kustomer_unset_notify_when_updated
func kustomer_unset_notify_when_updated() C.ulonglong {
	err := UnsetNotifyWhenUpdated()
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}
	return kustomer.StatusSuccess
}

//export kustomer_dump_claims
func kustomer_dump_claims() (C.ulonglong, *C.char) {
	claims, err := CurrentClaims()
	if err != nil {
		return asKnownErrorOrUnknown(err), nil
	}

	b, err := json.Marshal(claims.Dump())
	if err != nil {
		return asKnownErrorOrUnknown(err), nil
	}

	return kustomer.StatusSuccess, C.CString(string(b))
}

//export kustomer_err_numeric_text
func kustomer_err_numeric_text(errNum C.ulonglong) *C.char {
	err := asErrNumeric(errNum)
	return C.CString(ErrNumericText(err))
}

//export kustomer_begin_ensure
func kustomer_begin_ensure() (statusNum C.ulonglong, transactionPtr unsafe.Pointer) {
	kpc, err := CurrentKopanoProductClaims()
	if err != nil {
		return asKnownErrorOrUnknown(err), nil
	}

	transactionPtr = pointer.Save(kpc)

	return kustomer.StatusSuccess, transactionPtr
}

//export kustomer_end_ensure
func kustomer_end_ensure(transactionPtr unsafe.Pointer) C.ulonglong {
	v := pointer.Restore(transactionPtr)
	kpc, _ := v.(*kustomer.KopanoProductClaims)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}
	pointer.Unref(transactionPtr)

	return kustomer.StatusSuccess
}

func restoreKopanoProductClaimsFromPointer(transactionPtr unsafe.Pointer) *kustomer.KopanoProductClaims {
	v := pointer.Restore(transactionPtr)
	kpc, _ := v.(*kustomer.KopanoProductClaims)
	return kpc
}

//export kustomer_dump_ensure
func kustomer_dump_ensure(transactionPtr unsafe.Pointer) (statusNum C.ulonglong, jsonBytes *C.char) {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction), nil
	}

	m := kpc.Dump()
	b, err := json.Marshal(m)
	if err != nil {
		return asKnownErrorOrUnknown(err), nil
	}

	return kustomer.StatusSuccess, C.CString(string(b))
}

//export kustomer_ensure_set_must_be_online
func kustomer_ensure_set_must_be_online(transactionPtr unsafe.Pointer, flagCInt C.int) C.ulonglong {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}

	var flag bool
	if flagCInt != 0 {
		flag = true
	}
	kpc.SetMustBeOnline(flag)

	return kustomer.StatusSuccess
}

//export kustomer_ensure_set_allow_untrusted
func kustomer_ensure_set_allow_untrusted(transactionPtr unsafe.Pointer, flagCInt C.int) C.ulonglong {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}

	var flag bool
	if flagCInt != 0 {
		flag = true
	}
	kpc.SetAllowUntrusted(flag)

	return kustomer.StatusSuccess
}

//export kustomer_ensure_ok
func kustomer_ensure_ok(transactionPtr unsafe.Pointer, productNameCString *C.char) C.ulonglong {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}

	err := kpc.EnsureOK(C.GoString(productNameCString))
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}

//export kustomer_ensure_get_bool
func kustomer_ensure_get_bool(transactionPtr unsafe.Pointer, productNameCString, claimCString *C.char) (C.ulonglong, C.int) {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction), 0
	}

	value, err := kpc.GetBool(C.GoString(productNameCString), C.GoString(claimCString))
	if err != nil {
		return asKnownErrorOrUnknown(err), 0
	}

	var valueCInt C.int = 0
	if value {
		valueCInt = 1
	}
	return kustomer.StatusSuccess, valueCInt
}

//export kustomer_ensure_ensure_bool
func kustomer_ensure_ensure_bool(transactionPtr unsafe.Pointer, productNameCString, claimCString *C.char, valueCInt C.int) C.ulonglong {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}

	var value bool
	if valueCInt != 0 {
		value = true
	}
	err := kpc.EnsureBool(C.GoString(productNameCString), C.GoString(claimCString), value)
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}

//export kustomer_ensure_get_string
func kustomer_ensure_get_string(transactionPtr unsafe.Pointer, productNameCString, claimCString *C.char) (C.ulonglong, *C.char) {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction), nil
	}

	value, err := kpc.GetString(C.GoString(productNameCString), C.GoString(claimCString))
	if err != nil {
		return asKnownErrorOrUnknown(err), nil
	}

	return kustomer.StatusSuccess, C.CString(value)
}

//export kustomer_ensure_ensure_string
func kustomer_ensure_ensure_string(transactionPtr unsafe.Pointer, productNameCString, claimCString, valueCString *C.char) C.ulonglong {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}

	err := kpc.EnsureString(C.GoString(productNameCString), C.GoString(claimCString), C.GoString(valueCString))
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}

//export kustomer_ensure_get_int64
func kustomer_ensure_get_int64(transactionPtr unsafe.Pointer, productNameCString, claimCString *C.char) (C.ulonglong, C.longlong) {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction), 0
	}

	value, err := kpc.GetInt64(C.GoString(productNameCString), C.GoString(claimCString))
	if err != nil {
		return asKnownErrorOrUnknown(err), 0
	}

	return kustomer.StatusSuccess, C.longlong(value)
}

//export kustomer_ensure_ensure_int64
func kustomer_ensure_ensure_int64(transactionPtr unsafe.Pointer, productNameCString, claimCString *C.char, valueCLongLong C.longlong) C.ulonglong {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}

	err := kpc.EnsureInt64(C.GoString(productNameCString), C.GoString(claimCString), int64(valueCLongLong))
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}

var operatorCodeArray = []kustomer.OperatorType{
	kustomer.OperatorGreaterThan,
	kustomer.OperatorGreaterThanOrEqual,
	kustomer.OperatorLesserThan,
	kustomer.OperatorLesserThanOrEqual,
}

func getOperatorFromCode(opCode int) *kustomer.OperatorType {
	if opCode < 1 || opCode > len(operatorCodeArray) {
		return nil
	}

	op := operatorCodeArray[opCode]
	return &op
}

//export kustomer_ensure_ensure_int64_op
func kustomer_ensure_ensure_int64_op(transactionPtr unsafe.Pointer, productNameCString, claimCString *C.char, valueCLongLong C.longlong, opCode C.int) C.ulonglong {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}

	op := getOperatorFromCode(int(opCode))
	if op == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureUnknownOperator)
	}

	err := kpc.EnsureInt64WithOperator(C.GoString(productNameCString), C.GoString(claimCString), int64(valueCLongLong), *op)
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}

//export kustomer_ensure_get_float64
func kustomer_ensure_get_float64(transactionPtr unsafe.Pointer, productNameCString, claimCString *C.char) (C.ulonglong, C.double) {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction), 0
	}

	value, err := kpc.GetFloat64(C.GoString(productNameCString), C.GoString(claimCString))
	if err != nil {
		return asKnownErrorOrUnknown(err), 0
	}

	return kustomer.StatusSuccess, C.double(value)
}

//export kustomer_ensure_ensure_float64
func kustomer_ensure_ensure_float64(transactionPtr unsafe.Pointer, productNameCString, claimCString *C.char, valueCDouble C.double) C.ulonglong {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}

	err := kpc.EnsureFloat64(C.GoString(productNameCString), C.GoString(claimCString), float64(valueCDouble))
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}

//export kustomer_ensure_ensure_float64_op
func kustomer_ensure_ensure_float64_op(transactionPtr unsafe.Pointer, productNameCString, claimCString *C.char, valueCDouble C.double, opCode C.int) C.ulonglong {
	kpc := restoreKopanoProductClaimsFromPointer(transactionPtr)
	if kpc == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureInvalidTransaction)
	}

	op := getOperatorFromCode(int(opCode))
	if op == nil {
		return asKnownErrorOrUnknown(kustomer.ErrEnsureUnknownOperator)
	}

	err := kpc.EnsureFloat64WithOperator(C.GoString(productNameCString), C.GoString(claimCString), float64(valueCDouble), *op)
	if err != nil {
		return asKnownErrorOrUnknown(err)
	}

	return kustomer.StatusSuccess
}
