/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#ifndef KUSTOMER_CALLBACKS_H
#define KUSTOMER_CALLBACKS_H

typedef void (*kustomer_cb_func_log_s) (char*);
typedef void (*kustomer_cb_func_watch) ();

void bridge_kustomer_log_cb_func_log_s(kustomer_cb_func_log_s f, char* s);
void bridge_kustomer_watch_cb_func_updated(kustomer_cb_func_watch f);

#endif /* !KUSTOMER_CALLBACKS_H */
