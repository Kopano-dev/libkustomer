/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

import (
	"context"
	"fmt"
	"log"
	"os"
	"sync"

	"stash.kopano.io/kc/libkustomer"
)

var (
	mutex             sync.RWMutex
	debug             bool
	initializedLogger *log.Logger
	instance          *kustomer.Kustomer
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

	instance = nil
	if debug {
		fmt.Println("kustomer-c uninitialize success")
	}
	return nil
}

func main() {}
