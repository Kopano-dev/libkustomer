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
	"os/signal"
	"syscall"
	"time"

	kustomer "stash.kopano.io/kc/libkustomer"
)

func main() {
	logger := log.New(os.Stderr, "", log.Lshortfile)

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
	err = k.WaitUntilReady(ctx, 30*time.Second)
	if err != nil {
		panic(err)
	}
	fmt.Println("Ready.")

	fmt.Println("\nPress CTRL+C to exit.")
	signalCh := make(chan os.Signal, 1)
	signal.Notify(signalCh, syscall.SIGINT, syscall.SIGTERM)
	<-signalCh
}
