/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package kustomer

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"sync"
	"time"

	"github.com/longsleep/sse"
	api "stash.kopano.io/kgol/kustomer/server/api-v1"

	"stash.kopano.io/kc/libkustomer/version"
)

// A Kustomer is the representation of the Kustomer interface.
type Kustomer struct {
	mutex sync.RWMutex

	initialized bool
	trusted     bool
	ready       chan struct{}
	ctx         context.Context
	cancel      context.CancelFunc

	httpClient *http.Client

	logger      Logger
	debug       bool
	autoRefresh bool

	apiPath string

	version   string
	buildDate string

	updated                    chan struct{}
	currentKopanoProductClaims *api.ClaimsKopanoProductsResponse

	fetching      chan struct{}
	currentClaims *api.ClaimsResponse
}

func New(config *Config) (*Kustomer, error) {
	if config == nil {
		config = &Config{
			Logger: DefaultLogger,
		}
	}

	k := &Kustomer{
		logger:      config.Logger,
		debug:       config.Debug,
		autoRefresh: config.AutoRefresh,

		version:   version.Version,
		buildDate: version.BuildDate,

		updated: make(chan struct{}),
		currentKopanoProductClaims: &api.ClaimsKopanoProductsResponse{
			Trusted:  false,
			Offline:  true,
			Products: make(map[string]*api.ClaimsKopanoProductsResponseProduct),
		},
	}

	var dialer net.Dialer
	k.httpClient = &http.Client{
		Transport: &http.Transport{
			DialContext: func(ctx context.Context, proto, addr string) (conn net.Conn, err error) {
				if k.initialized == false {
					return nil, fmt.Errorf("cannot dial to API: %w", ErrStatusNotInitialized)
				}
				return dialer.DialContext(ctx, "unix", k.apiPath)
			},
		},
	}

	return k, nil
}

func (k *Kustomer) Version() string {
	return k.version
}

func (k *Kustomer) BuildDate() string {
	return k.buildDate
}

