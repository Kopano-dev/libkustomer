/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package kustomer

import "testing"

func TestErrors(t *testing.T) {
	for err := range ErrNumericToTextMap {
		t.Logf("%d: %s", err, err)
	}
}
