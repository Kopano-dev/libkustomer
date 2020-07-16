/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#include <iostream>
#include <signal.h>
#include <unistd.h>

namespace kustomer {
	extern "C" {
		#include "kustomer.h"
	}
}

int handleError(std::string msg, int code)
{
	std::cerr << "> Error (0x" << std::hex << code << "): " << msg << std::endl;
	return -1;
}

void handleSignal(int sigNum)
{
	std::cout << "Signal (" << sigNum << ") received.\n";
	int res;

	if ((res = kustomer::kustomer_uninitialize()) != 0) {
		handleError("failed to uninitialize", res);
	};

	exit(sigNum);
}

int main(int argc, char** argv)
{
	std::cout << "Initializing ..." << std::endl;
	int res;

	std::string productName = "test";

	if ((res = kustomer::kustomer_initialize(const_cast<char*>(productName.c_str()))) != 0) {
		return handleError("initialize failed", res);
	};
	if ((res = kustomer::kustomer_wait_until_ready(10)) != 0) {
		return handleError("failed to get ready in time", res);
	};

	// TODO(longsleep): Do something.

	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);
	std::cout << std::endl << "Press CTRL+C to exit." << std::endl;
	pause();

	return 0;
}
