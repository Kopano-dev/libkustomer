/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package kustomer

import (
	"fmt"
)

// ErrStatus is the Error type as used by kustomer.
type ErrStatus uint64

func (errStatus ErrStatus) Error() string {
	return fmt.Sprintf("%s (:0x%x)", ErrStatusText(errStatus), uint64(errStatus))
}

// ErrStatusors as defined by this library.
const (
	ErrStatusNone              = iota
	ErrStatusUnknown ErrStatus = (1 << 8) | iota
	ErrStatusInvalidProductName
	ErrStatusAlreadyInitialized
	ErrStatusNotInitialized
	ErrStatusTimeout
)

// StatusSuccess is the success response as returned by this library.
const StatusSuccess = ErrStatusNone

// ErrStatusTextMap maps ErrStatusos to readable names.
var ErrStatusTextMap = map[ErrStatus]string{
	ErrStatusUnknown:            "Unknown",
	ErrStatusInvalidProductName: "Invalid Product Name Value",
	ErrStatusAlreadyInitialized: "Already Initialized",
	ErrStatusNotInitialized:     "Not Initialized",
	ErrStatusTimeout:            "Timeout",
}

// ErrStatusText returns a text for the ErrStatus. It returns the empty string
// if the code is unknown.
func ErrStatusText(code ErrStatus) string {
	text := ErrStatusTextMap[code]
	return text
}
