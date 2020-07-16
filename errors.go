/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package kustomer

import (
	"fmt"
)

// ErrNumeric is the Error type as used by kustomer.
type ErrNumeric uint64

func (errStatus ErrNumeric) Error() string {
	return fmt.Sprintf("%s (:0x%x)", ErrStatusText(errStatus), uint64(errStatus))
}

// Numeric errors for status.
const (
	ErrStatusNone               = iota
	ErrStatusUnknown ErrNumeric = (1 << 8) | iota
	ErrStatusInvalidProductName
	ErrStatusAlreadyInitialized
	ErrStatusNotInitialized
	ErrStatusTimeout
)

// StatusSuccess is the success response as returned by this library.
const StatusSuccess = ErrStatusNone

// Numeric errors for ensure comparison.
const (
	ErrEnsureOnlineFailed ErrNumeric = (1 << 16) | iota
	ErrEnsureTrustedFailed
	ErrEnsureProductNotFound
	ErrEnsureProductNotLicensed
	ErrEnsureProductClaimNotFound
	ErrEnsureProductClaimValueTypeMismatch
	ErrEnsureProductClaimValueMismatch
	ErrEnsureUnknownOperator
)

// ErrNumericToTextMap maps numeric errors to readable names.
var ErrNumericToTextMap = map[ErrNumeric]string{
	ErrStatusUnknown:            "Unknown",
	ErrStatusInvalidProductName: "Invalid Product Name Value",
	ErrStatusAlreadyInitialized: "Already Initialized",
	ErrStatusNotInitialized:     "Not Initialized",
	ErrStatusTimeout:            "Timeout",

	ErrEnsureOnlineFailed:                  "Ensure failed, product claim set not online",
	ErrEnsureTrustedFailed:                 "Ensure failed, product claim set not trusted",
	ErrEnsureProductNotFound:               "Ensure failed, product entry not found",
	ErrEnsureProductNotLicensed:            "Ensure failed, product is not licensed",
	ErrEnsureProductClaimNotFound:          "Ensure failed, product claim entry not found",
	ErrEnsureProductClaimValueTypeMismatch: "Ensure failed, product claim value type mismatch",
	ErrEnsureProductClaimValueMismatch:     "Ensure failed, product claim value mismatch",
	ErrEnsureUnknownOperator:               "Ensure failed, unknown operator",
}

// ErrStatusText returns a text for the ErrStatus. It returns the empty string
// if the code is unknown.
func ErrStatusText(code ErrNumeric) string {
	text := ErrNumericToTextMap[code]
	return text
}
