/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package kustomer

import (
	"io"
	"net/http"

	"stash.kopano.io/kc/libkustomer/version"
)

// DefaultUserAgent is the HTTP user agent set in to request headers for HTTP
// requests created by this library.
var DefaultUserAgent = "libkustomer/" + version.Version

func newRequestWithUserAgent(method string, uri string, body io.Reader) (*http.Request, error) {
	request, err := http.NewRequest(method, uri, body)
	if err != nil {
		return nil, err
	}

	request.Header.Set("User-Agent", DefaultUserAgent)
	return request, nil
}
