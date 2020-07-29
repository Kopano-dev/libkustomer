/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

import (
	"context"
	"encoding/json"
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	kustomer "stash.kopano.io/kc/libkustomer"
)

var (
	debug          bool
	dumpClaimsOnly bool
	dumpKpcOnly    bool
	watch          bool

	logger   *log.Logger
	instance *kustomer.Kustomer
)

func initialize(ctx context.Context) {
	logger = log.New(os.Stderr, "> ", 0)

	k, err := kustomer.New(&kustomer.Config{
		Logger:      logger,
		Debug:       false,
		AutoRefresh: true,
	})
	if err != nil {
		panic(err)
	}

	logger.Println("initializing ...")
	err = k.Initialize(ctx, nil)
	if err != nil {
		panic(err)
	}

	logger.Printf("library version: %s (%s) initialized\n", k.Version(), k.BuildDate())
	instance = k
}

func uninitialize() {
	instance.Uninitialize()
	instance = nil
}

func dump(ctx context.Context) {
	if !dumpClaimsOnly {
		logger.Println("aggregated product claims ready")
		if err := dumpAsJSON(instance.CurrentKopanoProductClaims(ctx).Dump()); err != nil {
			panic(err)
		}
	}
	if !dumpKpcOnly {
		logger.Println("active claims loaded")
		if err := dumpAsJSON(instance.CurrentClaims(ctx).Dump()); err != nil {
			panic(err)
		}
	}
}

func main() {
	flag.BoolVar(&debug, "debug", false, "Enable debug output")
	flag.BoolVar(&dumpClaimsOnly, "claims-only", false, "Only dump raw active claim set JSON")
	flag.BoolVar(&dumpKpcOnly, "kpc-only", false, "Only dump raw active Kopano product claim set JSON")
	flag.BoolVar(&watch, "watch", false, "Keep running and watch for changes")
	flag.Parse()

	var err error
	ctx := context.Background()

	initialize(ctx)
	defer uninitialize()

	timeoutCtx, cancel := context.WithTimeout(ctx, 30*time.Second)
	defer cancel()

	logger.Println("waiting until ready ...")
	if watch {

		updateCh := make(chan bool)
		errCh := make(chan error)
		go func() {
			notifyErr := instance.NotifyWhenUpdated(ctx, updateCh)
			errCh <- notifyErr
		}()

		select {
		case <-updateCh:
		case err = <-errCh:
			panic(err)
		case <-timeoutCtx.Done():
			panic("timeout waiting for first update")
		}
		dump(ctx)
		go func() {
			for v := range updateCh {
				logger.Println("claims have been updated:", v)
				dump(ctx)
			}
		}()
	} else {
		err = instance.WaitUntilReady(timeoutCtx)
		if err != nil {
			panic(err)
		}
		dump(ctx)
	}

	if watch {
		logger.Println("watching for changes, Press CTRL+C to or send INT or TERM signal to exit ...")
		signalCh := make(chan os.Signal, 1)
		signal.Notify(signalCh, syscall.SIGINT, syscall.SIGTERM)
		<-signalCh
	}
}

func dumpAsJSON(v interface{}) error {
	b, err := json.MarshalIndent(v, "", "  ")
	if err != nil {
		return err
	}

	fmt.Println(string(b))
	return nil
}