func (k *Kustomer) Initialize(ctx context.Context, productName *string) error {
	if productName != nil && *productName == "" {
		return ErrStatusInvalidProductName
	}

	k.mutex.Lock()
	defer k.mutex.Unlock()
	if k.initialized {
		return ErrStatusAlreadyInitialized
	}
	initializeCtx, cancel := context.WithCancel(ctx)
	k.ctx = initializeCtx
	k.cancel = cancel
	k.initialized = true

	trigger := make(chan bool, 1)
	ready := make(chan struct{})
	k.ready = ready

	apiPath := DefaultAPIPath
	trusted := true
	if a := os.Getenv("KUSTOMER_API_PATH"); a != "" {
		absPath, absErr := filepath.Abs(a)
		if absErr != nil {
			return absErr
		}
		apiPath = absPath
		trusted = false
	}
	k.apiPath = apiPath
	k.trusted = trusted
	if k.debug {
		k.logger.Printf("kustomer initializing with %s (trusted: %v)\n", k.apiPath, k.trusted)
	}

	go func() {
		k.mutex.RLock()
		debug := k.debug
		logger := k.logger
		autoRefresh := k.autoRefresh
		if !autoRefresh || k.ready != ready || !k.initialized {
			k.mutex.RUnlock()
			return
		}
		k.mutex.RUnlock()

		uri := url.URL{
			Scheme: "http",
			Host:   "localhost",
			Path:   "/api/v1/claims/watch",
		}

		first := true
		for {
			eventCh, errCh := func() (<-chan *sse.Event, <-chan error) {
				c := make(chan *sse.Event, 4)
				e := make(chan error)

				go func() {
					if debug {
						logger.Printf("libkustomer claims watch start\n")
					}
					err := sse.Notify(uri.String(), k.httpClient, newRequestWithUserAgent, c)
					e <- err
				}()

				return c, e
			}()
		retry:
			for {
				select {
				case err := <-errCh:
					if debug {
						if err == nil {
							logger.Printf("libkustomer claims watch ended (will reconnect)\n")
						} else {
							logger.Printf("libkustomer claims watch error (will reconnect): %v\n", err.Error())
						}
					}
					// Automatic reconect.
					select {
					case <-initializeCtx.Done():
						return
					case <-time.After(5 * time.Second):
						// breaks
						break retry
					}
				case event := <-eventCh:
					var data []byte
					if event.Data != nil {
						data, _ = ioutil.ReadAll(event.Data)
					}
					switch event.Type {
					case "hello":
						if first {
							if debug {
								logger.Printf("libkustomer claims watch first hello received: %v\n", string(data))
							}
							first = false
							trigger <- true
						}
					case "claims-updated":
						if debug {
							logger.Printf("libkustomer claims watch update notification received\n")
						}
						select {
						case trigger <- true:
						default:
							if debug {
								logger.Printf("libkustomer claims trigger busy\n")
							}
						}
					}
				case <-initializeCtx.Done():
					return
				}
			}
		}
	}()

	go func() {
		var first = true
		for {
			k.mutex.Lock()
			debug := k.debug
			logger := k.logger
			autoRefresh := k.autoRefresh
			if k.ready != ready || !k.initialized {
				k.mutex.Unlock()
				return
			}
			k.currentClaims = nil // Always reset any loaded claims before we refresh.
			k.mutex.Unlock()

			if autoRefresh && first {
				// If auto refresh is turned on, the first run is delayed until
				// the auto refresh watcher is ready. This avoids double fetch
				// on startup.
				select {
				case <-initializeCtx.Done():
					return
				case <-trigger:
					// breaks
				}
			}

			timeoutContext, timeoutContextCancel := context.WithTimeout(k.ctx, 60*time.Second)
			kopanoProductClaims, err := k.fetchClaimsKopanoProducts(timeoutContext, productName)
			timeoutContextCancel()
			if err != nil {
				if debug {
					logger.Printf("libkustomer fetch error: %v\n", err.Error())
				}

				// Automatic retry on error.
				select {
				case <-initializeCtx.Done():
					return
				case <-time.After(5 * time.Second):
					// breaks
				}
				continue
			}

			if kopanoProductClaims != nil {
				k.mutex.Lock()
				k.currentKopanoProductClaims = kopanoProductClaims
				updated := k.updated
				k.updated = make(chan struct{})
				close(updated)
				k.mutex.Unlock()
			}

			if first {
				// If this is the first run, signal that operation is ready.
				first = false
				close(ready)
			}
			if !autoRefresh {
				// No auto refresh, exit here directly.
				return
			}
			// Wait for signal to run again or to exit.
			select {
			case <-initializeCtx.Done():
				return
			case <-trigger:
				// breaks
			}
		}
	}()

	return nil
}

func (k *Kustomer) Uninitialize() error {
	k.mutex.Lock()
	defer k.mutex.Unlock()

	if !k.initialized {
		return ErrStatusNotInitialized
	}
	k.initialized = false
	k.cancel()

	return nil
}

func (k *Kustomer) WaitUntilReady(ctx context.Context) error {
	k.mutex.RLock()
	if !k.initialized {
		k.mutex.RUnlock()
		return ErrStatusNotInitialized
	}
	ready := k.ready
	initializeCtx := k.ctx
	k.mutex.RUnlock()

	var err error
	select {
	case <-ready:
	case <-ctx.Done():
	case <-initializeCtx.Done():
		err = initializeCtx.Err()
	}

	return err
}

func (k *Kustomer) NotifyWhenUpdated(ctx context.Context, eventCh chan<- bool) error {
	err := func() error {
		for {
			k.mutex.RLock()
			if !k.initialized {
				k.mutex.RUnlock()
				return ErrStatusNotInitialized
			}
			updated := k.updated
			initializeCtx := k.ctx
			k.mutex.RUnlock()

			select {
			case <-updated:
				eventCh <- true
			case <-ctx.Done():
				return nil
			case <-initializeCtx.Done():
				return initializeCtx.Err()
			}
		}
	}()
	return err
}

