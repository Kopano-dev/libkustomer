/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

import (
	"context"
	"errors"
	"os"
	"sync"
	"time"

	"stash.kopano.io/kc/libkustomer" //nolint:goimports // False positive.
	"stash.kopano.io/kc/libkustomer/version"
)

var (
	mutex sync.RWMutex

	debug             bool
	autoRefresh       = false
	initializedLogger kustomer.Logger
	instance          *kustomer.Kustomer

	initializedContext       context.Context
	initializedContextCancel context.CancelFunc

	initializedNotifyCancel context.CancelFunc
)

func init() { //nolint:gochecknoinits // This library uses init to set up env.
	if os.Getenv("KUSTOMER_DEBUG") != "" {
		debug = true
		initializedLogger = getDefaultDebugLogger()
	}
}

func Version() string {
	return version.Version
}

func BuildDate() string {
	return version.BuildDate
}

// SetAutoRefresh toggles weather or not this library should enable auto
// refresh of active claims or not. It must be called before the call to
// Initialize.
func SetAutoRefresh(flag bool) error {
	mutex.Lock()
	defer mutex.Unlock()

	if instance != nil {
		return kustomer.ErrStatusAlreadyInitialized
	}
	autoRefresh = flag
	return nil
}

// SetLogger sets the logger to be used by this library and if to use debug
// logging. It must be called before the call to initialize.
func SetLogger(logger kustomer.Logger, debugFlag *bool) error {
	mutex.Lock()
	defer mutex.Unlock()

	if instance != nil {
		return kustomer.ErrStatusAlreadyInitialized
	}
	initializedLogger = logger
	if debugFlag != nil {
		debug = *debugFlag
	}
	return nil
}

// Initialize initializes the global library state with the provided product name.
func Initialize(ctx context.Context, productName *string) error {
	mutex.Lock()
	defer mutex.Unlock()

	if instance != nil {
		return kustomer.ErrStatusAlreadyInitialized
	}

	if initializedLogger == nil && debug {
		initializedLogger = getDefaultDebugLogger()
	}

	k, err := kustomer.New(&kustomer.Config{
		Logger:      initializedLogger,
		Debug:       debug,
		AutoRefresh: autoRefresh,
	})
	if err != nil {
		if debug {
			initializedLogger.Printf("kustomer-c initialize failed: %v\n", err)
		}
		return err
	}

	if debug {
		initializedLogger.Printf("kustomer-c initializing (autoRefresh: %v, debug: %v)\n", autoRefresh, debug)
	}
	err = k.Initialize(ctx, productName)
	if err != nil {
		if debug {
			initializedLogger.Printf("kustomer-c initialize failed: %v\n", err)
		}
		return err
	}

	instance = k
	initializedContext, initializedContextCancel = context.WithCancel(ctx)
	if debug {
		var productNameString string
		if productName != nil {
			productNameString = *productName
		}
		initializedLogger.Printf("kustomer-c initialize success: %v\n", productNameString)
	}
	return nil
}

// Uninitialize uninitializes the global library state.
func Uninitialize() error {
	mutex.Lock()
	defer mutex.Unlock()

	if instance == nil {
		return kustomer.ErrStatusNotInitialized
	}

	if debug {
		initializedLogger.Printf("kustomer-c uninitialize\n")
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
		initializedLogger.Printf("kustomer-c uninitialize success\n")
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
		initializedLogger.Printf("kustomer-c waiting until ready\n")
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
		initializedLogger.Printf("kustomer-c finished waiting until ready: %v\n", err)
	}

	return err
}

func CurrentClaims() (*kustomer.Claims, error) {
	mutex.RLock()
	k := instance
	ctx := initializedContext
	mutex.RUnlock()

	if k == nil {
		return nil, kustomer.ErrStatusNotInitialized
	}

	return k.CurrentClaims(ctx), nil
}

func CurrentKopanoProductClaims() (*kustomer.KopanoProductClaims, error) {
	mutex.RLock()
	k := instance
	ctx := initializedContext
	mutex.RUnlock()

	if k == nil {
		return nil, kustomer.ErrStatusNotInitialized
	}

	return k.CurrentKopanoProductClaims(ctx), nil
}

func ErrNumericText(err kustomer.ErrNumeric) string {
	return kustomer.ErrNumericText(err)
}

func SetNotifyWhenUpdated(updateCb func(), exitCb func()) error {
	mutex.Lock()
	if initializedNotifyCancel != nil {
		mutex.Unlock()
		return kustomer.ErrStatusAlreadyInitialized
	}
	k := instance
	notifyCtx, cancel := context.WithCancel(initializedContext)
	initializedNotifyCancel = cancel
	mutex.Unlock()

	if k == nil {
		return kustomer.ErrStatusNotInitialized
	}

	eventCh := make(chan bool, 4)

	go func() {
		defer cancel()
		err := k.NotifyWhenUpdated(notifyCtx, eventCh)
		if err != nil {
			if initializedLogger != nil && !errors.Is(err, context.Canceled) {
				initializedLogger.Printf("kustomer-c notify exit with error: %v\n", err)
			}
		}
		mutex.Lock()
		initializedNotifyCancel = nil
		mutex.Unlock()
	}()

	go func() {
		for {
			select {
			case <-notifyCtx.Done():
				close(eventCh)
				exitCb()
				return
			case updated := <-eventCh:
				if updated {
					updateCb()
				}
			}
		}
	}()

	return nil
}

func UnsetNotifyWhenUpdated() error {
	mutex.Lock()
	if initializedNotifyCancel == nil {
		mutex.Unlock()
		return kustomer.ErrStatusNotInitialized
	}

	cancel := initializedNotifyCancel
	initializedNotifyCancel = nil
	mutex.Unlock()

	cancel()

	return nil
}

func main() {}
