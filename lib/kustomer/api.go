/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

/*
#define KUSTOMER_API 1
#define KUSTOMER_API_MINOR 1

#define KUSTOMER_VERSION (KUSTOMER_API * 10000 + KUSTOMER_API_MINOR * 100)

*/
import "C"

//export kustomer_available
func kustomer_available() C.int {
	return 1
}
