/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

package main

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	kustomer "stash.kopano.io/kc/libkustomer"
)

func main() {
	logger := log.New(os.Stderr, "Log: ", 0)

	k, err := kustomer.New(&kustomer.Config{
		Logger:      logger,
		Debug:       true,
		AutoRefresh: true,
	})
	if err != nil {
		panic(err)
	}

	ctx := context.Background()

	fmt.Println("Initializing ...")
	err = k.Initialize(ctx, nil)
	if err != nil {
		panic(err)
	}

	fmt.Printf("Initialized, library version: %s\n", k.Version())

	fmt.Println("Waiting until ready ...")
	updateCh := make(chan bool)
	errCh := make(chan error)
	go func() {
		notifyErr := k.NotifyWhenUpdated(ctx, updateCh)
		errCh <- notifyErr
	}()

	select {
	case <-updateCh:
	case err = <-errCh:
		panic(err)
	case <-time.After(30 * time.Second):
		panic("timeout waiting for first update")
	}
	fmt.Println("Ready with aggregated claims:")
	dumpAsJSON(k.CurrentKopanoProductClaims(ctx).Dump())

	go func() {
		for v := range updateCh {
			fmt.Println("Claims have been updated:", v)
			dumpAsJSON(k.CurrentKopanoProductClaims(ctx).Dump())
		}
	}()

	fmt.Println("Claims active on load:")
	dumpAsJSON(k.CurrentClaims(ctx).Dump())

	fmt.Println("\nPress CTRL+C to exit.")
	signalCh := make(chan os.Signal, 1)
	signal.Notify(signalCh, syscall.SIGINT, syscall.SIGTERM)
	<-signalCh
}

func dumpAsJSON(v interface{}) error {
	b, err := json.MarshalIndent(v, "", "\t")
	if err != nil {
		return err
	}

	fmt.Println(string(b))
	return nil
}
