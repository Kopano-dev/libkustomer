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
	"stash.kopano.io/kc/libkustomer/internal/version"
)

// This module uses a global state, to track initialization per process. The
// following mutex and values reflect this behavior.
var (
	mutex sync.RWMutex

	debug             bool
	autoRefresh       = false
	initializedLogger kustomer.Logger
	productUserAgent  *string
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

// Version returns the runtime version string of this module.
func Version() string {
	return version.Version
}

// BuildDate returns the build data string of this module.
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

// SetProductUserAgent sets an additional user agent string which is used when
// creating HTTP requests (User-Agent request header.) Use this value to
// make it possible to identify the software doing license checks. Set as nil
// to reset any previously set value.
func SetProductUserAgent(ua *string) error {
	mutex.Lock()
	defer mutex.Unlock()

	if instance != nil {
		return kustomer.ErrStatusAlreadyInitialized
	}
	productUserAgent = ua
	return nil
}

// Initialize initializes the global library state with the provided product
// name. Use nil productName to initialize for all products. The initialization
// is bound to the provided context and resources are relased when it is done.
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
		Logger: initializedLogger,

		Debug:       debug,
		AutoRefresh: autoRefresh,

		ProductUserAgent: productUserAgent,
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

// Uninitialize uninitializes the global library state. Make sure to call this
// when cleaning up and having call Initialize before.
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

// WaitUntilReady blocks until the initialization is ready or until the provided
// timeout. It also stops when the global library state is uninitialized.
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

// CurrentClaims return the current active claim set using the global library
// state instance.
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

// CurrentKopanoProductClaims returns the current active Kopanp product claims
// using the global library state instance.
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

// InstanceEnsure is a way to start an ensure transaction without having to
// initialize the global library strate. The transaction is bound to the
// provided context and is using the provide product name and user agent
// accordingly. This function blocks until the transaction data is available or
// until the provided context is done or the provided timeout was reached. On
// success, the Kopano product claims transaction is returned.
func InstantEnsure(ctx context.Context,
	productName, productUserAgent *string, timeout time.Duration) (*kustomer.KopanoProductClaims, error) {
	mutex.RLock()
	logger := initializedLogger
	mutex.RUnlock()

	k, err := kustomer.New(&kustomer.Config{
		Logger: logger,

		Debug:       debug,
		AutoRefresh: false,

		ProductUserAgent: productUserAgent,
	})
	if err != nil {
		if debug {
			initializedLogger.Printf("kustomer-c begin instant ensure failed: %v\n", err)
		}
		return nil, err
	}

	if debug {
		initializedLogger.Printf("kustomer-c begin instant ensure (debug: %v)\n", debug)
	}

	err = k.Initialize(ctx, productName)
	if err != nil {
		if debug {
			initializedLogger.Printf("kustomer-c begin instant ensure initialize failed: %v\n", err)
		}
		return nil, err
	}
	defer k.Uninitialize() //nolint

	timeoutCtx, timeoutCtxCancel := context.WithTimeout(ctx, timeout)
	err = k.WaitUntilReady(timeoutCtx)
	timeoutCtxCancel()
	if err != nil {
		if errors.Is(err, context.DeadlineExceeded) {
			err = kustomer.ErrStatusTimeout
		}
		return nil, err
	}

	kpc := k.CurrentKopanoProductClaims(ctx)
	return kpc, nil
}

// ErrNumericText is a helper function to retrieve a string message associated
// with the provided numeric error.
func ErrNumericText(err kustomer.ErrNumeric) string {
	return kustomer.ErrNumericText(err)
}

// SetNotifyWhenUpdated sets the callback function to watch for claim updates
// on the global library state instance. The global library state must have
// been initialized to use this function. The lifetime of all resources
// created by this function is bound to the global library initialization and
// can be unset by using the UnsetNotifyWhenUpdated function.
func SetNotifyWhenUpdated(updateCb, exitCb func()) error {
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

// UnsetNotifyWhenUpdated removes the set notify callback if there is any, and
// releases its resources.
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
