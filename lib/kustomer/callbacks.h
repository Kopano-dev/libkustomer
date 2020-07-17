/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * Copyright 2020 Kopano and its licensors
 */

#ifndef KUSTOMER_CALLBACKS_H
#define KUSTOMER_CALLBACKS_H

typedef void (*kustomer_cb_func_log_s) (char*);

void bridge_kustomer_log_cb_func_log_s(kustomer_cb_func_log_s f, char* s);

#endif /* !KUSTOMER_CALLBACKS_H */
