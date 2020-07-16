/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

import (
	"context"
	"errors"
	"fmt"
	"log"
	"os"
	"sync"
	"time"

	"stash.kopano.io/kc/libkustomer"
)

var (
	mutex                    sync.RWMutex
	debug                    bool
	initializedContext       context.Context
	initializedContextCancel context.CancelFunc
	initializedLogger        *log.Logger
	instance                 *kustomer.Kustomer
)

func init() {
	if os.Getenv("KUSTOMER_DEBUG") != "" {
		debug = true
		fmt.Println("kustomer-c debug enabled")
		initializedLogger = log.New(os.Stdout, "kustomer-c debug ", 0)
	}
}

// Initialize initializes the global library state with the provided product name.
func Initialize(ctx context.Context, productName string) error {
	mutex.Lock()
	defer mutex.Unlock()

	if instance != nil {
		return kustomer.ErrStatusAlreadyInitialized
	}

	k, err := kustomer.New(&kustomer.Config{
		Logger:      initializedLogger,
		Debug:       debug,
		AutoRefresh: true,
	})
	if err != nil {
		if debug {
			fmt.Printf("kustomer-c initialize failed: %v\n", err)
		}
		return err
	}

	err = k.Initialize(ctx, &productName)
	if err != nil {
		if debug {
			fmt.Printf("kustomer-c initialize failed: %v\n", err)
		}
		return err
	}

	instance = k
	initializedContext, initializedContextCancel = context.WithCancel(ctx)
	if debug {
		fmt.Printf("kcoidc-c initialize success: %v\n", productName)
	}
	return nil
}

// Uninitialize uninitializes the global library state.
func Uninitialize() error {
	mutex.Lock()
	defer mutex.Unlock()

	if debug {
		fmt.Println("kustomer-c uninitialize")
	}

	err := instance.Uninitialize()
	if err != nil {
		return err
	}

	initializedContextCancel()
	initializedContext = nil
	initializedContextCancel = nil

	instance = nil
	if debug {
		fmt.Println("kustomer-c uninitialize success")
	}
	return nil
}

// WaitUntilReady blocks until the initialization is ready or timeout.
func WaitUntilReady(timeout time.Duration) error {
	mutex.RLock()
	k := instance
	ctx := initializedContext
	mutex.RUnlock()

	var err error
	if debug {
		fmt.Println("kustomer-c waiting until ready")
	}

	if k == nil {
		err = kustomer.ErrStatusNotInitialized
	} else {
		timeoutCtx, timeoutCtxCancel := context.WithTimeout(ctx, timeout)
		err = k.WaitUntilReady(timeoutCtx)
		timeoutCtxCancel()
		if errors.Is(err, context.DeadlineExceeded) {
			err = kustomer.ErrStatusTimeout
		}
	}
	if debug {
		fmt.Printf("kustomer-c finished waiting until ready: %v\n", err)
	}

	return err
}

func main() {}
