/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

import (
	"fmt"
	"sort"
	"strings"

	kustomer "stash.kopano.io/kc/libkustomer"
)

func main() {
	var sorted sort.IntSlice

	for err := range kustomer.ErrNumericToTextMap {
		sorted = append(sorted, int(err))
	}

	sorted.Sort()

	fmt.Printf("#define KUSTOMER_ERRSTATUSSUCCESS\t%d\n", kustomer.StatusSuccess)
	for _, errNum := range sorted {
		err := kustomer.ErrNumeric(errNum)
		fmt.Printf("#define KUSTOMER_%s\t0x%x\t// %d\n", strings.ToUpper(err.String()), int(err), int(err))
	}
}