func (k *Kustomer) fetchClaimsKopanoProducts(ctx context.Context, productName *string) (*api.ClaimsKopanoProductsResponse, error) {
	uri := url.URL{
		Scheme: "http",
		Host:   "localhost",
		Path:   "/api/v1/claims/kopano/products",
	}
	query := uri.Query()
	if productName != nil {
		query.Set("product", *productName)
	}
	uri.RawQuery = query.Encode()

	request, err := newRequestWithUserAgent(http.MethodGet, uri.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("API request could not be created: %w", err)
	}

	request = request.WithContext(ctx)

	response, err := k.httpClient.Do(request)
	if err != nil {
		return nil, fmt.Errorf("API request failed: %w", err)
	}
	defer response.Body.Close()
	if response.StatusCode != http.StatusOK {
		bodyBytes, _ := ioutil.ReadAll(response.Body)
		return nil, fmt.Errorf("API request failed with status: %v (%v)", response.StatusCode, string(bodyBytes))
	}

	kpc := &api.ClaimsKopanoProductsResponse{}
	err = json.NewDecoder(response.Body).Decode(kpc)
	if err != nil {
		return nil, fmt.Errorf("API response parse error: %w", err)
	}
	return kpc, nil
}

func (k *Kustomer) fetchClaims(ctx context.Context) (*api.ClaimsResponse, error) {
	uri := url.URL{
		Scheme: "http",
		Host:   "localhost",
		Path:   "/api/v1/claims",
	}

	request, err := newRequestWithUserAgent(http.MethodGet, uri.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("API request could not be created: %w", err)
	}

	request = request.WithContext(ctx)

	response, err := k.httpClient.Do(request)
	if err != nil {
		return nil, fmt.Errorf("API request failed: %w", err)
	}
	defer response.Body.Close()
	if response.StatusCode != http.StatusOK {
		bodyBytes, _ := ioutil.ReadAll(response.Body)
		return nil, fmt.Errorf("API request failed with status: %v (%v)", response.StatusCode, string(bodyBytes))
	}

	cr := &api.ClaimsResponse{}
	err = json.NewDecoder(response.Body).Decode(cr)
	if err != nil {
		return nil, fmt.Errorf("API response parse error: %w", err)
	}
	return cr, nil
}

func (k *Kustomer) CurrentKopanoProductClaims(ctx context.Context) *KopanoProductClaims {
	k.mutex.RLock()
	defer k.mutex.RUnlock()
	return &KopanoProductClaims{
		response: k.currentKopanoProductClaims,
	}
}

func (k *Kustomer) CurrentClaims(ctx context.Context) *Claims {
	k.mutex.RLock()
	claims := k.currentClaims
	debug := k.debug
	logger := k.logger
	k.mutex.RUnlock()

	if claims == nil {
		k.mutex.Lock()
		claims = k.currentClaims
		if claims == nil {
			fetching := k.fetching
			if fetching == nil {
				fetching = make(chan struct{})
				k.fetching = fetching
				k.mutex.Unlock()
				var err error
				claims, err = k.fetchClaims(ctx)
				k.mutex.Lock()
				k.fetching = nil
				defer close(fetching)
				if err == nil {
					k.currentClaims = claims
					k.mutex.Unlock()
				} else {
					k.mutex.Unlock()
					if debug {
						logger.Printf("libcustomer failed to fetch claims: %w\n", err)
					}
				}
			} else {
				k.mutex.Unlock()
				select {
				case <-fetching:
					return k.CurrentClaims(ctx)
				case <-ctx.Done():
					return nil
				}
			}
		} else {
			k.mutex.Unlock()
		}
	}

	return &Claims{
		response: claims,
	}
}
