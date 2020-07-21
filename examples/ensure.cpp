/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

namespace kustomer {
	#include "kustomer.h"
	#include "kustomer_errors.h"
}

std::string productName;

int handleError(std::string msg, int code)
{
	std::cerr << "> Error (0x" << std::hex << code << "): " << msg << std::endl;
	return -1;
}

int handleExit(int code) {
	int res;

	if ((res = kustomer::kustomer_uninitialize()) != KUSTOMER_ERRSTATUSSUCCESS) {
		handleError("failed to uninitialize", res);
	};

	exit(code);
}

void handleSignal(int sigNum)
{
	std::cout << "Signal (" << sigNum << ") received.\n";

	handleExit(sigNum);
}

int handleUpdate()
{
	int res;
	int status = 0;

	auto transaction = kustomer::kustomer_begin_ensure();
	if (transaction.r0 != 0) {
		return handleError("failed to begin ensure transaction", transaction.r0);
	}

	if ((res = kustomer::kustomer_ensure_set_allow_untrusted(transaction.r1, 1)) != KUSTOMER_ERRSTATUSSUCCESS) {
		auto text = kustomer::kustomer_err_numeric_text(res);
		std::cerr << "Ensured failed to allow untrusted (Error: " << text << ")" << std::endl;
		free(text);
		status = -1;
	}

	if ((res = kustomer::kustomer_ensure_ok(transaction.r1, const_cast<char*>(productName.c_str()))) == KUSTOMER_ERRSTATUSSUCCESS) {
		std::cout << "Ensured OK" << std::endl;
	} else {
		auto text = kustomer::kustomer_err_numeric_text(res);
		std::cerr << "Ensured NOT OK (Error: " << text << ")" << std::endl;
		free(text);
		status = -1;
	}

	auto dump = kustomer::kustomer_dump_ensure(transaction.r1);
	if (dump.r0 != 0) {
		auto text = kustomer::kustomer_err_numeric_text(dump.r0);
		std::cerr << "> Error: dump failed (" << text << ")" << std::endl;
		free(text);
		status = -1;
	} else {
		std::cout << dump.r1 << std::endl;
		free(dump.r1);
	}

	if ((res = kustomer::kustomer_end_ensure(transaction.r1)) != KUSTOMER_ERRSTATUSSUCCESS) {
		return handleError("failed to end ensure transaction", res);
	}

	return status;
}

void onWatchUpdate()
{
	std::cout << "Update triggered ..." << std::endl;
	handleUpdate();
}

void onWatchExit()
{
	std::cout << "Watch has ended!" << std::endl;
}

void log(char* msg) {
	auto s = std::string(msg);

	std::cout << "> Log: " << s << std::endl;
}

int main(int argc, char** argv)
{

	if (argc <= 1) {
		handleError("Missing argument: productName", 0);
		exit(1);
	}
	productName = argv[1];
	int debug = -1;

	int res;

	if ((res = kustomer::kustomer_set_logger(log, debug)) != KUSTOMER_ERRSTATUSSUCCESS) {
		return handleError("set logger failed", res);
	}

	if ((res = kustomer::kustomer_set_autorefresh(1)) != KUSTOMER_ERRSTATUSSUCCESS) {
		return handleError("set autorefresh failed", res);
	}

	std::cout << "Initializing ..." << std::endl;

	if ((res = kustomer::kustomer_initialize(nullptr)) != KUSTOMER_ERRSTATUSSUCCESS) {
		return handleError("initialize failed", res);
	};
	if ((res = kustomer::kustomer_wait_until_ready(10)) != KUSTOMER_ERRSTATUSSUCCESS) {
		return handleError("failed to get ready in time", res);
	};

	auto status = handleUpdate();
	if (status != 0) {
		return handleExit(status);
	}

	if ((res = kustomer::kustomer_set_notify_when_updated(onWatchUpdate, onWatchExit)) != KUSTOMER_ERRSTATUSSUCCESS) {
		return handleError("failed to watch for updates", res);
	}

	signal(SIGINT, handleSignal);
	signal(SIGTERM, handleSignal);
	std::cout << std::endl << "Press CTRL+C to exit." << std::endl;
	pause();

	return status;
}
