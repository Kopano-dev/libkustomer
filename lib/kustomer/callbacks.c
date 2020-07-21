/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#include "kustomer_callbacks.h"

void bridge_kustomer_log_cb_func_log_s(kustomer_cb_func_log_s f, char* s)
{
	return f(s);
}

void bridge_kustomer_watch_cb_func_updated(kustomer_cb_func_watch f)
{
	return f();
}
